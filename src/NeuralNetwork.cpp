#include <NeuralNetwork.h>
#include <Debugging.h>

    
NeuralNetwork::TrainData::TrainData(const Image& image, int value) 
{
    this->value = value;
    int size = image.width * image.height;

    float scale = 1.0f / 255.0f;
    input.resize(size);
    for(int i = 0; i < size; ++i)
    {
        input[i] = (float)image.pixels[i] * scale;
    }

    if(value >= 0 && value <= 9)
    {
        output.resize(10);
        std::fill(&output.front(), &output.back(), 0.0f);
        output[value] = 1.0f;
    }
}

NeuralNetwork::NeuralNetwork()
{
    ann = fann_create_standard(num_layers, num_input, num_neurons_hidden1, num_neurons_hidden2, num_output);

    //fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_hidden(ann, FANN_ELLIOT);
    //fann_set_activation_function_hidden(ann, FANN_SIN);
    fann_set_activation_function_output(ann, FANN_SIGMOID);

    fann_set_training_algorithm(ann, FANN_TRAIN_RPROP);
}

void NeuralNetwork::addTrainingData(const Image& image, int value)
{
    ASSERT(image.width == IMAGE_SIZE && image.height == IMAGE_SIZE);
    trainDataList.emplace_back(image, value);
}

void NeuralNetwork::train()
{
    //fann_train_data* trainData = fann_create_train(trainDataList.size(), num_input, num_output);
    std::vector<float*> inputs;
    std::vector<float*> outputs;
    inputs.resize(trainDataList.size());
    outputs.resize(trainDataList.size());

    int index = 0;
    for(TrainData& td : trainDataList)
    {
        inputs[index] = td.input.data();
        outputs[index] = td.output.data();
        index += 1;
    }

    fann_train_data* trainData = fann_create_train_pointer_array(trainDataList.size(), num_input, inputs.data(), num_output, outputs.data());

    fann_train_on_data(ann, trainData, max_epochs, epochs_between_reports, desired_error);

    /*for(int i = 0; i < max_epochs; i++)
    {
        float error = fann_train_epoch(ann, trainData);
        printf("Epochs     %8d. Current error: %.10f\n", i, error);
        if(error <= desired_error)
            break;
    }*/
    fann_destroy_train(trainData);
}

int NeuralNetwork::run(const Image& image)
{
    ASSERT(image.width == IMAGE_SIZE && image.height == IMAGE_SIZE);

    TrainData td(image, -1);
    auto out = fann_run(ann, td.input.data());

    int val = 0;
    float maxStrength = 0.f;
    for(int i = 0; i < 10; ++i)
    {
        if(out[i] > maxStrength)
        {
            maxStrength = out[i];
            val = i;
        }
    }

    return val;
}