#include <Image.h>

Image::Image(int width, int height, uint8_t* pixels) : width(width), height(height), pixels(pixels)
{
    createdBuffer = false;
}

Image::Image(int width, int height) : width(width), height(height)
{
    pixels = new uint8_t[width*height];
    createdBuffer = true;
}

Image::Image()
{
    
}

Image::~Image()
{
    if(createdBuffer)
        delete[] pixels;
}

ImageRGB::ImageRGB(int width, int height, uint8_t* pixels) : Image(width, height, pixels)
{
}

ImageRGB::ImageRGB(int width, int height)
{
    this->width = width;
    this->height = height;
    pixels = new uint8_t[width*height*3];
    createdBuffer = true;
}