#pragma once

#include <stdint.h>
#include <Image.h>
#include <Utils.h>

namespace PictureReader
{
    void readImage(ImageRGB& image);
    PlayField findPlayField(Image& image);
    void findNumbers(const Image& image, const PlayField& playField);
};