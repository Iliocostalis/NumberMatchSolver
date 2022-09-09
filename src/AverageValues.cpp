#include <AverageValues.h>

AverageValues::AverageValues(int count)
{
    this->count = count;
    values.resize(count);
    index = 0;
    currentCount = 0;
}

void AverageValues::addValue(int value)
{
    values[index] = value;
    index = (index + 1) % count;
    currentCount = std::min(currentCount+1, count);
}

void AverageValues::reset()
{
    currentCount = 0;
}

int AverageValues::getValue()
{
    int value = 0;
    for(int i = 0; i < currentCount; ++i)
        value += values[i];
    return value / currentCount;
}