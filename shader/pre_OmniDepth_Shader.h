#ifndef __OMNI_DEPTH_SHADER_H__
#define __OMNI_DEPTH_SHADER_H__
/*
#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

struct OmniShadowShader : public IShader
{
    Data *data;
    Texture *texture;
    Image *image;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> fragPoses_screenSpace;
    mat<3, 3, float> fragPoses_worldSpace;

    float farplane; //大
    Vec3f light_pos;

    OmniShadowShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

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
        fragPoses_worldSpace.set_col(j, proj<3>(model * gl_Vertex));

        gl_Vertex = projection * view * model * gl_Vertex;
        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p = fragPoses_screenSpace * bar;

        Vec3f p_worldspace = fragPoses_worldSpace * bar;
        float lightdistance = distance(p_worldspace, light_pos);
        lightdistance = lightdistance / farplane;

        image->setVal(p.x, p.y, lightdistance);

        color = Vec4i(255, 255, 255, 255) * (1 - lightdistance); //？p.z<0与视锥裁剪有关
        color[3] = 255;
        return false;
    }
};

#endif

*/