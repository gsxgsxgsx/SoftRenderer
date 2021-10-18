#ifndef __LINEAR_DEPTH_SHADER_H__
#define __LINEAR_DEPTH_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/geometry.h"
#include "../include/texture.h"

struct LinDepthShader : public IShader
{
    Data *data;
    Image *buffer;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> fragPoses_screenSpace;
    mat<3, 3, float> fragPoses_viewSpace;

    Vec3f tmp;

    float max;

    LinDepthShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

    void setData(Data *data_)
    {
        data = data_;
    }
    Data *getData()
    {
        return data;
    }

    void perspectiveDivide(Vec4f &gl_Vertex)
    {
        gl_Vertex = gl_Vertex / gl_Vertex[3];
    }

    virtual Vec4f vertex(int i, int j)
    {
        assert(data != nullptr);
        Vec4f gl_Vertex = embed<4>(data->vert(i, j));

        gl_Vertex = view * model * gl_Vertex;
        fragPoses_viewSpace.set_col(j, proj<3>(gl_Vertex));
        
       // float linear_depth = gl_Vertex.z;
        //tmp[j] = linear_depth;

        gl_Vertex = projection * gl_Vertex;
        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        // gl_Vertex[2] = -linear_depth;//没透视除法是背面,view空间是负的,不取反通不过深度测试
        //gl_Vertex[3] = linear_depth;

        //if (abs(linear_depth) > max)
        //    max = abs(linear_depth);

        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));


        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p_ss = fragPoses_screenSpace * bar;
        Vec3f p_vs = fragPoses_viewSpace * bar;
        
        float width = viewPort[0][0] * 2;
        float height = viewPort[1][1] * 2;

        buffer->setBufferf(p_ss.x/width, p_ss.y/height,p_vs.z);

        //可视化
        color = Vec4i(255, 255, 255, 255) *(p_ss.z);
        return false;
    }
};

#endif