#ifndef __SIMPLE_SHADER_H__
#define __SIMPLE_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/geometry.h"
#include "../include/texture.h"

struct SimpleShader : public IShader
{
    Data *data;
    Texture *texture;
    int id_tex;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<2, 3, float> texcoords2D;
    mat<3, 3, float> fragPoses_screenSpace;

    Vec3f fragPoses_depth_clipSpace;

    Vec2f *uvBuffer;

    SimpleShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

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
        gl_Vertex = projection * view * model * gl_Vertex;
        //fragPoses_depth_clipSpace[j] = 1 / gl_Vertex[2]; //透视插值

        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        //texcoords2D.set_col(j, data->uv(i, j));
        //fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        color = WHITE;
        
        /*mat<4, 3, float> colors;
        colors.set_col(0, RED);
        colors.set_col(1, GREEN);
        colors.set_col(2, BLUE);
        Vec4f colorf = colors * bar;
        color = Vec4i(colorf[0], colorf[1], colorf[2], colorf[3]);*/

        //Vec2f uv = texcoords2D * bar;

        // Vec3f p_ss = fragPoses_screenSpace * bar;
        //Vec2f uv = perspectiveCorrect(bar, fragPoses_depth_clipSpace, texcoords2D);

        //color = texture->getTexture2Di(uv[0], uv[1], id_tex, BILINEAR);
        //color = texture->getTexture2Di(uv[0], uv[1], id_tex);
        //color = texture->getTexture2Di_midmap(uv,(round)(p_ss.x), (round)(p_ss.y), id_tex, uvBuffer);

        return false;
    }
};

#endif