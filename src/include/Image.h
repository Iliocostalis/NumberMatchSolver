#pragma once

#include <stdint.h>
#include <vector>

class Image
{
protected:
    bool createdBuffer;
    Image();

public:
    int width;
    int height;
    uint8_t* pixels;

    Image(int width, int height, uint8_t* pixels);
    Image(int width, int height);
    ~Image();

    inline uint8_t getValue(int x, int y) const
    {
        return pixels[y * width + x];
    }

    inline void setValue(int x, int y, uint8_t value)
    {
        pixels[y * width + x] = value;
    }
};

class ImageRGB : public Image
{
public:
    ImageRGB(int width, int height, uint8_t* pixels);
    ImageRGB(int width, int height);

    inline uint8_t getValue(int x, int y, int rgb) const
    {
        return pixels[(y * width + x)*3 + rgb];
    }

    inline void setValue(int x, int y, int rgb, uint8_t value)
    {
        pixels[(y * width + x)*3 + rgb] = value;
    }
};