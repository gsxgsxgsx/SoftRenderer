
#ifndef __F_BUFFER_H__
#define __F_BUFFER_H__
#include "our_gl.h"
#include "math.h"
//帧缓冲
class FrameBuffer
{
    //uint8_t *colorBuffer; //ldr
    float *colorBufferf; //hdr
    //float *zBuffer;

    int SCR_WIDTH;
    int SCR_HEIGHT;

public:
    uint8_t *colorBuffer; //返回常量指针？
        float *zBuffer;


    Vec2f *uvBuffer; //用于屏幕空间纹理坐标随屏幕坐标变换的导数（破坏封装性，仅模拟实现

    FrameBuffer(int SCR_WIDTH_, int SCR_HEIGHT_) : SCR_WIDTH(SCR_WIDTH_), SCR_HEIGHT(SCR_HEIGHT_)
    {
        zBuffer = new float[SCR_WIDTH * SCR_HEIGHT];
        colorBuffer = new uint8_t[SCR_WIDTH * SCR_HEIGHT * 4];

        uvBuffer = new Vec2f[SCR_WIDTH * SCR_HEIGHT];

        for (int i = 0; i < SCR_WIDTH; i++)
        {
            for (int j = 0; j < SCR_HEIGHT; j++)
            {
                colorBuffer[(j * SCR_WIDTH + i) * 4 + 0] = 0;   //(uint8_t)random() % 256;
                colorBuffer[(j * SCR_WIDTH + i) * 4 + 1] = 0;   //(uint8_t)random() % 256;
                colorBuffer[(j * SCR_WIDTH + i) * 4 + 2] = 0;   //(uint8_t)random() % 256;
                colorBuffer[(j * SCR_WIDTH + i) * 4 + 3] = 255; //png格式
            }
        }
        for (int i = SCR_WIDTH * SCR_HEIGHT; --i;)
        {
            zBuffer[i] = __FLT_MAX__;
        }

        for (int i = SCR_WIDTH * SCR_HEIGHT; --i;)
        {
            uvBuffer[i] = 0;
        }
    }

    ~FrameBuffer()
    {
        delete[] colorBuffer;
        delete[] zBuffer;
        delete[] uvBuffer;
    }

    void clearColor(Vec4i color = Vec4i(50, 50, 50, 255))
    {
        for (int i = 0; i < SCR_WIDTH; i++)
        {
            for (int j = 0; j < SCR_HEIGHT; j++)
            {
                setColor(i, j, color);
            }
        }
    }

    void clearZBuffer()
    {
        for (int i = SCR_WIDTH * SCR_HEIGHT; --i;)
            zBuffer[i] = __FLT_MAX__;
    }

    void clearBuffer()
    {
        clearColor();
        clearZBuffer();
    }

    void setColor(int x, int y, Vec4i color)
    {
        if (!(x < 0 || y < 0 || x > SCR_WIDTH || y > SCR_HEIGHT || (x + y * SCR_WIDTH) > SCR_WIDTH * SCR_HEIGHT))
        {
            for (int i = 0; i < 4; i++)
                colorBuffer[(x + y * SCR_WIDTH) * 4 + i] = color[i];
        }
    }

    Vec4i getColor(int x, int y)
    {
        Vec4i color(0, 0, 0, 255);
        if (x < 0 || y < 0 || x > SCR_WIDTH || y > SCR_HEIGHT)
            return color;
        for (int i = 0; i < 4; i++)
            color[i] = colorBuffer[(x + y * SCR_WIDTH) * 4 + i];
        return color;
    }

    //post-process------------------------------------------------------------------------

    void invertColor()
    {
        for (int i = 0; i < SCR_WIDTH; i++)
        {
            for (int j = 0; j < SCR_HEIGHT; j++)
            {
                Vec4i color = getColor(i, j);
                color = Vec4i(255, 255, 255, 2) - color;
                setColor(i, j, color);
            }
        }
    }

