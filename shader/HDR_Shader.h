#ifndef __HDR_SHADER_H__
#define __HDR_SHADER_H__
#define GLEW_STATIC

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

struct HDRShader : public IShader
{
    Data *data;

    Texture *texture;
    int id_hdr;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<2, 3, float> texcoords;
    mat<3, 3, float> fragPoses_viewSpace;
    mat<3, 3, float> fragPoses_screenSpace;

    HDRShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

    ~HDRShader() {}

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
        Vec2f uv = sphericalMap(proj<3>(gl_Vertex));
        texcoords.set_col(j, uv);

        gl_Vertex = projection * view * model * gl_Vertex;
        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec2f uv = texcoords * bar;
        Vec4f hdr = texture->getTexture2Df(uv[0], uv[1], id_hdr);
        Vec3f ldr = ReinhardToneMapping(hdr);

        color = Vec4i((uint8_t)(ldr.x * 255), (uint8_t)(ldr.y * 255), (uint8_t)(ldr.z * 255), 255);
        //std::cout<<(int)color.x<<" "<<(int)color.y<<" "<<(int)color.z<<std::endl;
        return false;
    }
};

#endif