#ifndef __OMNI_HDR_SHADER_H__
#define __OMNI_HDR_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"
enum CTYPE
{
    HDR,
    LDR,
};
struct GenSkyboxShader : public IShader
{
    Data *data;
    Texture *texture;
    Image *image;

    CTYPE type;
    int id;
    int id_hdr;
    int id_ldr;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> fragPoses_screenSpace;
    mat<2, 3, float> texcoords;

    float width;
    float height;

    GenSkyboxShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_)
    {
        width = viewPort[0][0] * 2;
        height = viewPort[1][1] * 2;
    };

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
        Vec2f uv = sphericalMap(proj<3>(gl_Vertex));
        texcoords.set_col(j, uv);

        gl_Vertex = projection * view * model * gl_Vertex;
        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p_ss = fragPoses_screenSpace * bar;
        Vec2f uv = texcoords * bar;

        if (type == LDR)
        {
            color = texture->getTexture2Di(uv[0], uv[1], id,BILINEAR);
            image->setBuffer((round)(p_ss.x), (round)(p_ss.y), color);
            
            //std::cout<<(round)(p_ss.x)<<" "<< (round)(p_ss.y)<<std::endl;
        }
        else if (type == HDR)
        {

            Vec4f hdr = texture->getTexture2Df(uv[0], uv[1], id);
            image->setBufferf(p_ss.x / (float)width, p_ss.y / (float)height, hdr);
            Vec3f ldr = ReinhardToneMapping(hdr);
            color = Vec4i((uint8_t)(ldr.x * 255), (uint8_t)(ldr.y * 255), (uint8_t)(ldr.z * 255), 255);
        }

        //printColor(color);
    }
};

#endif
