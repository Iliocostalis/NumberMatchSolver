#pragma once
#include <Image.h>
#include <Utils.h>

class BlockToColorThreshold
{
    int sampleSizeX;
    int sampleSizeY;

    std::vector<int> colors;

public:
    BlockToColorThreshold(int sampleSizeMin, int sampleSizeMax, Direction direction);

    int getColorThreshold(const Image& image, const Position<int>& pos);
};