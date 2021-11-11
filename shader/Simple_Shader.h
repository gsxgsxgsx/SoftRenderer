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

        //std::cout <<"before: "<<gl_Vertex  << std::endl;

        gl_Vertex = projection * view * model * gl_Vertex;
        //fragPoses_depth_clipSpace[j] = 1 / gl_Vertex[2]; //透视插值
//std::cout <<"after pro: "<< gl_Vertex  << std::endl;

        perspectiveDivide(gl_Vertex);

        //std::cout <<"after per: "<< gl_Vertex  << std::endl;

        gl_Vertex = viewPort * gl_Vertex;

        //texcoords2D.set_col(j, data->uv(i, j));
        //fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {

        color = Vec4i((uint8_t)(255 * bar.x), (uint8_t)(255 * bar.y), (uint8_t)(255 * bar.z), 255);

        return false;
    }
};

#endif