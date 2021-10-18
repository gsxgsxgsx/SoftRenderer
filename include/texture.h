#ifndef _TEXTURE_H
#define _TEXTURE_H
#include <string>
#include <memory>
#include "geometry.h"

enum I_FORMAT
{
    FLOAT,
    UINT
};

enum Wrap_Mode
{
    NEAREST_NEIGHBOR,
    BILINEAR
};

class Image
{
    I_FORMAT format;
    //std::shared_ptr<uint8_t> color; //ldr
    uint8_t *color; //ldr
    float *buffer;  //hdr

    int width;
    int height;
    int n; //通道数
    int size;

public:
    Image() {}
    // Image(const Image &image)
    //{
    //  std::cout << "调用了拷贝构造函数" << std::endl;
    //}

    Image(int SCR_WIDTH, int SCR_HEIGHT, int n_ = 1, I_FORMAT format_ = FLOAT) : width(SCR_WIDTH), height(SCR_HEIGHT), format(format_)
    {
        n = n_;
        size = width * height * n;
        if (format == FLOAT)
        {
            buffer = new float[size];
            for (int i = 0; i < size; i++)
                buffer[i] = std::numeric_limits<float>::max();
        }
        else if (format == UINT)
        {
            color = new uint8_t[size];
            // color = std::shared_ptr<uint8_t>(new uint8_t[size]);
            for (int i = 0; i < size; i++)
                color[i] = 255;
        }
    }

    Image(std::string path)
    {
        loadImage(path);
    }

    ~Image()
    {
        //   std::cout << "调用了析构函数" << std::endl;

        ///if (format == UINT&&color!=nullptr)
        //delete[] color;
        //if (format == FLOAT)
        //delete[] buffer;
    }

    void loadImage(std::string path)
    {
        if (path.substr(path.size() - 3, 3) == "png")
            stbi_set_flip_vertically_on_load(true);

        if (path.substr(path.size() - 3, 3) == "hdr")
            buffer = stbi_loadf(path.c_str(), &width, &height, &n, 0);
        else
            color = stbi_load(path.c_str(), &width, &height, &n, 0); //string2const char* :c_str()
        size = width * height * n;

        std::cout << "load texture: " << path << "  " << width << " x " << height << " " << n << std::endl;
    }

    Vec4i readBuffer(int x, int y)
    {
        Vec4i res(0, 0, 0, 255);
        int idx = n * (x + y * width);
        if (!(color == nullptr || idx < 0 || idx >= size))
        {
            for (int k = 0; k < n; k++)
                res[k] = color[idx + k];
        }
        return res;
    }

    Vec4f readBufferf(int x, int y)
    {
        Vec4f res(0, 0, 0, 255);
        int idx = n * (x + y * width);
        if (!(buffer == nullptr || idx < 0 || idx >= size))
        {
            for (int k = 0; k < n; k++)
                res[k] = buffer[idx + k];
        }
        return res;
    }

    Vec4i
    getColor(float u, float v, Wrap_Mode mode = NEAREST_NEIGHBOR) //纹理坐标空间[0,1]
    {
        //tile
        if (u > 1)
            u = u - (int)u;
        if (v > 1)
            v = v - (int)v;

        Vec4i res;
        float i = u * width, j = v * height; //图片坐标空间[w,h]

        if (mode == NEAREST_NEIGHBOR)
        {
            res = readBuffer(round(i), round(j));
        }
        else if (mode == BILINEAR)
        {
            int x = floor(i - 0.5), y = floor(j - 0.5);
            float uf = i - 0.5 - x, vf = j - 0.5 - y;
            res = (1 - vf) * ((1 - uf) * readBuffer(x, y) + uf * readBuffer(x + 1, y)) + vf * ((1 - uf) * readBuffer(x, y + 1) + uf * readBuffer(x + 1, y + 1));
        }
        //printColor(res);
        return res;
    }

    Vec4f getColorf(float u, float v)
    {
        //tile
        //if (u > 1)
          //  u = u - (int)u;
        //if (v > 1)
          //  v = v - (int)v;

        Vec4f res;
        float i = u * width, j = v * height;

        res = readBufferf(round(i), round(j));
        return res;
    }

    void setBuffer(int x, int y, Vec4i val)
    {
        int idx = n * (x + y * width);

        if (!(color == nullptr || idx < 0 || idx >= size))
        {
            for (int k = 0; k < n; k++)
                color[idx + k] = val[k];
        }
    }

    void setBufferf(float u, float v, Vec4f val)
    {
        float i = u * width, j = v * height;
        int idx = n * (round(i) + round(j) * width);

        if (!(buffer == nullptr || idx < 0 || idx >= size))
        {
            for (int k = 0; k < n; k++)
                buffer[idx + k] = val[k];
        }
    }

