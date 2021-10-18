#ifndef __AO_SHADER_H__
#define __AO_SHADER_H__
#define GLEW_STATIC
#include <glm/gtc/type_ptr.hpp>

#include <time.h>
#include <random>
#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

struct AOShader : public IShader
{
    Data *data;

    Texture *texture;
    Image *buffer;
    int id_tn;
    int id_lindepth;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<2, 3, float> texcoords;
    mat<3, 3, float> fragPoses_viewSpace;
    mat<3, 3, float> fragPoses_screenSpace;
    float width;
    float height;

    Vec3f fragPoses_depth_clipSpace;

    int kernelSize = 64;
    int noiseSize = 16;
    float radius = 1;
    Vec3f *samples;
    Vec3f *randoms;

    float occlusion;

    AOShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_)
    {
        randoms = new Vec3f[noiseSize];
        samples = new Vec3f[kernelSize];

        getSamples(samples);
        getRandoms(randoms);

        width = viewPort[0][0] * 2;
        height = viewPort[1][1] * 2;
    };

    ~AOShader()
    {
        delete[] samples;
        delete[] randoms;
    }

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
        Vec4f gl_Vertex = embed<4>(data->vert(i, j));

        gl_Vertex = view * model * gl_Vertex;
        fragPoses_viewSpace.set_col(j, proj<3>(gl_Vertex)); //观察空间位置

        gl_Vertex = projection * gl_Vertex;
        fragPoses_depth_clipSpace[j] = 1.f / gl_Vertex[2];

        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;
        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        Vec2f uv = data->uv(i, j);
        texcoords.set_col(j, uv);

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p_screenSpace = fragPoses_screenSpace * bar;
        Vec3f origin = fragPoses_viewSpace * bar;
        Vec2f uv = perspectiveCorrect(bar, fragPoses_depth_clipSpace, texcoords);

        //Vec3f normal = texture->getTannorm(uv[0], uv[1]).normalize(); //切线空间法线
        Vec3f normal = color2normal(texture->getTexture2Di(uv[0], uv[1], id_tn)).normalize(); //切线空间法线

        srand((unsigned int)(time(NULL)));

        Vec3f randomVec = randoms[(random()) % noiseSize]; //随机性会带来噪声，需要引入模糊

        Vec3f tangent = (randomVec - normal * (randomVec * normal)).normalize();
        Vec3f bitangent = cross(normal, tangent);
        mat<3, 3, float> TBN;
        TBN.set_col(0, tangent);
        TBN.set_col(1, bitangent);
        TBN.set_col(2, normal);

        occlusion = 0.0;
        for (int i = 0; i < kernelSize; i++)
        {
            Vec3f sample = TBN * samples[i];   // From tangent to view-space
            sample = origin + sample * radius; //view space

            Vec4f offset = embed<4>(sample);
            offset = projection * offset;
            perspectiveDivide(offset);
            offset = viewPort * offset;

            float sampleDepth = texture->getTexture2Df(offset.x / width, offset.y / height, id_lindepth).x; //是取样点对应的screen coord的有效线性深度值

            if (sampleDepth == __FLT_MAX__ || sampleDepth == -__FLT_MAX__)
                continue; //边缘/背景影响

            // range check & accumulate
            float rangeCheck = smoothstep(0.0, radius, radius / abs(origin.z - sampleDepth));
            occlusion += (-sampleDepth <= -sample.z ? 1.0 : 0.0) * rangeCheck; //周围采样深度小于当前深度，凹，遮蔽因子增加
        }
        //occlusion = 1.0 - (occlusion / kernelSize); //可视化
        occlusion = (occlusion / kernelSize); //屏蔽因子越大，环境光越少

        color = Vec4i((1 - occlusion) * 255, (1 - occlusion) * 255, (1 - occlusion) * 255, 255);

        buffer->setBufferf(p_screenSpace.x / width, p_screenSpace.y / height, occlusion);
        return false;
    }

    void getSamples(Vec3f *samples)
    {
        for (int i = 0; i < kernelSize; i++)
        {
            Vec3f sample(
                disxy(gen),
                disxy(gen),
                disz(gen));
            sample.normalize();
            sample = sample * disz(gen); //scale
                                         // sample = sample * disxy(gen); //scale
            float scale = (float)i / (float)kernelSize;
            scale = lerp(0.1f, radius, scale * scale);
            sample = sample * scale;
            samples[i] = sample;
        }
    }

    void getRandoms(Vec3f *randoms)
    {
        for (int i = 0; i < noiseSize; i++)
        {
            Vec3f noise(
                disxy(gen),
                disxy(gen),
                0.0f);
            randoms[i] = noise;
        }
    }
};

#endif