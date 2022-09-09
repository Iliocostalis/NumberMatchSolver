#pragma once
#include <vector>
#include <algorithm>
#include <Image.h>

struct Position
{
    int x;
    int y;
};

struct PlayField
{
    Position topLeft;
    Position topRight;
    float squareWidth;
    float squareHeight;
};

enum Direction
{
    LEFT,
    RIGHT,
    UP,
    DOWN
};

inline Direction rotateDirection90(Direction direction)
{
    switch (direction)
    {
        case Direction::UP:
            return Direction::RIGHT;

        case Direction::DOWN:
            return Direction::LEFT;

        case Direction::RIGHT:
            return Direction::DOWN;

        case Direction::LEFT:
            return Direction::UP;
    }
    
    return Direction::UP;
}

extern void saveImage(const std::string& name, const Image& image);

inline uint8_t median5(std::array<uint8_t, 5> colors, int index)
{
    std::sort(colors.begin(), colors.end());
    return colors[index];
}

inline void blurVector(std::vector<int>& values)
{
    int buf[3];
    int indexBuf = 2;
    buf[0] = values[0];
    buf[1] = values[1];
    
    for(int i = 1; i < values.size() - 1; ++i)
    {
        //buf[indexBuf] = std::max(values[i+1], 0);
        buf[indexBuf] = values[i+1];
        indexBuf = (indexBuf + 1) % 3;

        values[i] = (buf[0] + buf[1] + buf[2] + values[i]) / 4;
    }

    values.front() = 0;
    values.back() = 0;
}

inline void blurVector(std::vector<float>& values)
{
    float buf[3];
    int indexBuf = 2;
    buf[0] = values[0];
    buf[1] = values[1];
    
    for(int i = 1; i < values.size() - 1; ++i)
    {
        buf[indexBuf] = values[i+1];
        values[i] = (buf[0] + buf[1] + buf[2] + values[i]) / 4.0f;
        indexBuf = (indexBuf + 1) % 3;
    }

    values.front() = (values[0] + values[1] + 0 + values[0]) / 4.0f;
    values.back() = (values[values.size() - 1] + values[values.size() - 2] + 0 + values[values.size() - 1]) / 4.0f;
}

inline std::vector<float> getFrequenzyOfValues(std::vector<int>& values, int frequenzyWidth, float* pMinVal, float* pMaxVal, float* pScale)
{
    int addExtra = 2;
    float minVal = 1e20;
    float maxVal = -1e20;

    for(auto& v : values)
    {
        float val = (float)v;
        minVal = std::min(minVal, val);
        maxVal = std::max(maxVal, val);
    }

    float scale = (float)(frequenzyWidth-addExtra) / (maxVal - minVal);

    std::vector<float> averages;
    averages.resize(frequenzyWidth+addExtra+addExtra);
    std::fill(averages.begin(), averages.end(), 0);

    for(auto& v : values)
    {
        int index = (int)(((float)v - minVal) * scale + (float)addExtra + 0.5f);
        averages[index] += 1;
    }

    *pMinVal = minVal;
    *pMaxVal = maxVal;
    *pScale = scale;
    return averages;
}

inline std::vector<int> getHighPoints(std::vector<int>& values)
{
    std::vector<int> highPoints;
    for(int i = 1; i < values.size()-1; ++i)
    {
        if(values[i] > values[i-1] && values[i] >= values[i+1])
        {
            highPoints.push_back(i);
        }
    }

    return highPoints;
}

inline int frequenzyIndexToValue(float scale, int minVal, int index)
{
    return (int)(((float)index - 2.0f) / scale + 0.5f) + minVal;
}

inline float frequenzyIndexToValueF(float scale, int minVal, int index)
{
    return (((float)index - 2.0f) / scale) + minVal;
}

