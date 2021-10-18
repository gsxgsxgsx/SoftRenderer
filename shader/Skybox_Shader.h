#ifndef __SKYBOX_SHADER_H__
#define __SKYBOX_SHADER_H__

#include "shader.h"
#include <iostream>
#include <vector>

#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/geometry.h"
#include "../include/texture.h"

struct SkyboxShader : public IShader
{
    Data *data;

    Texture *texture;
    int id_;

    CTYPE type;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> texcoords3D;

    Vec3f eyePos;

    SkyboxShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

    ~SkyboxShader() {}

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

    void perspectiveDivide(Vec4f &gl_Vertex)
    {
        gl_Vertex = gl_Vertex / gl_Vertex[3];
    }

    virtual Vec4f vertex(int i, int j)
    {
        assert(data != nullptr);

        Vec4f gl_Vertex = embed<4>(data->vert(i, j));
        texcoords3D.set_col(j, proj<3>(gl_Vertex));

        Vec3f n = data->normal(i, j).normalize();
        Vec3f v = (eyePos - proj<3>(gl_Vertex));
        if (n * v > 0)
        {
            // std::cout << "failed" << std::endl;
            // return Vec4f(__FLT_MIN__, -1, depth, -1);
        }

        for (int i = 0; i < 3; i++)
            view[i][3] = 0; //移除位移部分

        gl_Vertex = projection * view * gl_Vertex;

        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        // gl_Vertex[2] = depth;//须视锥裁剪才可开启

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f uv = texcoords3D * bar;

        if (type == LDR)
        {
            //uv = uv * (-1.f);
            color = texture->getTexture3Di(uv, id_);
        }
        else if (type == HDR)
        {
            Vec4f hdr = texture->getTexture3Df(uv, id_);
            Vec3f ldr = ReinhardToneMapping(hdr);
            color = Vec4i((uint8_t)(ldr.x * 255), (uint8_t)(ldr.y * 255), (uint8_t)(ldr.z * 255), 255);
        }

        return false;
    }
};

#endif