#include "include/data_input.h"

int DataInput::nverts()
{
    return (int)verts_.size();
}

int DataInput::nfaces()
{
    return (int)faces_.size();
}

Vec3f DataInput::vert(int iface, int nthvert)
{
    return verts_[faces_[iface][nthvert][0]];
}

std::vector<int> DataInput::face(int idx)
{
    std::vector<int> face;
    for (int i = 0; i < (int)faces_[idx].size(); i++)
        face.push_back(faces_[idx][i][0]);
    return face;
}

Vec3f DataInput::vert(int i)
{
    return verts_[i];
}



Vec2f DataInput::uv(int iface, int nthvert)
{
    return uv_[faces_[iface][nthvert][1]];
}


Vec3f DataInput::normal(int iface, int nthvert)
{
    int idx = faces_[iface][nthvert][2];
    return norms_[idx].normalize();
}

