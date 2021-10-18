
#ifndef __DATA_H__
#define __DATA_H__
#include "geometry.h"

struct Data
{
    std::vector<Vec3f> verts_;              //顶点
    std::vector<std::vector<Vec3i>> faces_; //面 vertex/uv/normal
    std::vector<Vec3f> norms_;              //法线
    std::vector<Vec2f> uv_;                 //纹理坐标

    //virtual Data(){} //constructors cannot be declared 'virtual'
    Data(){};
    virtual ~Data(){};

    virtual int nverts() = 0;
    virtual int nfaces() = 0;
    virtual Vec3f normal(int iface, int nthvert) = 0;
    //virtual Vec3f tangent_normal(Vec2f uvf) = 0;
    virtual Vec3f vert(int i) = 0;
    virtual Vec3f vert(int iface, int nthvert) = 0;
    virtual Vec2f uv(int iface, int nthvert) = 0;
    virtual std::vector<int> face(int idx) = 0;
};
#endif