    void setBufferf(float u, float v, float val)
    {
        float i = u * width, j = v * height;
        int idx = n * (round(i) + round(j) * width);

        if (!(buffer == nullptr || idx < 0 || idx >= size))
        {
            buffer[idx] = val;
        }
    }

    //ao
    void blur()
    {
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {

                float val = 0;
                for (int x = -2; x < 2; x++)
                {
                    for (int y = -2; y < 2; y++)
                    {
                        int idx = (y + j) * width + (x + i);
                        if (idx >= 0 && idx < width * height && buffer[idx] <= 1 && buffer[idx] >= 0)
                            val += buffer[idx];
                    }
                    val /= (4.f * 4.f);
                    buffer[i + j * width] = val;
                }
            }
        }
    }

    int getWidth()
    {
        return width;
    }

    int getHeight()
    {
        return height;
    }

    int getChannel()
    {
        return n;
    }

    I_FORMAT getImgType()
    {
        return format;
    }
};

class Texture
{
    std::vector<Image> images;

public:
    Texture() {}

    int loadTexture(std::string path)
    {
        Image image(path);
        images.push_back(image);

        return images.size() - 1;
    }

    int loadTexture_plusMipmap(std::string path, int level = 0)
    {
        Image image(path);
        images.push_back(image);
        int id_base = images.size() - 1;
        generateMidMap(level);
        return id_base;
    }

    int setTexture(Image buffer)
    {
        images.push_back(buffer);
        return images.size() - 1;
    }

    Vec4i getTexture2Di(float u, float v, int idx, Wrap_Mode mode = NEAREST_NEIGHBOR)
    {
        assert(idx >= 0 && idx < images.size());
        return images[idx].getColor(u, v, mode);
    }

    Vec4i getTexture2Di_midmap(Vec2f uv, int x, int y, int idx_base, Vec2f *uvBuffer, int level = 0, Wrap_Mode mode = NEAREST_NEIGHBOR)
    {
        Image base = images[idx_base];
        int width = base.getWidth();
        int height = base.getHeight();
        if (level == 0)
            level = floor(log2(std::min(width, height)));

        //计算dx/du
        Vec2f u1, u2, u3, u4;
#define idx(x, y) (x + SCR_WIDTH * y)
        if (idx(x, y) >= 0 && idx(x, y) < SCR_HEIGHT * SCR_WIDTH)
            u1 = uvBuffer[idx(x, y)];
        if (idx((x + 1), y) >= 0 && idx((x + 1), y) < SCR_HEIGHT * SCR_WIDTH)
            u2 = uvBuffer[idx((x + 1), y)];
        if (idx(x, (y + 1)) >= 0 && idx(x, (y + 1)) < SCR_HEIGHT * SCR_WIDTH)
            u3 = uvBuffer[idx(x, (y + 1))];

        float du1 = width * (u2.x - u1.x);
        float du2 = width * (u3.x - u1.x);

        float dv1 = height * (u2.y - u1.y);
        float dv2 = height * (u3.y - u1.y);

        float dx = 1.f, dy = 1.f;

        float d1 = std::max(abs(du1 / dx), abs(dv1 / dx));
        float d2 = std::max(abs(du2 / dy), abs(dv2 / dy));

        float dmin = std::min(d1, d2);
        float dmax = std::max(d1, d2);

        float lambda = log2(dmax);
        //std::cout << lambda << " " << std::endl;

        //nearest
        int d = std::max(idx_base, idx_base + std::min((int)(round(lambda)), level - 1));
        //三线性插值
        Vec4i res(0, 0, 0, 255);
        //std::cout << d << " " << std::endl;
        if (d >= 0 && d < images.size())
        {
            Image sub = images[d]; //浅拷贝
            int width = base.getWidth();
            int height = base.getHeight();
            res = sub.getColor(uv.x, uv.y, BILINEAR);
        }
        return res;
    }

    Vec4f getTexture2Df(float u, float v, int idx)
    {
        assert(idx >= 0 && idx < images.size());
        return images[idx].getColorf(u, v);
    }

