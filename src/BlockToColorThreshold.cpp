#include <BlockToColorThreshold.h>
#include <algorithm>
#include <iostream>

BlockToColorThreshold::BlockToColorThreshold(int sampleSizeMin, int sampleSizeMax, Direction direction)
{
    switch (direction)
    {
        case Direction::UP:
            sampleSizeX = sampleSizeMax;
            sampleSizeY = sampleSizeMin;
            break;
        case Direction::DOWN:
            sampleSizeX = sampleSizeMax;
            sampleSizeY = sampleSizeMin;
            break;
        case Direction::LEFT:
            sampleSizeY = sampleSizeMax;
            sampleSizeX = sampleSizeMin;
            break;
        case Direction::RIGHT:
            sampleSizeY = sampleSizeMax;
            sampleSizeX = sampleSizeMin;
            break;
    }

    colors.resize((sampleSizeMin*2+1)*(sampleSizeMax*2+1));
}

void printVector(std::vector<float>& vector)
{
    std::cout << "Vector:" << std::endl;
    int resolution = 10;

    bool running;
    int highestVal = 0;
    
    for(auto& v : vector)
        highestVal = std::max(highestVal, (int)v);

    float scale = (float)resolution / highestVal;

    for(int i = 0; i < resolution; ++i)
    {
        for(auto& v : vector)
        {
            if(v*scale > i)
                std::cout << "#";
            else
                std::cout << " ";
        }
        std::cout << std::endl;
    }
}

int BlockToColorThreshold::getColorThreshold(const Image& image, const Position& pos)
{
    int index = 0;
    for (int x = -sampleSizeX; x <= sampleSizeX; x++)
    {
        for (int y = -sampleSizeY; y <= sampleSizeY; y++)
        {
            colors[index++] = image.getValue(pos.x + x, pos.y + y);
        }
    }

    auto& values = colors;

    //std::vector<int>values = {2,4,3,6,4,0};
    std::sort(values.begin(), values.end());

    int minVal = values.front();
    int maxVal = values.back();

    //int median = values[values.size()/2];
    //int medianSpace = std::min(median, 255-median);
    /*int valueLast = 0;
    int valueLastMin = -1000;
    int valueLastMax = 1000;
    for(int i = 0; i < values.size(); ++i)
    {
        int indexMin = i;
        int indexMax = values.size() - 1 - i;

        int spaceMin = values[indexMin] - valueLastMin;
        if(spaceMin > 20)
            minVal = values[indexMin];
        
        valueLastMin = values[indexMin];

        int spaceMax = valueLastMax - values[indexMax];
        if(spaceMax > 20)
            maxVal = values[indexMax];

        valueLastMax = values[indexMax];
    }*/


    int div = 8;

    std::vector<float> averages;
    averages.resize((maxVal - minVal)/div+1);
    std::fill(averages.begin(), averages.end(), 0);

    if(averages.size() < 3)
        return (maxVal + minVal) / 2;

    for(auto& v : values)
    {
        int index = (v-minVal)/div;
        //if(index >= 0 && index < averages.size())
        averages[index] += 1;
    }

    std::vector<float> averagesTmp;
    averagesTmp.resize(averages.size());

    auto& b = averages;
    auto& a = averagesTmp;

    //printVector(b);
    //std::cout << "lowest: " << minVal << "\n"<< std::endl;
    
    for(int i = 0; i < 4; ++i)
    {
        auto& t = a;
        a = b;
        b = t;
        b.front() = (0 + a[0] + a[1]) / 3.0f;
        b.back() = (a[a.size()-2] + a[a.size()-1] + 0) / 3.0f;
        for(int v = 1; v < averages.size()-1; ++v)
        {
            b[v] = (a[v-1] + a[v] + a[v+1]) / 3.0f;
        }
        //printVector(b);
    }
    //printVector(b);

    int indexLast = 0;
    std::vector<int> highPoints;
    for(int v = 1; v < b.size()-1; ++v)
    {
        if(b[v] > b[v-1] && b[v] >= b[v+1])
        {
            highPoints.push_back(v*div+minVal);
            indexLast = v;
        }
    }

    if(highPoints.size() <= 1)
        return 0;
        
    // median
    //return b.size() / 2 + 1 + minVal;

    // average
    //return (highPoints.front() + highPoints.back()) / 2;

    // highest point still falling
    float valueLast = b[indexLast];
    for(int i = indexLast; i > 0; --i)
    {
        if(b[i-1] > valueLast)
        {
            return i*div+minVal;
        }
        valueLast = b[i];
    }

    return 0;
}