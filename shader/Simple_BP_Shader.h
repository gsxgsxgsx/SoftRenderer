#ifndef __SIMPLE_BLINN_PHONG_SHADER_H__
#define __SIMPLE_BLINN_PHONG_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

enum Shadow_Mode
{
    OFF,
    HARD,
    PCF
};

struct SimpleBPShader : public IShader
{
    Data *data;
    Texture *texture;
    int id_diff;
    int id_tn;
    int id_ao;
    int id_d;
    float width;
    float height;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;
    //平行光
    mat<4, 4, float> lightspace_mat;
    //点光源
    mat<4, 4, float> proj_lightspace;
    mat<4, 4, float> view_lightspace;
    mat<3, 3, float> uv_directions;

    mat<2, 3, float> texcoords_uv;
    mat<3, 3, float> normals;

    mat<4, 3, float> fragPoses_worldSpace;
    mat<4, 3, float> fragPoses_lightSpace; //世界坐标到光空间坐标变换矩阵
    mat<3, 3, float> fragPoses_screenSpace;

    Vec3f fragPoses_depth_clipSpace;

    //计算tbn
    Vec3f fragPoses_modelSpace[3];
    Vec2f texcoords[3];

    PointLight *plight;
    DirLight *dlight;
    Lighting_Mode l_mode;
    //Shadow_Mode s_mode = HARD;
    Shadow_Mode s_mode = PCF;

    Vec3f eyePos;
    float bias;

    SimpleBPShader(mat<4, 4, float> m, mat<4, 4, float> v, mat<4, 4, float> p, mat<4, 4, float> vp) : model(m), view(v), projection(p), viewPort(vp) {}

    ~SimpleBPShader() {}

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

    void setLight(PointLight *plight_)
    {
        plight = plight_;
        l_mode = POINT;
    }

    void setLight(DirLight *dlight_)
    {
        dlight = dlight_;
        l_mode = DIR;
    }

    virtual Vec4f vertex(int i, int j)
    {
        assert(data != nullptr);

        Vec4f gl_Vertex = embed<4>(data->vert(i, j));
        fragPoses_modelSpace[j] = proj<3>(gl_Vertex);
        Vec2f uv_ = data->uv(i, j);
        texcoords[j] = uv_;

        gl_Vertex = model * gl_Vertex;
        fragPoses_worldSpace.set_col(j, gl_Vertex);
        //平行光
        fragPoses_lightSpace.set_col(j, lightspace_mat * gl_Vertex);
        //点光源
        Vec4f gl_Vertex_lightspace = proj_lightspace * view_lightspace * gl_Vertex;
        perspectiveDivide(gl_Vertex_lightspace);

        gl_Vertex = projection * view * gl_Vertex;
        fragPoses_depth_clipSpace[j] = 1 / gl_Vertex[2]; //透视插值

        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        fragPoses_screenSpace.set_col(j, proj<3>(gl_Vertex));

        texcoords_uv.set_col(j, uv_);

        return gl_Vertex;
    }

    virtual bool
    fragment(Vec3f bar, Vec4i &color)
    {
        Vec3f p_ss = fragPoses_screenSpace * bar;
        Vec4f p_ls = fragPoses_lightSpace * bar;
        width = viewPort[0][0] * 2;
        height = viewPort[1][1] * 2;

        Vec2f uv = perspectiveCorrect(bar, fragPoses_depth_clipSpace, texcoords_uv);
        color = texture->getTexture2Di(uv[0], uv[1], id_diff);

        mat<3, 3, float> TBN = getTBN(fragPoses_modelSpace, texcoords);
        Vec3f n = color2normal(texture->getTexture2Di(uv[0], uv[1], id_tn)).normalize();
        // Vec3f test = TBN * n;
        n = TBN * n;

        Vec3f p_ws = proj<3>(fragPoses_worldSpace * bar);
        Vec3f l, r, e, half;
        float attenuation = 1, shadow = 1, occlusion = 0;

        if (l_mode == POINT)
            l = (plight->pos - p_ws).normalize();
        else
            l = (dlight->lightDir * (1)).normalize();
        r = reflect(l, n);
        e = (eyePos - p_ws).normalize();
        half = (l + e).normalize();

        if (l_mode == POINT)
            attenuation = plight->getAttenuation(p_ws);

        float diff = std::max(0.f, n * l);

        float specluarStrength = 0.9; //镜面反射系数
        float shinness = 256;         //反光度
        float spec = pow(std::max((half * n), 0.f), shinness);

        float ambient = 0.33;

        occlusion = texture->getTexture2Df(p_ss.x / width, p_ss.y / height, id_ao).x;

        bias = std::max(0.01 * (1.0 - (n * l)), 0.0015);
        //shadow = ominshadow_map(fragPos);
        shadow = shadow_pcf(p_ls);

        float light_intensity = (diff + spec) * attenuation * shadow + ambient * (1 - occlusion);
        //float light_intensity = 1 * (1 - occlusion);

        for (int i = 0; i < 3; i++)
        {
            //color[i] = std::min<uint8_t>(light_intensity * 255, 255.f);
            if (l_mode == POINT)
                color[i] = std::min<uint8_t>(light_intensity * color[i] * plight->color[i] / 255.f, 255);
            else
                color[i] = std::min<uint8_t>(light_intensity * color[i] * dlight->color[i] / 255.f, 255);
        }

        return false;
    }

    float shadow_hard(const Vec4f &p_lightSpace)
    {
        float shadow = 0;
        //  bias =0.015; //0.01;
        float depth = texture->getTexture2Df(p_lightSpace.x / width, p_lightSpace.y / height, id_d).x;

        shadow = 0.f + 1 * (depth + bias > p_lightSpace.z);
        return shadow;
    }

    float shadow_pcf(const Vec4f &p_lightSpace)
    {
        float shadow = 0;

        for (int x = -2; x <= 2; x++)
        {
            for (int y = -2; y <= 2; y++)
            {
                float depth = texture->getTexture2Df((x + p_lightSpace.x) / width, (y + p_lightSpace.y) / height, id_d).x;
                shadow += 0.f + (1 - 0.f) * (depth + bias > p_lightSpace.z);
            }
        }
        shadow /= 25.f;
        return shadow;
    }

    /*  float ominshadow_map(Vec3f fragPos)
    {
        Vec3f frag2light = fragPos - plight->pos;
        // std::cout<<frag2light<<std::endl;
        float depth = texture->getTexture3Df(frag2light);
        depth *= 10;
        float shadow = 0;
        shadow = 0.f + 1 * (depth + bias > distance(frag2light, Vec3f(0, 0, 0)));
        //        std::cout<<shadow<<std::endl;

        return shadow;
    }*/
};

#endif