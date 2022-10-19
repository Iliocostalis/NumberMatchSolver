#pragma once

#define GAME_WIDTH 9
#define GAME_HEIGHT_MAX 48
#define GAME_START_HEIGHT 3
#define GAME_HEIGHT_VISIBLE 11
#define FIELD_EMPTY 200
#define FIELD_GRAY 100



namespace NumberGameBrute
{
    extern void solve(std::vector<int>& numbersIn);
    extern void solveNew(std::vector<int>& numbersIn);
}