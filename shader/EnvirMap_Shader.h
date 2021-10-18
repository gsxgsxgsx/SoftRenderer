#ifndef __ENVIR_MAPPING_SHADER_H__
#define __ENVIR_MAPPING_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

enum Type
{
    REFLECT,
    REFRACT
};
const float AIR = 1.f;
const float WATER = 1.33f;
const float ICE = 1.309f;
const float GLASS = 1.52f;
const float DIAMOND = 2.42f;

struct EMShader : public IShader
{
    Data *data;

    Texture *texture;
    int idx_skybox;

    Type type;
    float material;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> uv_directions;
    mat<3, 3, float> normals;
    Vec3f eye;

    EMShader(mat<4, 4, float> m, mat<4, 4, float> v, mat<4, 4, float> p, mat<4, 4, float> vp, Type type_ = REFLECT) : model(m), view(v), projection(p), viewPort(vp), type(type_),material(1.f) {}

    ~EMShader() {}

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
        this->texture = tex_;
    }

    void setEye(Vec3f eyePos_)
    {
        this->eye = eyePos_;
    }

    void perspectiveDivide(Vec4f &gl_Vertex)
    {
        gl_Vertex = gl_Vertex / gl_Vertex[3];
    }

    virtual Vec4f vertex(int i, int j)
    {

        assert(data != nullptr);
        Vec4f gl_Vertex = embed<4>(data->vert(i, j));
        gl_Vertex = model * gl_Vertex;

        mat<4, 4, float> u_MIT = (model).invert_transpose();
        Vec3f n = proj<3>(u_MIT * embed<4>(data->normal(i, j))).normalize();

        Vec3f e = (proj<3>(gl_Vertex) - eye).normalize(); //视线
        Vec3f r;                                          // Vec3f r = (n * (n * e * 2.f) - e).normalize();    // reflected light
        if (type == REFLECT)
            r = reflect(e, n);
        else
            r = refract(e, n, 1.f / material); //r=r*(-1.f);
        uv_directions.set_col(j, r);

        gl_Vertex = projection * view * gl_Vertex;
        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        return gl_Vertex;
    }

    virtual bool
    fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f uv = uv_directions * bar;
        color = texture->getTexture3Di(uv,idx_skybox);
        return false;
    }
};

#endif