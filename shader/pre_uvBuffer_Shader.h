#ifndef __UV_BUFFER_SHADER_H__
#define __UV_BUFFER_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/geometry.h"
//#include "../include/texture.h"

struct UVShader : public IShader
{
    Data *data;
    //Texture *texture;
    //int id_tex;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<2, 3, float> texcoords2D;
    mat<3, 3, float> fragPoses_screenSpace;

    Vec3f fragPoses_depth_clipSpace;

    /////
    Vec2f *uvBuffer;

    UVShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

    void setData(Data *data_)
    {
        data = data_;
    }

    Data *getData()
    {
        return data;
    }

    /* void setTexture(Texture *tex_)
    {
        texture = tex_;
    }*/

    virtual Vec4f vertex(int i, int j)
    {
        assert(data != nullptr);
        Vec4f gl_Vertex = embed<4>(data->vert(i, j));
        gl_Vertex = projection * view * model * gl_Vertex;
        fragPoses_depth_clipSpace[j] = 1 / gl_Vertex[2];

        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        texcoords2D.set_col(j, data->uv(i, j));

        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p_ss = fragPoses_screenSpace * bar;
        Vec2f uv = perspectiveCorrect(bar, fragPoses_depth_clipSpace, texcoords2D);

        int idx = (round)(p_ss.x) + (round)(p_ss.y) * SCR_WIDTH;
        //int idx = (floor)(p_ss.x-0.5) + (floor)(p_ss.y-0.5) * SCR_WIDTH;
      
        if (idx >= 0 && idx < SCR_WIDTH * SCR_HEIGHT)
        {
            uvBuffer[idx] = uv;
        }

        //color=Vec4i(222);
        //color = texture->getTexture2Di(uv[0], uv[1], id_tex, BILINEAR);
        //color = texture->getTexture2Di(uv[0], uv[1], id_tex);

        return false;
    }
};

#endif