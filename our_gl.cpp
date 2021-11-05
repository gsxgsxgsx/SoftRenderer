#include "include/math.h"
#include "include/our_gl.h"
#include "include/fbuffer.h"
#include "include/data_input.h"

#include "shader/shader.h"
#include <stdint.h>
#include <limits>

void triangle(Vec4f *pts, IShader &shader, FrameBuffer *fbuffer)
{
    bool iscull = false;
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (pts[i][3] == 0)
                iscull = true;
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]);
        }
    }
    //  if (iscull)
    //    return;
    Vec2i P;
    Vec4i color;
    //简单裁剪
    /*for (int i = 0; i < 2; i++)
    {
        bboxmin[i] = std::max(0.f, bboxmin[i]);
        bboxmax[i] = std::min((float)SCR_WIDTH, bboxmax[i]);
    }*/
      bboxmin.x = std::max(0.f, bboxmin.x);
      bboxmin.y = std::max(0.f, bboxmin.y);
      bboxmax.x = std::min((float)SCR_WIDTH, bboxmax.x);
      bboxmax.y = std::min((float)SCR_HEIGHT, bboxmax.y);

    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f c = barycentric(proj<2>(pts[0]), proj<2>(pts[1]), proj<2>(pts[2]), proj<2>(P));
            float frag_depth = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            // std::cout << pts[0] << " " << pts[1] << " " << pts[2] << std::endl;
            int idx = round(P.y) * SCR_WIDTH + round(P.x);
            if (idx < 0 || idx >= SCR_WIDTH * SCR_HEIGHT)
            {
                continue;
            }

            //当前深度小于等于zbuffer，通过深度测试
            //重心全为正数说明在三角形内部
            if (c.x < 0 || c.y < 0 || c.z < 0 || fbuffer->zBuffer[idx] < frag_depth)
            {
                continue;
            }
            bool discard = shader.fragment(c, color);
            if (!discard)
            {
                fbuffer->zBuffer[idx] = frag_depth;
                fbuffer->setColor(P.x, P.y, color);

                ////
               // fbuffer->uvBuffer[idx]=
            }
        }
    }
}

void line(Vec4f *pts, IShader &shader, FrameBuffer *fbuffer)
{
    for (int i = 0; i < 3; i++)
    {
        int x0 = pts[i][0], y0 = pts[i][1], x1 = pts[(i + 1) % 3][0], y1 = pts[(i + 1) % 3][1];

        bool steep = false;
        if (std::abs(x0 - x1) < std::abs(y0 - y1))
        {
            std::swap(x0, y0);
            std::swap(x1, y1);
            steep = true;
        }
        if (x0 > x1)
        {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        Vec4i color(222, 222, 222, 1);
        shader.fragment(Vec3f(0, 0, 0), color);

        for (int x = x0; x <= x1; x++)
        {
            float t = (x - x0) / (float)(x1 - x0);
            int y = y0 * (1. - t) + y1 * t;

            if (steep)
                fbuffer->setColor(y, x, color);
            else
                fbuffer->setColor(x, y, color);

            //   std::cout << x << " " << y << std::endl;
        }
    }
}

//正向渲染--------------------------------------------------------------------------------------------------
void render(IShader &shader, FrameBuffer *fbuffer, Render_Mode mode)
{
    for (int i = 0; i < shader.getData()->nfaces(); i++)
    {
        Vec4f screen_coords[3];
        // bool iscull = false;
        for (int j = 0; j < 3; j++)
        {
            if (screen_coords[j][0] == __FLT_MIN__)
                return;
            screen_coords[j] = shader.vertex(i, j);
        }
        switch (mode)
        {
        case LINE:
            line(screen_coords, shader, fbuffer);
            break;
        case TRIANGLE:
            triangle(screen_coords, shader, fbuffer);
            break;
        }
    }
}
/*
int genSkybox(std::vector<Image> &maps,不可父类shader，除非传入、传出数据分开)
{
    std::vector<Vec3f> dirs = {
        Vec3f(1, 0, 0),
        Vec3f(-1, 0, 0),
        Vec3f(0, 1, 0),
        Vec3f(0, -1, 0),
        Vec3f(0, 0, 1),
        Vec3f(0, 0, -1)};

    std::vector<Vec3f> ups = {
        Vec3f(0, -1, 0),
        Vec3f(0, -1, 0),
        Vec3f(0, 0, 1),
        Vec3f(0, 0, -1),
        Vec3f(0, -1, 0),
        Vec3f(0, -1, 0)};

    std::vector<std::string> paths = {
        "+x.png",
        "-x.png",
        "+y.png",
        "-y.png",
        "+z.png",
        "-z.png",
    };

    std::vector<Matrix> viewmats;
    for (int i = 0; i < 6; i++)
        viewmats.push_back(lookat(Vec3f(0, 0, 0), dirs[i], ups[i]));

    Matrix projMat_sp = projection(90.f, SCR_WIDTH / SCR_HEIGHT, 5, -10.f);
    for (int i = 0; i < 6; i++)
    {
        oshader.view = viewmats[i];
        oshader.image = &hdrmaps[i];
        render(oshader, &fbuffer, TRIANGLE);

        int id_ = texture.setTexture(hdrmaps[i]);
        if (i == 0)
            id_skybox_hdr = id_;

        fbuffer.clearBuffer();

        //stbi_flip_vertically_on_write(true);
        //stbi_write_png(paths[i].c_str(), SCR_WIDTH, SCR_HEIGHT, 4, fbuffer.colorBuffer, SCR_WIDTH * 4);
    }
}
*/