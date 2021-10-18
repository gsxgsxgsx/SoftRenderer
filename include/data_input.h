#ifndef __DATA_INPUT_H__
#define __DATA_INPUT_H__

#include "geometry.h"
#include "data.h"

//简单-data-----------------------------------------------------------------------------------------------
struct DataInput : public Data
{
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i>> faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;

    ~DataInput() {}

    DataInput() {}

    DataInput(float *vbo, int vbo_size, int *ebo, int ebo_size)
    {
        for (int i = 0; i < vbo_size; i++)
            verts_.push_back(Vec3f(vbo[i * 3 + 0], vbo[i * 3 + 1], vbo[i * 3 + 2]));

        for (int i = 0; i < ebo_size / 3; i++)
        {
            std::vector<Vec3i> face;
            for (int j = 0; j < 3; j++)
                face.push_back(Vec3i(ebo[i * 3 + j], -1, -1));

            faces_.push_back(face);
        }
    }

    DataInput(std::vector<Vec3f> verts, std::vector<std::vector<Vec3i>> faces) : verts_(verts), faces_(faces)
    {
    }

    int nverts();
    int nfaces();

    Vec3f normal(Vec2f uvf);
    Vec3f normal(int iface, int nthvert);
    Vec3f tangent_normal(Vec2f uvf);
    Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);
    Vec2f uv(int iface, int nthvert);
    float specular(Vec2f uvf);
    std::vector<int> face(int idx);

    Vec4i diffuse(Vec2f uvf);

    void addVertex(const Vec3f &v)
    {
        verts_.push_back(v);
    }

    void addVertex(const Vec3f &v, int idx)
    {
        verts_.insert(verts_.begin() + idx, v);
    }

    void addFace(const Vec3i &face_vt)
    {
        //i 0 Vec3i[v1,vt1,vn1]
        //  1
        //  2
        std::vector<Vec3i> face;
        for (int i = 0; i < 3; i++)
            face.push_back(Vec3i(face_vt[i], -1, -1));
        faces_.push_back(face);
    }
};

#endif
