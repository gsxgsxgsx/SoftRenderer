#ifndef __MY_MATH_H__
#define __MY_MATH_H__
#include <random>
#include "geometry.h"
#include "data_input.h"

//math-----------------------------------------------------------------------------------------------

float distance(Vec3f pt1, Vec3f pt2);
Vec3f color2normal(Vec4i color); //？仿射变换
Vec2f uvmapping(Vec3f pt);
Vec2f uvmapping(Vec3f pt, int repeat);

void printColor(const Vec4i &color);
float radians(float angle);
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P);
float lerp(float x, float y, float t);
Vec3f lerp(Vec3f x, Vec3f y, float m);
float clamp(float x, float min, float max);
float smoothstep(float t1, float t2, float x);

Vec3f reflect(Vec3f e, Vec3f n);
Vec3f refract(Vec3f i, Vec3f n, float eta);

DataInput icosphere(int recursionLevel);

void perspectiveDivide(Vec4f &gl_Vertex);
Vec2f perspectiveCorrect(Vec3f bar, Vec3f &fragPoses_depth_clipSpace, mat<2, 3, float> &texcoords_uv);

//model-----------------------------------------------------------------------------------------------
void scale(float s1, float s2, float s3, Matrix &ModelMat);
void translate(float t1, float t2, float t3, Matrix &ModelMat);
void rotate(float angle, Vec3f axis, Matrix &ModelMat);

//view-----------------------------------------------------------------------------------------------
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);
Matrix rotate_camera(Vec3f eye, Vec3f center, Vec3f up, float pitch, float yaw);

//projection-----------------------------------------------------------------------------------------------
Matrix orthProjection(float l, float r, float b, float t, float f, float n);
Matrix projection(float fovy, float aspect, float n, float f);

//viewport-----------------------------------------------------------------------------------------------
Matrix viewport(int w, int h);

mat<3, 3, float> getTBN(Vec3f *pts, Vec2f *uv);

const float simple_vbo[] = {-0.5, -0.5, 0.0, -0.5, 0.5, 0.0, 0.5, 0.5, 0.0, 0.5, -0.5, 0.0};
const int simple_ebo[] = {0, 1, 2, 0, 2, 3};

//面剔除需要特别定义顶点顺序
//旋转180度后？
//只适用于封闭物体？
//部分在背面？

//Vec4i ReinhardToneMapping(Vec4f &hdrColor);
Vec3f ReinhardToneMapping(Vec4f &hdrColor);

Vec3f fresnelSchlick(float cosTheta, Vec3f F0);
float DistributionGGX(Vec3f N, Vec3f H, float roughness);

float GeometrySchlickGGX(float NdotV, float roughness);

float GeometrySmith(Vec3f N, Vec3f V, Vec3f L, float roughness);

Vec2f sphericalMap(Vec3f pt);

//随机数
static std::random_device rd;  // Will be used to obtain a seed for the random number engine
static std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
static std::uniform_real_distribution<> disxy(-1.0, 1.0);
static std::uniform_real_distribution<> disz(0.0, 1.0);

#endif