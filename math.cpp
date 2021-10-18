
#include <cmath>
#include <limits>
#include <cstdlib>
#include <iostream>
#include "include/geometry.h"
#include "include/our_gl.h"
#include "include/math.h"
#include "include/data_input.h"
//#include "include/data.h"

//线性插值
float lerp(float a, float b, float t)
{
    if (t <= 0)
        return a;
    if (t >= 1)
        return b;
    return a + t * (b - a);
}

Vec3f lerp(Vec3f a, Vec3f b, float m) //m=0电介质a,m=1纯金属b
{
    if (m <= 0)
        return a;
    if (m >= 1)
        return b;
    return a + (b - a) * m;
}

float clamp(float x, float min, float max)
{
    if (x < min)
        return x;
    if (x > max)
        return max;
    return x;
}

float smoothstep(float t1, float t2, float x)
{
    x = clamp((x - t1) / (t2 - t1), 0.0, 1.0);
    return x * x * (3 - 2 * x);
}

float radians(float angle)
{
    return (angle / 180.f) * M_PI;
};

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
    Vec3f s[2];
    for (int i = 2; i--;)
    {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

Vec3f reflect(Vec3f e, Vec3f n)
{
    return (n * (n * e * (-2.f)) + e).normalize();
}

Vec3f refract(Vec3f i, Vec3f n, float eta)
{
    float k = 1.0 - eta * eta * (1 - (n * i) * (n * i));
    if (k < 0)
        return Vec3f(0, 0, 0);
    else
        return i * eta - n * (eta * (n * i) + sqrt(k));
}

/*
model矩阵：转换到世界坐标系
调用顺序：先scale 后tran
model*scale*tran 实际效果：先tran 后scale
*/
void scale(float s1, float s2, float s3, Matrix &ModelMat)
{
    Matrix scaleMat = Matrix::identity(); //单位矩阵对角线为1
    scaleMat[0][0] = s1;
    scaleMat[1][1] = s2;
    scaleMat[2][2] = s3;

    ModelMat = ModelMat * scaleMat;
}

void translate(float t1, float t2, float t3, Matrix &ModelMat)
{
    Matrix tran = Matrix::identity();
    tran[0][3] = t1;
    tran[1][3] = t2;
    tran[2][3] = t3;
    ModelMat = ModelMat * tran;
}

void rotate(float angle, Vec3f axis, Matrix &ModelMat)
{
    float x = axis[0];
    float y = axis[1];
    float z = axis[2];
    float r = radians(angle);
    Matrix rotate = Matrix::identity();
    rotate[0][0] = cos(r) + x * x * (1 - cos(r));
    rotate[0][1] = x * y * (1 - cos(r)) + z * sin(r);
    rotate[0][2] = x * z * (1 - cos(r)) - y * sin(r);

    rotate[1][0] = x * y * (1 - cos(r)) - z * sin(r);
    rotate[1][1] = cos(r) + y * y * (1 - cos(r));
    rotate[1][2] = y * z * (1 - cos(r)) + x * sin(r);

    rotate[2][0] = x * z * (1 - cos(r)) + y * sin(r);
    rotate[2][1] = y * z * (1 - cos(r)) - x * sin(r);
    rotate[2][2] = cos(r) + z * z * (1 - cos(r));

    ModelMat = ModelMat * rotate;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f v = (eye - center).normalize();
    Vec3f r = cross(up, v).normalize();
    up = cross(v, r).normalize();

    Matrix trans = Matrix::identity();
    trans[0][3] = -eye[0];
    trans[1][3] = -eye[1];
    trans[2][3] = -eye[2];

    Matrix change = Matrix::identity();
    for (int i = 0; i < 3; i++)
    {
        change[0][i] = r[i];
        change[1][i] = up[i];
        change[2][i] = v[i];
    }

    return change * trans;
}

Matrix rotate_camera(Vec3f eye, Vec3f center, Vec3f up, float pitch, float yaw)
{
    float p = radians(pitch);
    float y = radians(yaw);
    center[0] = cos(p) * cos(y);
    center[1] = sin(p);
    center[2] = cos(p) * sin(y);

    return lookat(eye, center, up);
}

/*
正交投影矩阵
near:+z,far:-z
*/
Matrix orthProjection(float l, float r, float b, float t, float f, float n)
{
    Matrix projection = Matrix::identity();
    projection[0][3] = -(r + l) / (r - l);
    projection[1][3] = -(t + b) / (t - b);
    projection[2][3] = -(f + n) / (f - n);

    projection[0][0] = 2.f / (r - l);
    projection[1][1] = 2.f / (t - b);
    projection[2][2] = -2.f / (n - f); //注意取负，变换左手坐标系

    return projection;
}

/*
透视投影矩阵
*/
//near:+z,far:-z
Matrix projection(float fovy, float aspect, float n, float f)
{
    Matrix projection = Matrix::identity();

    projection[0][0] = 1.0f / (tan(radians(0.5f * fovy)) * aspect);
    projection[1][1] = 1.0f / tan(radians(0.5f * fovy));
    projection[2][2] = (f + n) / (f - n);
    projection[2][3] = -2.f * f * n / (f - n);

    //透视除法 w=-eye.z
    projection[3][2] = -1;

    projection[3][3] = 0;

    return projection;
}

/*
viewport:标准化设备空间->屏幕空间
*/
Matrix viewport(int w, int h)
{
    Matrix viewPort = Matrix::identity();
    viewPort[0][3] = w / 2.f; //+w/8.f; //+x
    viewPort[1][3] = h / 2.f; //+h/8.f; //+y偏移量

    viewPort[0][0] = w / 2.f;
    viewPort[1][1] = h / 2.f;

    viewPort[2][3] = depth / 2.f;
    viewPort[2][2] = depth / 2.f;

    return viewPort;
}

//--------------------------------------------------------------------------------------------------------------
Vec3f getMiddlePoint(const Vec3f &pt1, const Vec3f &pt2)
{
    // Vec3f res = pt1 + pt2;
    return (pt1 + pt2) / 2.f;
}
void addFace(const Vec3i &face_vt, std::vector<std::vector<Vec3i>> &faces)
{
    //i 0 Vec3i[v1,vt1,vn1]
    //  1
    //  2
    std::vector<Vec3i> face;
    for (int i = 0; i < 3; i++)
        face.push_back(Vec3i(face_vt[i], -1, face_vt[i]));
    faces.push_back(face);
}
DataInput icosphere(int recursionLevel)
{
    DataInput data;
    float t = (1.0 + sqrt(5.0)) / 2.f;

    //正20面体
    data.addVertex(Vec3f(-1, t, 0).normalize());
    data.addVertex(Vec3f(1, t, 0).normalize());
    data.addVertex(Vec3f(-1, -t, 0).normalize());
    data.addVertex(Vec3f(1, -t, 0).normalize());

    data.addVertex(Vec3f(0, -1, t).normalize());
    data.addVertex(Vec3f(0, 1, t).normalize());
    data.addVertex(Vec3f(0, -1, -t).normalize());
    data.addVertex(Vec3f(0, 1, -t).normalize());

    data.addVertex(Vec3f(t, 0, -1).normalize());
    data.addVertex(Vec3f(t, 0, 1).normalize());
    data.addVertex(Vec3f(-t, 0, -1).normalize());
    data.addVertex(Vec3f(-t, 0, 1).normalize());

    data.addFace(Vec3i(0, 11, 5));
    data.addFace(Vec3i(0, 5, 1));
    data.addFace(Vec3i(0, 1, 7));
    data.addFace(Vec3i(0, 7, 10));
    data.addFace(Vec3i(0, 10, 11));

    data.addFace(Vec3i(1, 5, 9));
    data.addFace(Vec3i(5, 11, 4));
    data.addFace(Vec3i(11, 10, 2));
    data.addFace(Vec3i(10, 7, 6));
    data.addFace(Vec3i(7, 1, 8));

    data.addFace(Vec3i(3, 9, 4));
    data.addFace(Vec3i(3, 4, 2));
    data.addFace(Vec3i(3, 2, 6));
    data.addFace(Vec3i(3, 6, 8));
    data.addFace(Vec3i(3, 8, 9));

    data.addFace(Vec3i(4, 9, 5));
    data.addFace(Vec3i(2, 4, 11));
    data.addFace(Vec3i(6, 2, 10));
    data.addFace(Vec3i(8, 6, 7));
    data.addFace(Vec3i(9, 8, 1));

    //refine
    for (int k = 0; k < recursionLevel; k++)
    {
        std::vector<std::vector<Vec3i>> new_faces;
        for (int i = 0; i < data.nfaces(); i++)
        {
            Vec3f pts[3];
            for (int j = 0; j < 3; j++)
            {
                pts[j] = data.vert(i, j);
            }
            //三角形的三个顶点分别取中点，逆时针顺序生成新的面
            Vec3f a(getMiddlePoint(pts[0], pts[1]));
            Vec3f b(getMiddlePoint(pts[1], pts[2]));
            Vec3f c(getMiddlePoint(pts[2], pts[0]));

            int idx = data.nverts(); //0 1 2 size=3  add new 0 1 2 3
            data.addVertex(a.normalize());
            data.addVertex(b.normalize());
            data.addVertex(c.normalize());

            /* data.addFace(Vec3i(data.faces_[i][0][0], idx, idx + 2));
            data.addFace(Vec3i(data.faces_[i][1][0], idx + 1, idx));
            data.addFace(Vec3i(data.faces_[i][2][0], idx + 2, idx + 1));
            data.addFace(Vec3i(idx, idx + 1, idx + 2));*/

            addFace(Vec3i(data.faces_[i][0][0], idx, idx + 2), new_faces);
            addFace(Vec3i(data.faces_[i][1][0], idx + 1, idx), new_faces);
            addFace(Vec3i(data.faces_[i][2][0], idx + 2, idx + 1), new_faces);
            addFace(Vec3i(idx, idx + 1, idx + 2), new_faces);
        }
        data.faces_ = new_faces;
        data.norms_ = data.verts_;
    }

    return data;
}

//TBN矩阵--------------------------------------------------------------------------------------------------------------

mat<3, 3, float> getTBN(Vec3f *pts, Vec2f *uv)
{
    Vec3f tangent, bitangent, normal;
    Vec3f p1 = pts[0];
    Vec3f p2 = pts[1];
    Vec3f p3 = pts[2];

    Vec2f t1 = uv[0];
    Vec2f t2 = uv[1];
    Vec2f t3 = uv[2];

    Vec3f e1 = p2 - p1;
    Vec3f e2 = p3 - p1;

    Vec2f deltaUV1 = t2 - t1;
    Vec2f deltaUV2 = t3 - t1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent.x = f * (deltaUV2.y * e1.x - deltaUV1.y * e2.x);
    tangent.y = f * (deltaUV2.y * e1.y - deltaUV1.y * e2.y);
    tangent.z = f * (deltaUV2.y * e1.z - deltaUV1.y * e2.z);
    tangent.normalize();

    bitangent.x = f * (-deltaUV2.x * e1.x + deltaUV1.x * e2.x);
    bitangent.y = f * (-deltaUV2.x * e1.y + deltaUV1.x * e2.y);
    bitangent.z = f * (-deltaUV2.x * e1.z + deltaUV1.x * e2.z);
    bitangent.normalize();

    //normal = cross(tangent, bitangent).normalize();//右手坐标系：t cross bi z向内
    normal = cross(bitangent, tangent).normalize(); //右手坐标系：bi cross t 向外 so法线贴图是右手坐标系

    mat<3, 3, float> tbn;
    tbn.set_col(0, tangent);
    tbn.set_col(1, bitangent);
    tbn.set_col(2, normal);

    return tbn;
}

void perspectiveDivide(Vec4f &gl_Vertex)
{
    gl_Vertex = gl_Vertex / gl_Vertex[3];
}

//透视校正插值
Vec2f perspectiveCorrect(Vec3f bar, Vec3f &fragPoses_depth_clipSpace, mat<2, 3, float> &texcoords_uv)
{
    for (int i = 0; i < 3; i++)
        bar[i] = fragPoses_depth_clipSpace[i] * bar[i];
    Vec2f uv = texcoords_uv * bar;
    uv = uv / (bar[0] + bar[1] + bar[2]);
    return uv;
}

float distance(Vec3f pt1, Vec3f pt2)
{
    Vec3f d = pt1 - pt2;
    return sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
}

Vec3f color2normal(Vec4i color)
{
    return Vec3f(color[0] / 255.f * 2.f - 1.f, color[1] / 255.f * 2.f - 1.f, color[2] / 255.f * 2.f - 1.f);
}

//sphere
Vec2f uvmapping(Vec3f pt)
{
    float u = 0.5 + atan2(pt.x, pt.z) / (2 * M_PI);
    float v = 0.5 - asin(pt.y) / M_PI;
    return Vec2f(u, v);
}

//tile
Vec2f uvmapping(Vec3f pt, int repeat)
{
    float u = 0.5 + atan2(pt.x, pt.z) / (2 * M_PI);
    float v = 0.5 - asin(pt.y) / M_PI;
    u = u * repeat;
    v = v * repeat;
    return Vec2f(u, v);
}

//等距柱状投影图
const Vec2f invAtan = Vec2f(0.1591, 0.3183);
Vec2f sphericalMap(Vec3f pt)
{
    pt.normalize();
    //Vec2f uv = Vec2f(atan2(pt.z, pt.x), asin(pt.y));
    float u = atan2(pt.z, pt.x);
    float v = asin(pt.y);
    u = u * invAtan.x + 0.5;
    v = v * invAtan.y + 0.5;

    return Vec2f(u, v);
}

//debug
void printColor(const Vec4i &color)
{
    std::cout << (int)color.x << " " << (int)color.y << " " << (int)color.z << " " << (int)color[3] << std::endl;
}

//色调映射+gamma校正
Vec3f ReinhardToneMapping(Vec4f &hdrColor)
{
    float gamma = 1.0 / 2.2;

    // Reinhard色调映射
    Vec3f mapped(hdrColor.x / (hdrColor.x + 1.f), hdrColor.y / (hdrColor.y + 1), hdrColor.z / (hdrColor.z + 1));
    //     std::cout<<gamma<<std::endl;

    // Gamma校正
    for (int i = 0; i < 3; i++)
        mapped[i] = pow(mapped[i], 1.f / 2.f);

    return mapped;
}

//BRDF--------------------------------------------------------------------------------------------------------
////F:菲涅尔方程
Vec3f fresnelSchlick(float cosTheta, Vec3f F0)
{
    // return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    Vec3f tmp(1.0 - F0.x, 1.0 - F0.y, 1.0 - F0.z);
    tmp = tmp * pow(1.0 - cosTheta, 5.0);
    return F0 + tmp;
}

////N:正态分布函数
float DistributionGGX(Vec3f N, Vec3f H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = std::max((N * H), 0.f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return nom / denom;
}

////D:几何函数
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(Vec3f N, Vec3f V, Vec3f L, float roughness)
{
    float NdotV = std::max((N * V), 0.f);
    float NdotL = std::max((N * L), 0.f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}