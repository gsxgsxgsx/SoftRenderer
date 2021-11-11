#ifndef __PRE_FILTER_SHADER_H__
#define __PRE_FILTER_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

struct prefilterShader : public IShader
{
    Data *data;
    Texture *texture;
    Image *image;

    int id;
    int id_hdr;
    int id_ldr;

    float roughness;
    int id_start;
    int level; //l*6+start

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> fragPoses_screenSpace;

    mat<3, 3, float> texcoords3D;

    float width;
    float height;

    prefilterShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_)
    {
        width = viewPort[0][0] * 2;
        height = viewPort[1][1] * 2;
    };

    void setData(Data *data_)
    {
        data = data_;
    }

    Data *getData()
    {
        return data;
    }

    void setTexture(Texture *tex_)
    {
        texture = tex_;
    }

    virtual Vec4f vertex(int i, int j)
    {
        assert(data != nullptr);
        Vec4f gl_Vertex = embed<4>(data->vert(i, j));
        texcoords3D.set_col(j, proj<3>(gl_Vertex));

        //for (int i = 0; i < 3; i++)
        //  view[i][3] = 0;

        gl_Vertex = projection * view * model * gl_Vertex;
        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p_ss = fragPoses_screenSpace * bar;
        Vec3f uv3d = texcoords3D * bar;
        Vec2f uv = sphericalMap(uv3d.normalize());

        //ldr中读取
        //计算后输入到hdr中
        color = texture->getTexture2Di(uv[0], uv[1], id, BILINEAR);

        // Vec4f hdr = texture->getTexture2Df(uv[0], uv[1], id);
        //image->setBufferf(p_ss.x / (float)width, p_ss.y / (float)height, hdr);
        //Vec3f ldr = ReinhardToneMapping(hdr);
        //color = Vec4i((uint8_t)(ldr.x * 255), (uint8_t)(ldr.y * 255), (uint8_t)(ldr.z * 255), 255);

        Vec3f N = uv3d.normalize();
        Vec3f R = N;
        Vec3f V = R;

        const uint SAMPLE_COUNT = 512;//256;//1024u;
        float totalWeight = 0.0;
        Vec3f prefilteredColor = Vec3f(0.0);
        for (uint i = 0u; i < SAMPLE_COUNT; ++i)
        {
            Vec2f Xi = Hammersley(i, SAMPLE_COUNT);
            Vec3f H = ImportanceSampleGGX(Xi, N, roughness);
            Vec3f L = reflect(V, H);

            float NdotL = std::max(N * L, 0.f);
            if (NdotL > 0.0)
            {
                //Vec4i tmpcolor = texture->getTexture2Di(uv[0], uv[1], id, BILINEAR);
                Vec4f hdr = texture->getTexture2Df(uv[0], uv[1], id);
                prefilteredColor = prefilteredColor + proj<3>(hdr * NdotL);

                totalWeight += NdotL;
            }
        }
        prefilteredColor = prefilteredColor / totalWeight;
        image->setBufferf(p_ss.x / (float)width, p_ss.y / (float)height, embed<4>(prefilteredColor));

        return false;
    }

    float RadicalInverse_VdC(uint bits)
    {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10; // / 0x100000000
    }
    // ----------------------------------------------------------------------------
    Vec2f Hammersley(uint i, uint N)
    {
        return Vec2f(float(i) / float(N), RadicalInverse_VdC(i));
    }

    Vec3f ImportanceSampleGGX(Vec2f Xi, Vec3f N, float roughness)
    {
        float a = roughness * roughness;

        float phi = 2.0 * M_PI * Xi.x;
        float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        // from spherical coordinates to cartesian coordinates
        Vec3f H;
        H.x = cos(phi) * sinTheta;
        H.y = sin(phi) * sinTheta;
        H.z = cosTheta;

        // from tangent-space vector to world-space sample vector
        Vec3f up = abs(N.z) < 0.999 ? Vec3f(0.0, 0.0, 1.0) : Vec3f(1.0, 0.0, 0.0);
        Vec3f tangent = cross(up, N).normalize();
        Vec3f bitangent = cross(N, tangent);

        Vec3f sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
        return sampleVec.normalize();
    }
};

#endif
