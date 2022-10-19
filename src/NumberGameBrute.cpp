#include <vector>
#include <algorithm>
#include <iostream>
#include <NumberGameBrute.h>
#include <cstring>
#include <list>
#include <unordered_set>
#include <city.h>

namespace NumberGameBrute
{
    struct Game
    {
        uint64_t indexParent;
        uint32_t fieldCount;
        uint32_t numberCount;
        uint32_t addCount;
        uint8_t numbers[GAME_HEIGHT_MAX*GAME_WIDTH];
    };

    struct Hash 
    {
        size_t operator()(const Game &game) const 
        {
            size_t hash = 0;

            //for(int i = 0; i < 32; ++i)
            //    hash = hash | ((game.numbers[i] & 0x1) << i);

            //hash = game.indexParent;

            //for(int i = 0; i < game.numberCount/sizeof(uint32_t); ++i)
            //{
            //    hash = hash ^ ((uint32_t*)game.numbers)[i];
            //}
            //hash = (hash >> 16) ^ (hash << 16);

            //for(int i = 0; i < game.numberCount/sizeof(uint32_t); ++i)
            //{
            //    hash += (uint32_t)((uint32_t*)game.numbers)[i];
            //}
            //hash += game.numberCount;
            //hash = hash & 0xfffff;

            //for(int i = 0; i < game.numberCount/sizeof(size_t); ++i)
            //{
            //    hash = hash ^ ((size_t*)game.numbers)[i];
            //}

            hash = CityHash32((char*)game.numbers, game.numberCount);
            //std::cout << hash << std::endl;
            return hash;
        }
    };

    uint64_t comp = 0;
    bool operator==(const Game& lhr, const Game& rhs)
    {
        comp++;
        //std::cout << "comp" << std::endl;
        if(lhr.numberCount != rhs.numberCount)
            return false;

        int res = std::memcmp(lhr.numbers, rhs.numbers, lhr.fieldCount);
        return !res;
    }

    std::unordered_set<Game, Hash> set(1000000);
    Game* games;
    uint32_t maxGamesCount = 10000000; // ca 450 bytes per game -> 4.5 GB

    uint32_t pairIndex;
    uint32_t pairs[GAME_HEIGHT_MAX*GAME_WIDTH / 2];

    inline bool areNumbersOk(uint8_t a, uint8_t b)
    {
        return a == b || a + b == 10;
    }

    inline void checkRight(const Game& game, uint32_t indexStart)
    {
        uint8_t numberStart = game.numbers[indexStart];

        uint32_t index = indexStart;
        while(true)
        {
            index += 1;

            if(index >= game.fieldCount)
                return;

            uint8_t number = game.numbers[index];

            if(number == FIELD_GRAY)
                continue;

            if(areNumbersOk(numberStart, number))
            {
                pairs[pairIndex] = indexStart;
                pairs[pairIndex+1] = index;
                pairIndex += 2;
            }
            return;
        }
    }

    inline void checkDown(const Game& game, uint32_t indexStart)
    {
        uint8_t numberStart = game.numbers[indexStart];

        uint32_t index = indexStart;
        while(true)
        {
            index += GAME_WIDTH;

            if(index >= game.fieldCount)
                return;

            uint8_t number = game.numbers[index];

            if(number == FIELD_GRAY)
                continue;

            if(areNumbersOk(numberStart, number))
            {
                pairs[pairIndex] = indexStart;
                pairs[pairIndex+1] = index;
                pairIndex += 2;
            }
            return;
        }
    }

    inline void checkRightDown(const Game& game, uint32_t indexStart)
    {
        uint8_t numberStart = game.numbers[indexStart];

        uint32_t index = indexStart;
        uint32_t indexX = indexStart % GAME_WIDTH;
        while(true)
        {
            indexX += 1;
            index += GAME_WIDTH + 1;

            if(index >= game.fieldCount || indexX == GAME_WIDTH)
                return;

            uint8_t number = game.numbers[index];

            if(number == FIELD_GRAY)
                continue;

            if(areNumbersOk(numberStart, number))
            {
                pairs[pairIndex] = indexStart;
                pairs[pairIndex+1] = index;
                pairIndex += 2;
            }
            return;
        }
    }