    //锐化
    void sharpenColor()
    {
        float kernel2[9] = {
            -1, -1, -1,
            -1, 9, -1,
            -1, -1, -1};
        float kernel[9] = {
            -2, -2, -2,
            -2, 15, -2,
            -2, -2, -2};
        processColorWithKernel(kernel);
    }

    void blurColor()
    {
        float kernel[9] = {
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16};
        float kernel_box[9] = {
            1.0 / 9, 1.0 / 9, 1.0 / 9,
            1.0 / 9, 1.0 / 9, 1.0 / 9,
            1.0 / 9, 1.0 / 9, 1.0 / 9};

        float kernel_gaussian[25] = {
            1.0 / 256,
            4.0 / 256,
            6.0 / 256,
            4.0 / 256,
            1.0 / 256,
            4.0 / 256,
            16.0 / 256,
            24.0 / 256,
            16.0 / 256,
            4.0 / 256,
            6.0 / 256,
            24.0 / 256,
            36.0 / 256,
            24.0 / 256,
            6.0 / 256,
            4.0 / 256,
            16.0 / 256,
            24.0 / 256,
            16.0 / 256,
            4.0 / 256,
            1.0 / 256,
            4.0 / 256,
            6.0 / 256,
            4.0 / 256,
            1.0 / 256,
        };
        processColorWithKernel(kernel_gaussian);
    }

    void processColorWithKernel(float *kernel)
    {
        for (int i = 0; i < SCR_WIDTH; i++)
        {
            for (int j = 0; j < SCR_HEIGHT; j++)
            {

                Vec4i color;
                int count = 0;
                for (int x = -2; x <= 2; x++)
                {
                    for (int y = -2; y <= 2; y++)
                    {
                        Vec4i sample(0, 0, 0, 255);
                        if (i + x >= 0 && i + x < SCR_WIDTH && j + y >= 0 && j + y < SCR_HEIGHT)
                            sample = getColor(i + x, j + y);
                        color = color + sample * (kernel[(x + 1) + (y + 1) * 5]);
                    }
                }
                setColor(i, j, color);
            }
        }
    }

    void blur()
    {
        for (int i = 0; i < SCR_WIDTH; i++)
        {
            for (int j = 0; j < SCR_HEIGHT; j++)
            {

                Vec4f color(0, 0, 0, 0);
                for (int x = -2; x < 2; x++)
                {
                    for (int y = -2; y < 2; y++)
                    {
                        int idx = (floor(y + j) * SCR_WIDTH + round(x + i)) * 4;
                        if (idx >= 0 && idx < SCR_WIDTH * SCR_HEIGHT * 4)
                        {
                            for (int k = 0; k < 4; k++)
                                color[k] += colorBuffer[idx + k];
                        }
                    }
                }
                color = color / (5.f * 5.f);
                for (int k = 0; k < 3; k++)
                    colorBuffer[int((round(i) + round(j) * SCR_WIDTH) * 4 + k)] = uint8_t(color[k]);
                colorBuffer[int((round(i) + round(j) * SCR_WIDTH) * 4 + 3)] = 255;
            }
        }
    }

    void copyBuffer(FrameBuffer &buffer)
    {
        for (int i = 0; i < SCR_WIDTH; i++)
        {
            for (int j = 0; j < SCR_HEIGHT; j++)
            {
                int idx = (i + j * SCR_WIDTH) * 4;
                for (int k = 0; k < 4; k++)
                {
                    if (buffer.colorBuffer[idx + k] <= 255 && buffer.colorBuffer[idx + k] > 25)
                    {
                        int c = std::min(255, (int)((float)buffer.colorBuffer[idx + k] * 1.2));
                        if (this->colorBuffer[idx + k] < c)
                            this->colorBuffer[idx + k] = c;
                    }
                }
            }
        }
    }
};

#endif