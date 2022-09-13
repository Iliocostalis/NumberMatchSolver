#include <iostream>
#include <floatfann.h>
#include <fann.h>
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
#include <NeuralNetwork.h>
#include <Debugging.h>

#define TRAIN
//#define IMAGE_INPUT
//#define USER_INPUT
//#define FIXED_INPUT

int main()
{
#ifdef TRAIN

NeuralNetwork nn;

int width = 0;
int height = 0;
int comp = 0;

std::string path = "../images/429732173956549456721787238#1.jpg";
stbi_uc* data = stbi_load(path.c_str(), &width, &height, &comp, STBI_rgb);
std::vector<int> numbers;

std::string startSymbol = "/";
std::string endSymbol = "#";
int nameStart = path.find_last_of(startSymbol) + 1;
int nameEnd = path.find_last_of(endSymbol);
std::string name = path.substr(nameStart, nameEnd-nameStart);

numbers.resize(name.size());

for(int i = 0; i < name.size(); ++i)
{
    int c = (int)name[i];
    ASSERT(c <= 57 && c >= 48);
    numbers[i] = c - 48;
}


ImageRGB image(width, height, data);
std::list<Image> numberImages;

PictureReader::getNumberImagesForNN(image, &numberImages, NeuralNetwork::IMAGE_SIZE, numbers);

ASSERT(numberImages.size() == numbers.size());

int index = 0;
for(const auto& image : numberImages)
{
    nn.addTrainingData(image, numbers[index]);
    index += 1;
}

nn.train();

stbi_image_free(data);

#elif IMAGE_INPUT

int width = 0;
int height = 0;
int comp = 0;

stbi_uc* data = stbi_load("../images/429732173956549456721787238#1.jpg", &width, &height, &comp, STBI_rgb);

ImageRGB image(width, height, data);

PictureReader::readImage(image);

stbi_image_free(data);

#elif defined USER_INPUT

    std::cout << "Input game numbers line by line!" << std::endl;
    inputNumbers inputs;
    int i = 0;
    while(i < 3)
    {
        std::string str;
        std::getline(std::cin, str);

        if(str.size() != 9)
        {
            std::cout << "Wrong input count" << std::endl;
            continue;
        }

        for(int x = 0; x < GAME_WIDTH; ++x)
        {
            char charRead = str[x];
            if(charRead >= 48 && charRead <= 57)
                inputs[i * GAME_WIDTH + x] = charRead - 48;
            else
            {
                std::cout << "Only numbers allowed" << std::endl;
                continue;
            }
        }

        ++i;
    }
    
    NumberGame numberGame(inputs);
    numberGame.findSolution();

#elif defined FIXED_INPUT
    //inputNumbers inputs = {1,7,6,1,9,4,1,7,4,
    //                        8,5,2,3,2,3,5,2,3,
    //                        4,9,6,1,6,1,4,1,8};
    //inputNumbers inputs = {4,2,1,6,7,8,7,8,9,
    //                        7,5,7,5,9,5,4,5,1,
    //                        8,1,2,6,3,2,8,5,2};
    inputNumbers inputs = {5,5,7,9,6,1,2,9,5,
                            7,9,2,5,8,3,5,7,8,
                            3,8,7,1,4,9,2,9,6};
    //inputNumbers inputs = {5,5,7,9,6,1,2,9,0,
    //                        0,9,2,5,8,3,5,7,8,
    //                        3,8,7,1,4,9,2,9,6};
    NumberGame numberGame(inputs);
    numberGame.findSolution();
#else

    srand(2);
    NumberGame numberGame;
    numberGame.findSolution();

#endif
    return 0;
}