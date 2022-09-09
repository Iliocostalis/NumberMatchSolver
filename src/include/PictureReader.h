#pragma once

#include <stdint.h>
#include <Image.h>
#include <Utils.h>

namespace PictureReader
{
    void readImage(ImageRGB& image);
    void imageToBlackWhite(ImageRGB& imageRGB, Image& image);
    void lineFilter(Image& in, Image& out);
    void scaleImage(Image& imageIn, Image& imageOut);
    //void findNumberPositions(Image& image);
    PlayField findPlayField(Image& image);
};