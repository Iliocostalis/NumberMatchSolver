#pragma once

#include <stdint.h>
#include <Image.h>
#include <Utils.h>
#include <list>

namespace PictureReader
{
    void readImage(ImageRGB& image);
    PlayField findPlayField(Image& image);
    void findNumbers(const Image& image, const PlayField& playField);
    void getNumberImages(const Image& image, Image& imageFiltered, const PlayField& playField, std::list<Image>* out, int imageSize, const std::vector<int>& numbers);
    void getNumberImagesForNN(ImageRGB& imageRGB, std::list<Image>* out, int imageSize, const std::vector<int>& numbers);
};