inline float findMostFreqentValue(std::vector<int>& values)
{
    float minVal;
    float maxVal;
    float scale;
    auto out = getFrequenzyOfValues(values, 32, &minVal, &maxVal, &scale);
    //auto out = blurFreqenzyOfValues(values, 8, &minVal, &maxVal, &scale);
    blurVector(out);
    blurVector(out);
    blurVector(out);

    int index = 0;
    int highestIndex = 0;
    float highest = 0;
    for(auto& v : out)
    {
        if(v > highest)
        {
            highest = v;
            highestIndex = index;
        }
        ++index;
    }    

    float val1 = frequenzyIndexToValueF(scale, minVal, highestIndex-1);
    float val2 = frequenzyIndexToValueF(scale, minVal, highestIndex);
    float val3 = frequenzyIndexToValueF(scale, minVal, highestIndex+1);

    float valF = (val1 * out[highestIndex-1] + val2 * out[highestIndex] + val3 * out[highestIndex+1]) / (out[highestIndex-1] + out[highestIndex] + out[highestIndex+1]);

    //float val = frequenzyIndexToValueF(scale, minVal, highestIndex);
    return valF;
}

inline int getPixelOnLineScoreSmall(Image& image, Position pos, Direction direction)
{
    int maskLR[] = {  3,  1,  0,  1,  3,
                     -1,  0,  0,  0, -1,
                     -3, -2, -2, -2, -3,
                     -1,  0,  0,  0, -1,
                      3,  1,  0,  1,  3};

    int maskUD[] = { 3, -1,  -3, -1,  3,
                     1,  0,  -2,  0,  1,
                     0,  0,  -2,  0,  0,
                     1,  0,  -2,  0,  1,
                     3, -1,  -3, -1,  3};

    int maskWidth;
    int maskHeight;
    int* mask;
    if(direction == Direction::UP || direction == Direction::DOWN)
    {
        mask = maskUD;
        maskWidth = 5;
        maskHeight = 5;
    }
    else
    {
        mask = maskLR;
        maskWidth = 5;
        maskHeight = 5;
    }

    int maskCenterX = maskWidth/2;
    int maskCenterY = maskHeight/2;

    int val = 0;
    for(int oy = 0; oy < maskHeight; ++oy)
    {
        for(int ox = 0; ox < maskWidth; ++ox)
        {
            val += image.getValue(pos.x + ox - maskCenterX, pos.y + oy - maskCenterY) * mask[oy*maskWidth+ox];
        }
    }

    return val;
}

inline int getPixelOnLineScore(Image& image, Position pos, Direction direction)
{
    int maskUD[] = {  3,  4,  2,  0,  2,  4,  3,
                     -3,  0,  0,  0,  0,  0, -3,
                     -7, -3, -2,  0, -2, -3, -7,
                     -3,  0,  0,  0,  0,  0, -3,
                      3,  4,  2,  0,  2,  4,  3};

    int maskLR[] = { 3, -3, -7, -3,  3,
                     4,  0, -3,  0,  4,
                     2,  0, -2,  0,  2,
                     0,  0,  0,  0,  0,
                     2,  0, -2,  0,  2,
                     4,  0, -3,  0,  4,
                     3, -3, -7, -3,  3};

    int maskWidth;
    int maskHeight;
    int* mask;
    if(direction == Direction::UP || direction == Direction::DOWN)
    {
        mask = maskUD;
        maskWidth = 7;
        maskHeight = 5;
    }
    else
    {
        mask = maskLR;
        maskWidth = 5;
        maskHeight = 7;
    }

    //int mv = 0;
    //for(int i = 0; i < maskWidth*maskHeight; ++i)
    //    mv += mask[i];
    int maskCenterX = maskWidth/2;
    int maskCenterY = maskHeight/2;

    int val = 0;
    for(int oy = 0; oy < maskHeight; ++oy)
    {
        for(int ox = 0; ox < maskWidth; ++ox)
        {
            val += image.getValue(pos.x + ox - maskCenterX, pos.y + oy - maskCenterY) * mask[oy*maskWidth+ox];
        }
    }

    return val;
}

inline void showPoint(Image& image, Position point)
{
    for(int y = -2; y <= 2; ++y)
        for(int x = -2; x <= 2; ++x)
        {
            if((x+y) & 0x1)
                image.setValue(point.x+x, point.y+y, 255);
            else
                image.setValue(point.x+x, point.y+y, 0);
        }
}