    int getFaceidx(Vec3f &pts, float &u, float &v)
    {
        float x = pts[0];
        float y = pts[1];
        float z = pts[2];
        int idx = 0;
        float maxd = std::max(abs(x), std::max(abs(y), abs(z)));
        if (maxd == abs(x))
        {
            if (x > 0)
            {
                idx = 0;
                u = -z;
                v = -y;
            }
            else
            {
                idx = 1;
                u = z;
                v = -y;
            }
        }
        else if (maxd == abs(y))
        {
            if (y > 0)
            {
                idx = 2;
                u = x;
                v = z;
            }
            else
            {
                idx = 3;
                u = x;
                v = -z;
            }
        }
        else
        {
            if (z > 0)
            {
                idx = 4;
                u = x;
                v = -y;
            }

            else
            {
                idx = 5;
                u = -x;
                v = -y;
            }
        }
        u = (u / maxd + 1) * 0.5f;
        v = (v / maxd + 1) * 0.5f;

        return idx;
    }

    Vec4i getTexture3Di( Vec3f &pts, int startidx)
    {
        //pts.normalize();
        float u = 0, v = 0;
        Vec3f pts2 = -1 * pts;
        int idx = getFaceidx(pts, u, v);

        return images[startidx + idx].getColor(u, v,BILINEAR);
    }

    Vec4f getTexture3Df(Vec3f &pts, int startidx)
    {
        // pts.normalize();
        float u = 0, v = 0;
        Vec3f pts2 = -1 * pts;

        int idx = getFaceidx(pts, u, v);

        return images[startidx + idx].getColorf(u, v);
    }

private:
    //debug:mipmap可视化
    void generateMidMap2(int level = 0)
    {
        Image base = images[images.size() - 1];
        int width = base.getWidth();
        int height = base.getHeight();
        if (level == 0)
            level = floor(log2(std::min(width, height)));

        for (int i = 0; i < level; i++)
        {
            width /= 2;
            height /= 2;
            Image subimage(width, height, base.getChannel(), base.getImgType());

            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    Vec4i color = Vec4i((uint8_t)(255.f / i), (uint8_t)(255.f / i), (uint8_t)(255.f / i), 255);
                    subimage.setBuffer(x, y, color);
                }
            }

            int id_sub = setTexture(subimage);
            base = subimage;
        }
    }

    void generateMidMap(int level = 0)
    {
        Image &base = images[images.size() - 1];
        int id_base = images.size() - 1;
        int width = base.getWidth();
        int height = base.getHeight();
        if (level == 0)
            level = floor(log2(std::min(width, height)));
        //预先分配内存
        for (int i = 0; i < level; i++)
        {
            width /= 2;
            height /= 2;
            setTexture(Image(width, height, base.getChannel(), base.getImgType()));
        }

        for (int i = 0; i < level; i++)
        {
            Image &base = images[id_base + i];
            Image &sub = images[id_base + i + 1];

            for (int x = 0; x < sub.getWidth(); x++)
            {
                for (int y = 0; y < sub.getHeight(); y++)
                { //box blur
                    Vec4i c1 = base.readBuffer(x * 2, y * 2);
                    Vec4i c2 = base.readBuffer(x * 2 + 1, y * 2);
                    Vec4i c3 = base.readBuffer(x * 2, y * 2 + 1);
                    Vec4i c4 = base.readBuffer(x * 2 + 1, y * 2 + 1);
                    Vec4i color((c1.x + c2.x + c3.x + c4.x) / 4, (c1.y + c2.y + c3.y + c4.y) / 4, (c1.z + c2.z + c3.z + c4.z) / 4, 255);
                    //printColor(color);
                    sub.setBuffer(x, y, color);
                }
            }
        }
    }

public:
    //ibl

    void generateMipMapf(int level)
    {
        Image &base = images[images.size() - 1];
        int id_base = images.size() - 1;
        int width = base.getWidth();
        int height = base.getHeight();

        level = std::max(level, (int)(floor(log2(std::min(width, height)))));

        for (int i = 0; i < level; i++)
        {
            width /= 2;
            height /= 2;
            setTexture(Image(width, height, base.getChannel(), base.getImgType()));
        }

        for (int i = 0; i < level; i++)
        {
            Image &base = images[id_base + i];
            Image &sub = images[id_base + i + 1];

            for (int x = 0; x < sub.getWidth(); x++)
            {
                for (int y = 0; y < sub.getHeight(); y++)
                {
                    Vec4i c1 = base.readBuffer(x * 2, y * 2);
                    Vec4i c2 = base.readBuffer(x * 2 + 1, y * 2);
                    Vec4i c3 = base.readBuffer(x * 2, y * 2 + 1);
                    Vec4i c4 = base.readBuffer(x * 2 + 1, y * 2 + 1);
                    Vec4i color((c1.x + c2.x + c3.x + c4.x) / 4, (c1.y + c2.y + c3.y + c4.y) / 4, (c1.z + c2.z + c3.z + c4.z) / 4, 255);
                    //printColor(color);
                    sub.setBuffer(x, y, color);
                }
            }
        }
    }
};

#endif