#ifndef __GLOW_SHADER_H__
#define __GLOW_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

struct GlowShader : public IShader
{
    Data *data;
    Texture *texture;
    int id_g;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;
    mat<4, 4, float> lightspace_mat;

    mat<2, 3, float> texcoords_uv;

    mat<4, 3, float> fragPoses_worldSpace;

    Vec3f fragPoses_depth_clipSpace;

    Vec2f texcoords[3];

    GlowShader(mat<4, 4, float> m, mat<4, 4, float> v, mat<4, 4, float> p, mat<4, 4, float> vp) : model(m), view(v), projection(p), viewPort(vp) {}
    ~GlowShader() {}

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
        Vec2f uv_ = data->uv(i, j);
        texcoords[j] = uv_;

        gl_Vertex = model * gl_Vertex;
        fragPoses_worldSpace.set_col(j, gl_Vertex);

        gl_Vertex = projection * view * gl_Vertex;
        fragPoses_depth_clipSpace[j] = 1 / gl_Vertex[2]; //透视插值

        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        texcoords_uv.set_col(j, uv_);

        return gl_Vertex;
    }

    virtual bool
    fragment(Vec3f bar, Vec4i &color)
    {

        Vec2f uv = perspectiveCorrect(bar, fragPoses_depth_clipSpace, texcoords_uv);
        color =texture->getTexture2Di(uv[0], uv[1],id_g);

        if (color[0] == 0 && color[0] == 0 && color[0] == 0)
            return true;


        Vec3f fragPos = proj<3>(fragPoses_worldSpace * bar);

        float light_intensity = 3.2;

        for (int i = 0; i < 3; i++)
        {
            color[i] = std::min<uint8_t>(light_intensity * color[i], 255);
            //color[i] =255;// std::min<uint8_t>(light_intensity * color[i], 255);
        }

        return false;
    }
};

#endif