#pragma once

#include <Utils.h>
#if DEFINED(LINUX)
#include <stdint.h>
#include <Image.h>
#include <list>
#include <NeuralNetwork.h>

namespace PictureReader
{
    void readImage(ImageRGB& imageRGB);
    void getNumbers(ImageRGB& imageRGB, std::vector<int>* numbers, const NeuralNetwork& nn);
    void getNumberImagesForNN(ImageRGB& imageRGB, std::list<Image>* out, const std::vector<int>& numbers);

    PlayField findPlayField(Image& image);
    void findNumbers(const Image& image, Image& imageFiltered, const PlayField& playField, std::vector<int>* numbers, const NeuralNetwork& nn);
    void getNumberImages(const Image& image, Image& imageFiltered, const PlayField& playField, std::list<Image>* out, const std::vector<int>& numbers);
};
#endif