    inline void checkLeftDown(const Game& game, uint32_t indexStart)
    {
        uint8_t numberStart = game.numbers[indexStart];

        uint32_t index = indexStart;
        uint32_t indexX = indexStart % GAME_WIDTH;
        while(true)
        {
            indexX -= 1;
            index += GAME_WIDTH - 1;

            if(index >= game.fieldCount || indexX == -1)
                return;

            uint8_t number = game.numbers[index];

            if(number == FIELD_GRAY)
                continue;

            if(areNumbersOk(numberStart, number))
            {
                pairs[pairIndex] = indexStart;
                pairs[pairIndex+1] = index;
                pairIndex += 2;
            }
            return;
        }
    }

    void findPairs(const Game& game)
    {
        pairIndex = 0;

        for(int i = 0; i < game.fieldCount; ++i)
        {
            if(game.numbers[i] == FIELD_GRAY)
                continue;

            checkRight(game, i);
            checkDown(game, i);
            checkRightDown(game, i);
            checkLeftDown(game, i);
        }
    }

    void findPairs(uint32_t index)
    {
        Game& game = games[index];
        pairIndex = 0;

        for(int i = 0; i < game.fieldCount; ++i)
        {
            if(game.numbers[i] == FIELD_GRAY)
                continue;

            checkRight(game, i);
            checkDown(game, i);
            checkRightDown(game, i);
            checkLeftDown(game, i);
        }
    }

    void add(uint32_t indexParent, uint32_t index)
    {
        int countField = games[indexParent].fieldCount;
        int indexField = 0;
        for(int i = 0; i < countField; ++i)
        {
            uint8_t number = games[indexParent].numbers[i];
            if(number != FIELD_GRAY)
            {
                games[index].numbers[indexField+countField] = number;
                indexField += 1;
            }
        }
    }

    void solveNew(std::vector<int>& numbersIn)
    {
        if(numbersIn.size() > GAME_HEIGHT_MAX*GAME_WIDTH)
            return;

        //auto bc = set.bucket_count();
        games = new Game[maxGamesCount];
        Game& startGame = games[0];

        for(uint32_t i = 0; i < numbersIn.size(); ++i)
            startGame.numbers[i] = (uint8_t)numbersIn[i];

        startGame.fieldCount = numbersIn.size();
        startGame.numberCount = numbersIn.size();
        startGame.indexParent = 0;
        startGame.addCount = 4;

        const auto pair = set.insert(startGame);

        int minNumCount = 30;
        uint32_t indexGames = 0;
        uint32_t indexGamesAdded = 1;
        int skipped = 0;
        while(true)
        {
            Game& gameCurrent = games[indexGames];
            findPairs(gameCurrent);

            int newCount = pairIndex/2;
            for(int i = 0; i < newCount; ++i)
            {
                Game& gameNew = games[indexGamesAdded];
                gameNew.indexParent = indexGames;
                gameNew.addCount = gameCurrent.addCount;
                gameNew.fieldCount = gameCurrent.fieldCount;

                for(int l = 0; l < gameCurrent.fieldCount; ++l)
                    gameNew.numbers[l] = gameCurrent.numbers[l];

                gameNew.numbers[pairs[i*2]] = FIELD_GRAY;
                gameNew.numbers[pairs[i*2+1]] = FIELD_GRAY;
                gameNew.numberCount = gameCurrent.numberCount - 2;

                if(gameNew.numberCount < minNumCount)
                {
                    minNumCount = gameNew.numberCount;
                    std::cout << "min: " << minNumCount << std::endl;
                }

                if(gameNew.numberCount == 0)
                    return;

                if(indexGamesAdded % 10000 == 0)
                    std::cout << indexGamesAdded << "  " << comp << std::endl;

                const auto pair = set.insert(gameNew);
                if(pair.second)
                {
                    /*for(int l = indexGamesAdded-1; l >= 0; --l)
                    {
                        if(games[indexGamesAdded].numberCount != games[l].numberCount)
                            continue;

                        int res = std::memcmp(games[indexGamesAdded].numbers, games[l].numbers, games[l].fieldCount);
                        if(!res)
                        {
                            break;
                        }
                    }*/
                    indexGamesAdded += 1;
                }
                else
                {
                    skipped += 1;
                }
            }

            // use add
            if(gameCurrent.addCount > 0 && newCount == 0)
            {
                Game& gameNew = games[indexGamesAdded];
                gameNew.indexParent = indexGames;
                gameNew.addCount = gameCurrent.addCount - 1;
                gameNew.fieldCount = gameCurrent.fieldCount + gameCurrent.numberCount;
                gameNew.numberCount = gameCurrent.numberCount * 2;

                for(int i = 0; i < gameCurrent.fieldCount; ++i)
                    gameNew.numbers[i] = gameCurrent.numbers[i];

                add(indexGames, indexGamesAdded);

                const auto pair = set.insert(gameNew);

                indexGamesAdded += 1;
            }

            indexGames += 1;
        }
    }

