#ifndef __SHADER_H__
#define __SHADER_H__

#include "../include/geometry.h"
#include "../include/our_gl.h"
#include "../include/data.h"

struct IShader
{
    Data *data;
    virtual void setData(Data *data_) = 0;
    virtual Data *getData() = 0;

    //virtual ~IShader()=0;
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, Vec4i &color) = 0;
};
//IShader::~IShader() {}

#endif