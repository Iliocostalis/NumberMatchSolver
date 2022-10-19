#include <vector>
#include <algorithm>
#include <iostream>
#include <NumberGame.h>
#include <cstring>
#include <list>
#include <windows.h> 

HANDLE hConsole;

inline void setOutputColorNormal()
{
    //std::cout << "\033[0m";
    SetConsoleTextAttribute(hConsole, 15);
}

inline void setOutputColorHighlight()
{
    //std::cout << "\033[32m";
    SetConsoleTextAttribute(hConsole, 10);
}

inline void setOutputColorRemoveNext()
{
    //std::cout << "\033[34m";
    SetConsoleTextAttribute(hConsole, 9);
}

NumberGame::NumberGame()
{
    width = GAME_WIDTH;
    height = GAME_START_HEIGHT;
    numbers.resize(GAME_WIDTH * GAME_START_HEIGHT * 2 * 2 * 2 * 2);
    std::fill(numbers.begin(), numbers.end(), FIELD_EMPTY);

    for(auto& n : numbers)
        n = rand() % 10;
}

NumberGame::NumberGame(std::vector<int>& numbersIn)
{
    width = GAME_WIDTH;
    height = (numbersIn.size() + GAME_WIDTH - 1) / GAME_WIDTH;
    numbers.resize(GAME_WIDTH * GAME_START_HEIGHT * 2 * 2 * 2 * 2);
    std::fill(numbers.begin(), numbers.end(), FIELD_EMPTY);
    std::copy(numbersIn.begin(), numbersIn.end(), numbers.data());
    //std::memcpy(numbers.data(), numbersIn.data(), GAME_WIDTH * GAME_START_HEIGHT * sizeof(int));
}

void NumberGame::findPairs()
{
    pairs.clear();
    pairs.reserve(16);

    for(int y = 0; y < height; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            int n = getNumber(x, y);
            if(n < 0)
                continue;
            check(n, x, y, 1, 0, &pairs);
            check(n, x, y, -1, 0, &pairs);
            check(n, x, y, 0, 1, &pairs);
            check(n, x, y, 0, -1, &pairs);
            check(n, x, y, 1, 1, &pairs);
            check(n, x, y, -1, 1, &pairs);
            check(n, x, y, 1, -1, &pairs);
            check(n, x, y, -1, -1, &pairs);
        }
    }
}

void NumberGame::removePair(int index)
{
    Pair& pair = pairs[index];
    numbers[pair.ay * width + pair.ax] = FIELD_GRAY;
    numbers[pair.by * width + pair.bx] = FIELD_GRAY;

    for(int y = 0; y < height; ++y)
    {
        int emptyCount = 0;
        for(int x = 0; x < width; ++x)
        {
            if(getNumber(x,y) < 0)
                ++emptyCount;
        }
        if(emptyCount == width)
        {
            for(int yc = y; yc < height; ++yc)
                for(int xc = 0; xc < width; ++xc)
                    numbers[yc * width + xc] = getNumber(xc, yc+1);

            --height;
            --y; // redo same y
        }
    }
}

void NumberGame::findSolution()
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    std::vector<NumberGame> gameHistory;
    gameHistory.push_back(*this);
    gameHistory.back().findPairs();
    //gameHistory.back().print();

    do
    {
        NumberGame& game = gameHistory.back();

        if(game.pairs.size() == 0)
        {
            if(game.height == 0)
            {
                std::cout << "Solution found!" << std::endl;
                while(gameHistory.size() > 0)
                {
                    gameHistory.back().print();
                    gameHistory.pop_back();
                }
                this->print();
                return;
            }
            if(game.addsLeft > 0)
            {
                gameHistory.push_back(game);
                gameHistory.back().addNewNumbers();
                gameHistory.back().findPairs();
                gameHistory.back().pairIndex = 0;
                continue;
            }
            else
            {
                gameHistory.pop_back();
                if(gameHistory.size() == 0)
                    break;
                continue;
            }
        }

        if(game.pairIndex >= game.pairs.size())
        {
            gameHistory.pop_back();
            continue;
        }

        int removePairIndex = game.pairIndex;
        game.pairIndex += 1;
        //game.print();

        gameHistory.push_back(game);
        gameHistory.back().removePair(removePairIndex);
        gameHistory.back().pairIndex = 0;
        gameHistory.back().findPairs();

    } while(true);







