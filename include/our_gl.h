#ifndef __MY_SHADER_H__
#define __MY_SHADER_H__

#include "geometry.h"
#include "math.h"
#include "fbuffer.h"
#include "../shader/shader.h"

extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;
const float depth = 1.f; //z-buffer[0,depth]

const Vec4i WHITE = Vec4i(255, 255, 255, 255);
const Vec4i RED = Vec4i(255, 0, 0, 255);
const Vec4i GREEN = Vec4i(0, 255, 0, 255);
const Vec4i BLUE = Vec4i(0, 0, 255, 255);

enum Render_Mode
{
    LINE,
    TRIANGLE
};

enum Lighting_Mode
{
    POINT,
    DIR
};

//生成天空盒-----------------------------------------------------------------------------------------------

const std::vector<Vec3f> cubemap_dirs = {
    Vec3f(1, 0, 0),
    Vec3f(-1, 0, 0),
    Vec3f(0, 1, 0),
    Vec3f(0, -1, 0),
    Vec3f(0, 0, 1),
    Vec3f(0, 0, -1)};

const std::vector<Vec3f> cubemap_ups = {
    Vec3f(0, -1, 0),
    Vec3f(0, -1, 0),
    Vec3f(0, 0, 1),
    Vec3f(0, 0, -1),
    Vec3f(0, -1, 0),
    Vec3f(0, -1, 0)};

const std::vector<std::string> cubemap_output_paths = {
    "+x.png",
    "-x.png",
    "+y.png",
    "-y.png",
    "+z.png",
    "-z.png",
};

//pipeline-----------------------------------------------------------------------------------------------

void triangle(Vec4f *pts, IShader &shader, FrameBuffer &fbuffer, float *zbuffer);
void line(Vec4f *pts, IShader &shader, FrameBuffer *fbuffer);
void render(IShader &shader, FrameBuffer *fbuffer, Render_Mode mode);

//simple light source-----------------------------------------------------------------------------------------------
struct DirLight
{
    Vec3f lightDir;
    Vec4i color;

    DirLight(Vec3f lightDir_, Vec4i color_) : lightDir(lightDir_), color(color_) {}
};

struct PointLight
{
    Vec3f pos;
    Vec4i color;
    Vec4f fcolor;

    float constant;
    float linear;
    float quadratic;
//1, 0.22, 0.2
    PointLight(Vec3f lightPos_, Vec4i color_, float constant_, float linear_, float quadratic_) : pos(lightPos_), color(color_), constant(constant_), linear(linear_), quadratic(quadratic_) {}
    PointLight(Vec3f lightPos_, Vec4f fcolor_, float constant_, float linear_, float quadratic_) : pos(lightPos_), fcolor(fcolor_), constant(constant_), linear(linear_), quadratic(quadratic_) {}

    float getAttenuation(const Vec3f &fragPos)
    {
        Vec3f d = pos - fragPos;
        float distance = sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
        return 1.f / (constant + linear * distance + quadratic * (distance * distance));
    }
};

#endif
