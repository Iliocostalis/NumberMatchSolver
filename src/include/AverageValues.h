#pragma once
#include <vector>

class AverageValues
{
    int count;
    int index;
    std::vector<int> values;
    bool hasToInit;
    int currentCount;

public:
    AverageValues(int count);

    void addValue(int value);

    void reset();

    int getValue();
};