/*
    std::list<NumberGame> gameHistoryShortestWin;
    std::list<NumberGame> gameHistory;
    gameHistory.push_back(*this);
    gameHistory.back().findPairs();
    //gameHistory.back().print();

    do
    {
        NumberGame& game = gameHistory.back();

        if(game.pairs.size() == 0)
        {
            if(game.height == 0)
            {
                //std::cout << "Solution found!" << std::endl;
                if(gameHistoryShortestWin.size() > gameHistory.size())
                {
                    gameHistoryShortestWin.clear();
                    for(const auto& gh : gameHistory)
                        gameHistoryShortestWin.push_back(gh);
                }
                else if(gameHistoryShortestWin.size() == 0)
                {
                    for(const auto& gh : gameHistory)
                        gameHistoryShortestWin.push_back(gh);
                }
                gameHistory.pop_back();
                if(gameHistory.size() == 0)
                    break;
                continue;
            }
            else if(game.addsLeft > 0)
            {
                gameHistory.push_back(game);
                gameHistory.back().addNewNumbers();
                gameHistory.back().findPairs();
                gameHistory.back().pairIndex = 0;
                continue;
            }
            else
            {
                gameHistory.pop_back();
                if(gameHistory.size() == 0)
                    break;
                continue;
            }
        }

        if(game.pairIndex >= game.pairs.size())
        {
            gameHistory.pop_back();
            if(gameHistory.size() == 0)
                break;
            continue;
        }

        int removePairIndex = game.pairIndex;
        game.pairIndex += 1;
        //game.print();

        gameHistory.push_back(game);
        gameHistory.back().removePair(removePairIndex);
        gameHistory.back().pairIndex = 0;
        gameHistory.back().findPairs();

    } while(true);

    // print solution
    while(gameHistoryShortestWin.size() > 0)
    {
        gameHistoryShortestWin.back().print();
        gameHistoryShortestWin.pop_back();
    }
    this->print();*/
}

void NumberGame::addNewNumbers()
{
    int countNumbers = 0;
    int countFields = 0;
    for(int i = 0; i < width*height; ++i)
    {
        if(numbers[i] >= 0)
            ++countNumbers;
        if(numbers[i] != FIELD_EMPTY)
            ++countFields;
    }

    int newCount = countFields+countNumbers;

    int indexCopy = 0;
    for(int i = 0; i < countNumbers; ++i)
    {
        while(numbers[indexCopy] < 0)
            indexCopy += 1;
            
        numbers[countFields+i] = numbers[indexCopy];
        indexCopy += 1;
    }

    height = 1 + (newCount-1) / width;
    addsLeft -= 1;
}

void NumberGame::printNumber(int x, int y)
{
    int n = getNumber(x, y);
    if(n < 0)
        std::cout << " |";
    else
    {
        if(isRemovable(x, y))
        {
            int pIndex = pairIndex-1;
            if(pairs.size() > pIndex && ((pairs[pIndex].ax == x && pairs[pIndex].ay == y) || (pairs[pIndex].bx == x && pairs[pIndex].by == y)))
                setOutputColorRemoveNext();
            else
                setOutputColorHighlight();
            std::cout << n;
            setOutputColorNormal();
            std::cout << "|";
        }
        else
            std::cout << n << "|";
    }
}

void NumberGame::print()
{
    std::cout << "Game:" << std::endl;

    for(int x = 0; x <= width*2; ++x)
        std::cout << "-";
    std::cout << std::endl;

    for(int y = 0; y < height; ++y)
    {
        std::cout << "|";
        for(int x = 0; x < width; ++x)
        {
            printNumber(x, y);
        }
        std::cout << std::endl;

        for(int x = 0; x <= width*2; ++x)
            std::cout << "-";

        std::cout << std::endl;
    }
}