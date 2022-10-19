#pragma once

#include <vector>
#include <array>

#define GAME_WIDTH 9
#define GAME_START_HEIGHT 3
#define GAME_HEIGHT_VISIBLE 11
#define FIELD_EMPTY -2
#define FIELD_GRAY -1

typedef std::array<int, GAME_WIDTH*GAME_START_HEIGHT> inputNumbers;

struct Pair
{
    int ax;
    int ay;
    int bx;
    int by;
};

class NumberGame
{
    int width;
    int height;
    int addsLeft = 4;
    
    std::vector<int> numbers;
    std::vector<Pair> pairs;
    int pairIndex = 0;

private:
    inline void addPairToList(std::vector<Pair>* pairs, Pair& pair)
    {
        for(auto& p : *pairs)
        {
            if(p.bx == pair.ax && p.by == pair.ay) // already inside
                return;
        }
        pairs->push_back(pair);
    }

    inline bool areNumbersOk(int a, int b)
    {
        return a == b || a + b == 10;
    }

    inline int getNumber(int x, int y)
    {
        return numbers[y * width + x];
    }

    inline bool check(int currentNumber, int x, int y, int moveX, int moveY, std::vector<Pair>* pairs)
    {
        Pair pair;
        pair.ax = x;
        pair.ay = y;

        // wrap around on right border
        int wrapOffsetR = 0;
        int wrapOffsetL = 0;
        if(moveX == 1 && moveY == 0)
            wrapOffsetR = 1;
        if(moveX == -1 && moveY == 0)
            wrapOffsetL = -1;

        while(y >= 0-moveY && y <= height-1-moveY && x >= 0-moveX+wrapOffsetL && x <= width-1-moveX+wrapOffsetR)
        {
            if(moveX == 1 && moveY == 0 && x == width-1) // wrap around on right border
            {
                ++y;
                x = -1;
            }
            if(moveX == -1 && moveY == 0 && x == 0)
            {
                if(y == 0)
                {
                    break;
                }
                --y;
                x = width;
            }

            x += moveX;
            y += moveY;

            int number = getNumber(x, y);
            if(number >= 0)
            {
                if(areNumbersOk(number, currentNumber))
                {
                    pair.bx = x;
                    pair.by = y;
                    addPairToList(pairs, pair);
                    return true;
                }
                else
                    return false;
            }
        }

        return false;
    }

    inline bool isRemovable(int x, int y)
    {
        int n = getNumber(x, y);

        std::vector<Pair> pairs;

        bool b = check(n, x, y, 1, 0, &pairs);
        b |= check(n, x, y, -1, 0, &pairs);
        b |= check(n, x, y, 0, 1, &pairs);
        b |= check(n, x, y, 0, -1, &pairs);
        b |= check(n, x, y, 1, 1, &pairs);
        b |= check(n, x, y, 1, -1, &pairs);
        b |= check(n, x, y, -1, 1, &pairs);
        b |= check(n, x, y, -1, -1, &pairs);
        return b;
    }

    void printNumber(int x, int y);

public:
    NumberGame();
    NumberGame(std::vector<int>& numbersIn);
    void findPairs();
    void removePair(int index);
    void findSolution();
    void addNewNumbers();
    void print();
};