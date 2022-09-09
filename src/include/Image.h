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

    uint8_t getValue(int x, int y) const;
    void setValue(int x, int y, uint8_t value);

    void scale(Image* out);
    void lineFilter(Image* out);
};

////////////////////////////////////////ImageRGB/////////////////////////////////////////

class ImageRGB : public Image
{
public:
    ImageRGB(int width, int height, uint8_t* pixels);
    ImageRGB(int width, int height);
    ~ImageRGB();

    uint8_t getValue(int x, int y, int rgb) const;
    void setValue(int x, int y, int rgb, uint8_t value);

    void toBlackWhite(Image* imageOut);
};