#ifndef __PBR_SHADER_H__
#define __PBR_SHADER_H__

#include "shader.h"
#include "../include/our_gl.h"
#include "../include/math.h"
#include "../include/texture.h"

struct PBRShader : public IShader
{
    Data *data;
    Texture *texture;
    Image *buffer;
    int id_ao = -1;     //ao
    int id_albedo = -1; //albedo:diffuse color
    int id_tn = -1;     //tagnent normal
    int id_m = -1;      //material
    int id_r = -1;      //roughness

    int id_ibl_irr = -1;
    int id_ibl_spec = -1;
    int id_ibl_brdf = -1;

    float roughness = 0;
    float metallic = 0;

    PointLight *plight;

    mat<4, 4, float> model;
    mat<4, 4, float> view;
    mat<4, 4, float> projection;
    mat<4, 4, float> viewPort;

    mat<3, 3, float> fragPoses_screenSpace;
    mat<3, 3, float> fragPoses_worldSpace;
    //计算tbn
    Vec3f fragPoses_modelSpace[3];
    Vec2f uvs[3];

    mat<3, 3, float> normals;
    mat<2, 3, float> texcoords;
    Vec3f eyePos;

    PBRShader(mat<4, 4, float> model_, mat<4, 4, float> view_, mat<4, 4, float> projection_, mat<4, 4, float> viewPort_) : model(model_), view(view_), projection(projection_), viewPort(viewPort_){};

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

        //uvmapping
        Vec2f uv = uvmapping(proj<3>(gl_Vertex), 3);
        texcoords.set_col(j, uv);

        //tbn
        fragPoses_modelSpace[j] = proj<3>(gl_Vertex);
        uvs[j] = uv;

        gl_Vertex = model * gl_Vertex;
        fragPoses_worldSpace.set_col(j, proj<3>(gl_Vertex));

        gl_Vertex = projection * view * gl_Vertex;
        perspectiveDivide(gl_Vertex);
        gl_Vertex = viewPort * gl_Vertex;

        normals.set_col(j, data->normal(i, j));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, Vec4i &color)
    {
        Vec2f uv = texcoords * bar;

        float ao = 1;

        if (id_r > -1)
            roughness = texture->getTexture2Di(uv[0], uv[1], id_r).x / 255.f;

        if (id_m > -1)
            metallic = texture->getTexture2Di(uv[0], uv[1], id_m).x / 255.f;

        if (id_ao > -1)
            ao = (texture->getTexture2Di(uv[0], uv[1], id_ao).x) / 255.f;

        //if (id_albedo > -1)
        Vec4i alc = texture->getTexture2Di(uv[0], uv[1], id_albedo);          //[0,255] hdr:[1,inf_max] ldr[0,1]
        Vec3f albedo = Vec3f(alc[0] / 255.f, alc[1] / 255.f, alc[2] / 255.f); //反照率，只包含表面的颜色（或者折射吸收系数）// albedo = Vec3f(pow(albedo.x, 2.2f), pow(albedo.y, 2.2f), pow(albedo.z, 2.2f));//SRGB 需要转换到线性空间 RGB

        Vec3f N;
        //N=N * (-1.f);
        if (id_tn > -1)
        {
            N = color2normal(texture->getTexture2Di(uv[0], uv[1], id_tn));
            mat<3, 3, float> TBN = getTBN(fragPoses_modelSpace, uvs);
            N = (TBN * N).normalize();
        }
        else
        {
            N = (normals * bar).normalize(); //phong插值
        }

        mat<4, 4, float> u_MIT = (model).invert_transpose(); //?没有不等比变换就不需要，物体在右手坐标系，天空盒ndc在左手？
        N = proj<3>(u_MIT * embed<4>(N)).normalize();

        Vec3f p_ws = fragPoses_worldSpace * bar;

        Vec3f V = (eyePos - p_ws).normalize();
        Vec3f L = (plight->pos - p_ws).normalize();
        Vec3f H = (V + L).normalize();

        //直接光照--------------------------------------------------------------------------------

        float attenuation = plight->getAttenuation(p_ws);
        Vec3f radiance = proj<3>(plight->fcolor * attenuation); //用rgb表示的辐射度

        Vec3f F0(0.04, 0.04, 0.04);
        F0 = lerp(F0, albedo, metallic); //金属albedo
        Vec3f F = fresnelSchlick(std::max((H * V), 0.f), F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        Vec3f nominator = F * (NDF * G);
        float denominator = 4.0 * std::max((N * V), 0.f) * std::max((N * L), 0.f) + 0.001;

        Vec3f specular = nominator / denominator;

        Vec3f kS = F;
        Vec3f kD = Vec3f(1.0) - kS;
        kD = kD * (1.0 - metallic); //金属漫反射为0

        float NdotL = std::max((N * L), 0.f);

        Vec3f Lo;
        for (int i = 0; i < 3; i++)
            Lo[i] = (kD[i] * albedo[i] / M_PI + specular[i]) * radiance[i] * NdotL;

        //间接光照即环境光--------------------------------------------------------------------------------
        ////漫反射辐照度贴图

        Vec3f irradiance2 = proj<3>(texture->getTexture3Df(N, id_ibl_irr));
        Vec3f diffuse = Vec3f(irradiance2.x * albedo.x, irradiance2.y * albedo.y, irradiance2.z * albedo.z);

        ////镜面反射贴图
        // Vec3f n2=N * (-1.f);

        int id_spec = id_ibl_spec + 6 * (roughness / 0.25);

        Vec3f prefilteredColor = proj<3>(texture->getTexture3Df(N, id_spec)); //未完成
        Vec4i brdf = texture->getTexture2Di(std::max((N * V), 0.f), roughness, id_ibl_brdf);
        Vec2f envBRDF(brdf.x / 255.f, brdf.y / 255.f);

        F = fresnelSchlick(std::max((H * V), 0.f), Vec3f(roughness, roughness, roughness));

        Vec3f tmp = F * envBRDF.x + envBRDF.y;
        Vec3f specular2 = Vec3f(prefilteredColor.x * tmp.x, prefilteredColor.y * tmp.y, prefilteredColor.z * tmp.z);

        Vec3f ambient;
        for (int i = 0; i < 3; i++)
            ambient[i] = (kD[i] * diffuse[i] + specular2[i]) * ao * albedo[i];
            
        //直接光+间接光+色调映射--------------------------------------------------------------------------------

        Vec3f total = ambient + Lo;
        Vec4f hdrcolor(total.x, total.y, total.z, 1);

        Vec3f ldrcolor = ReinhardToneMapping(hdrcolor);
        color = Vec4i((uint8_t)(ldrcolor.x * 255), (uint8_t)(ldrcolor.y * 255), (uint8_t)(ldrcolor.z * 255), 255);

        return false;
    }
};
#endif