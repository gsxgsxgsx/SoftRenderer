#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "geometry.h"

#include "data.h"

class Model : public Data
{
private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i>> faces_;
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;

public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f normal(int iface, int nthvert);
    Vec3f normal(Vec2f uv);
    Vec3f tangent_normal(Vec2f uvf);
    Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);
    Vec2f uv(int iface, int nthvert);
    float specular(Vec2f uv);
    std::vector<int> face(int idx);



};
#endif //__MODEL_H__
