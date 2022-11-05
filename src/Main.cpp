#include <iostream>
#include <vector>
#include <chrono>
#include <NumberGame.h>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <PictureReader.h>
#include <Image.h>
#include <Debugging.h>
#include <filesystem>
#include <NumberGameBrute.h>

#if DEFINED(LINUX)
#include <NeuralNetwork.h>
#endif

#define FIXED_INPUT
//#define TRAIN
//#define IMAGE_INPUT
//#define TEST_ALL
//#define USER_INPUT
//#define FIXED_INPUT

void exit()
{
    int forceBreakCount = 0;
    do 
    {
        forceBreakCount++;
        if(forceBreakCount > 5)
            return;
        std::cout << '\n' << "Press a key to continue...";
    } while (std::cin.get() != '\n');
}

int main()
{
#ifdef TRAIN

    NeuralNetwork nn;

    std::string path = "../images/";
    for (const auto & entry : std::filesystem::directory_iterator(path))
    {
        if(entry.is_directory())
            continue;

        std::cout << entry.path() << std::endl;
        int width = 0;
        int height = 0;
        int comp = 0;

        std::string imagePath = entry.path();
        //std::string path = "../images/429732173956549456721787238#1.jpg";
        stbi_uc* data = stbi_load(imagePath.c_str(), &width, &height, &comp, STBI_rgb);
        std::vector<int> numbers;

        std::string startSymbol = "/";
        std::string endSymbol = "#";
        int nameStart = imagePath.find_last_of(startSymbol) + 1;
        int nameEnd = imagePath.find_last_of(endSymbol);
        std::string name = imagePath.substr(nameStart, nameEnd-nameStart);

        numbers.reserve(name.size()); // get 4 empty squares for training

        int countNumber = 0;
        for(int i = 0; i < name.size(); ++i)
        {
            int c = (int)name[i];
            if(c == 'x')
            {
                numbers[i] = 10;
            }
            else
            {
                ASSERT(c <= 57 && c >= 48);
                int value = c - 48;
                numbers.push_back(value);
                countNumber += 1;
            }
        }

        ImageRGB image(width, height, data);
        std::list<Image> numberImages;

        PictureReader::getNumberImagesForNN(image, &numberImages, numbers);

        ASSERT(numberImages.size() == countNumber);

        int index = 0;
        for(const auto& image : numberImages)
        {
            nn.addTrainingData(image, numbers[index]);
            index += 1;
        }

        stbi_image_free(data);
    }

    nn.train();
    nn.save("../neural_network/network");

#elif defined IMAGE_INPUT

    int width = 0;
    int height = 0;
    int comp = 0;

    stbi_uc* data = stbi_load("../images/test/test3.jpg", &width, &height, &comp, STBI_rgb);

    ImageRGB image(width, height, data);

    PictureReader::readImage(image);

    stbi_image_free(data);

#elif defined TEST_ALL

    NeuralNetwork nn("../neural_network/network");
    std::string imagePath = "../images/test/test.jpg";

    int width = 0;
    int height = 0;
    int comp = 0;
    stbi_uc* data = stbi_load(imagePath.c_str(), &width, &height, &comp, STBI_rgb);
    ImageRGB image(width, height, data);

    std::vector<int> numbers;
    PictureReader::getNumbers(image, &numbers, nn);

    stbi_image_free(data);


    NumberGame numberGame(numbers);
    numberGame.findSolution();

#elif defined USER_INPUT

    std::cout << "Input game numbers line by line!" << std::endl;
    std::vector<int> inputs;
    inputs.reserve(3*9);
    int i = 0;
    while(i < 3)
    {
        std::string str;
        std::getline(std::cin, str);

        if(str.size() != 9)
        {
            std::cout << "Wrong input count" << std::endl;
            exit();
            return 0;
        }

        for(int x = 0; x < GAME_WIDTH; ++x)
        {
            char charRead = str[x];
            if(charRead >= 48 && charRead <= 57)
                inputs.push_back(charRead - 48);
            else
            {
                std::cout << "Only numbers allowed" << std::endl;
                exit();
                return 0;
            }
        }

        ++i;
    }
    
    //NumberGameBrute::solve(inputs);
    NumberGame numberGame(inputs);
    numberGame.findSolution();

    exit();

#elif defined FIXED_INPUT
    std::vector<int> inputs;
    inputs.reserve(3*9);
    //inputNumbers inputs = {1,7,6,1,9,4,1,7,4,
    //                        8,5,2,3,2,3,5,2,3,
    //                        4,9,6,1,6,1,4,1,8};
    //inputNumbers inputs = {4,2,1,6,7,8,7,8,9,
    //                        7,5,7,5,9,5,4,5,1,
    //                        8,1,2,6,3,2,8,5,2};
    //inputs = {5,5,7,9,6,1,2,9,5,
    //        7,9,2,5,8,3,5,7,8,
    //        3,8,7,1,4,9,2,9,6};
    //inputNumbers inputs = {5,5,7,9,6,1,2,9,0,
    //                        0,9,2,5,8,3,5,7,8,
    //                        3,8,7,1,4,9,2,9,6};
    //inputs = {7,5,1,5,3,1,7,8,5,
    //3,6,2,6,8,4,5,4,9,
    //1,5,1,5,9,7,2,6,5};
    inputs = {8,9,4,2,1,5,2,3,4,
    3,5,8,3,6,7,4,9,2,
    6,1,4,9,2,1,8,3,5};
    //NumberGame numberGame(inputs);
    //numberGame.findSolution();
    //NumberGameBrute::solve(inputs);
    NumberGameBrute::sovleFindMax4Times(inputs);
    exit();
#else

    srand(2);
    NumberGame numberGame;
    numberGame.findSolution();

#endif
    return 0;
}