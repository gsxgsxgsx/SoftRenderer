#ifndef __DEPTH_SHADER_H__
#define __DEPTH_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

struct ShadowShader : public IShader
{
    Data *data;
    Image *buffer;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> fragPoses_screenSpace;

    ShadowShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

    void setData(Data *data_)
    {
        data = data_;
    }

    Data *getData()
    {
        return data;
    }


    virtual Vec4f vertex(int i, int j)
    {
        assert(data != nullptr);
        Vec4f gl_Vertex = embed<4>(data->vert(i, j));
        gl_Vertex = viewPort * projection * view * model * gl_Vertex;

        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p = fragPoses_screenSpace * bar;
        float width = viewPort[0][0] * 2;
        float height = viewPort[1][1] * 2;
        buffer->setBufferf(p.x / width, p.y / height, Vec4f(p.z, 0, 0, 0));
        
        // color = Vec4i(255, 255, 255, 255) * clamp((1 - p.z / depth), 0, 1); //？p.z<0与视锥裁剪有关
        color = Vec4i(255, 255, 255, 255) * (1 - p.z / depth); //？p.z<0与视锥裁剪有关
        return false;
    }
};

#endif