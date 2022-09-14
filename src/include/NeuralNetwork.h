#pragma once
#include <fann.h>
#include <Image.h>
#include <vector>
#include <list>
#include <string>

#define NN_IMAGE_SIZE 20

class NeuralNetwork
{
    struct TrainData
    {
        std::vector<float> input;
        std::vector<float> output;
        int value;

        TrainData(const Image& image, int value, int countOutput);
    };


    const unsigned int num_input = NN_IMAGE_SIZE*NN_IMAGE_SIZE;
    const unsigned int num_output = 10;
    const unsigned int num_layers = 4;
    const unsigned int num_neurons_hidden1 = 100;
    const unsigned int num_neurons_hidden2 = 50;

    const float desired_error = 0.001f;
    const unsigned int max_epochs = 1000;
    const unsigned int epochs_between_reports = 5;

    std::list<TrainData> trainDataList;
    fann* ann;

public:

    NeuralNetwork();
    NeuralNetwork(const std::string& path);

    void addTrainingData(const Image& image, int value);

    void train();

    int run(const Image& image) const; 

    void save(const std::string& path) const;
};