    void solve(std::vector<int>& numbersIn)
    {
        if(numbersIn.size() > GAME_HEIGHT_MAX*GAME_WIDTH)
            return;

        games = new Game[maxGamesCount];
        for(uint32_t i = 0; i < maxGamesCount; ++i)
        {
            std::fill(&games[i].numbers[0], &games[i].numbers[GAME_HEIGHT_MAX*GAME_WIDTH], FIELD_EMPTY);
        }

        for(uint32_t i = 0; i < numbersIn.size(); ++i)
        {
            games[0].numbers[i] = (uint8_t)numbersIn[i];
        }
        games[0].fieldCount = numbersIn.size();
        games[0].numberCount = numbersIn.size();
        games[0].indexParent = 0;
        games[0].addCount = 4;

        Game* gameMin = &games[0];
        uint32_t indexGames = 0;
        uint32_t indexGamesAdded = 1;
        while(true)
        {
            findPairs(indexGames);

            int newCount = pairIndex/2;
            for(int i = 0; i < newCount; ++i)
            {
                games[indexGamesAdded].indexParent = indexGames;
                games[indexGamesAdded].addCount = games[indexGames].addCount;
                games[indexGamesAdded].fieldCount = games[indexGames].fieldCount;

                for(int l = 0; l < games[indexGames].fieldCount; ++l)
                    games[indexGamesAdded].numbers[l] = games[indexGames].numbers[l];

                games[indexGamesAdded].numbers[pairs[i*2]] = FIELD_GRAY;
                games[indexGamesAdded].numbers[pairs[i*2+1]] = FIELD_GRAY;
                games[indexGamesAdded].numberCount = games[indexGames].numberCount - 2;

                if(gameMin->numberCount > games[indexGamesAdded].numberCount)
                {
                    gameMin = &games[indexGamesAdded];
                }

                if(games[indexGamesAdded].numberCount == 0)
                    return;

                if(indexGamesAdded % 100 == 0)
                    std::cout << indexGamesAdded << " " << gameMin->numberCount << std::endl;

                int addedCount = 1;
                for(int l = indexGamesAdded-1; l >= 0; --l)
                {
                    if(games[indexGamesAdded].numberCount != games[l].numberCount)
                        continue;

                    int res = std::memcmp(games[indexGamesAdded].numbers, games[l].numbers, games[l].fieldCount);
                    if(!res)
                    {
                        addedCount = 0;
                        break;
                    }
                }
                indexGamesAdded += addedCount;
            }

            //indexGamesAdded += newCount;

            // use add
            if(games[indexGames].addCount > 0 && newCount == 0)
            {
                games[indexGamesAdded].indexParent = indexGames;
                games[indexGamesAdded].addCount = games[indexGames].addCount - 1;
                games[indexGamesAdded].fieldCount = games[indexGames].fieldCount + games[indexGames].numberCount;
                games[indexGamesAdded].numberCount = games[indexGames].numberCount * 2;

                for(int l = 0; l < games[indexGames].fieldCount; ++l)
                    games[indexGamesAdded].numbers[l] = games[indexGames].numbers[l];

                add(indexGames, indexGamesAdded);

                indexGamesAdded += 1;
            }

            indexGames += 1;
        }
    }
}