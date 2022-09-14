#include <NeuralNetwork.h>
#include <Debugging.h>

    
NeuralNetwork::TrainData::TrainData(const Image& image, int value, int countOutput) 
{
    this->value = value;
    int size = image.width * image.height;

    float scale = 1.0f / 255.0f;
    input.resize(size);
    for(int i = 0; i < size; ++i)
    {
        input[i] = (float)image.pixels[i] * scale;
    }

    ASSERT(value >= 0 && value < countOutput)

    output.resize(countOutput);
    std::fill(output.begin(), output.end(), 0.0f);

    output[value] = 1.0f;
}

NeuralNetwork::NeuralNetwork()
{
    ann = fann_create_standard(num_layers, num_input, num_neurons_hidden1, num_neurons_hidden2, num_output);

    //fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_hidden(ann, FANN_SIGMOID);
    //fann_set_activation_function_hidden(ann, FANN_SIN);
    fann_set_activation_function_output(ann, FANN_SIGMOID);

    fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL); // FANN_TRAIN_RPROP FANN_TRAIN_INCREMENTAL
}

NeuralNetwork::NeuralNetwork(const std::string& path)
{
    ann = fann_create_from_file(path.c_str());
}

void NeuralNetwork::addTrainingData(const Image& image, int value)
{
    ASSERT(image.width == NN_IMAGE_SIZE && image.height == NN_IMAGE_SIZE);
    trainDataList.emplace_back(image, value, num_output);
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

    /*int endI = 0;
    for(int i = 0; i < 100; ++i)
    {
        int epochs = 1;
        fann_train_on_data(ann, trainData, epochs, 1, desired_error);

        int startI = endI;
        endI = startI + 10;
        if(endI >= trainDataList.size())
        {
            startI = 0;
            endI = startI + 10;
        }

        int c = 0;
        for(TrainData& td : trainDataList)
        {
            if(c >= startI && c < endI)
            {
                auto out = fann_run(ann, td.input.data());
                printf("Number: %.1f Output: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", (float)td.value, out[0], out[1],out[2],out[3],out[4],out[5],out[6],out[7],out[8],out[9],out[10]);
                if(c == 9)
                    break;
            }
            ++c;
        }
    }*/

    fann_train_on_data(ann, trainData, max_epochs, epochs_between_reports, desired_error);

    /*for(int i = 0; i < max_epochs; i++)
    {
        float error = fann_train_epoch(ann, trainData);
        printf("Epochs     %8d. Current error: %.10f\n", i, error);
        if(error <= desired_error)
            break;
    }*/

    int c = 0;
    for(TrainData& td : trainDataList)
    {
        auto out = fann_run(ann, td.input.data());
        printf("Number: %.1f Output: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", (float)td.value, out[0], out[1],out[2],out[3],out[4],out[5],out[6],out[7],out[8],out[9]);
        //if(c == 100)
        //    break;
        //++c;
    }

    fann_destroy_train(trainData);
}

int NeuralNetwork::run(const Image& image) const
{
    ASSERT(image.width == NN_IMAGE_SIZE && image.height == NN_IMAGE_SIZE);

    float scale = 1.0f / 255.0f;
    int size = image.width * image.height;
    std::vector<float> input;
    input.resize(size);
    for(int i = 0; i < size; ++i)
    {
        input[i] = (float)image.pixels[i] * scale;
    }

    auto out = fann_run(ann, input.data());

    int val = 0;
    float maxStrength = 0.f;
    for(int i = 0; i < num_output; ++i)
    {
        if(out[i] > maxStrength)
        {
            maxStrength = out[i];
            val = i;
        }
    }
        
    return val;
}

void NeuralNetwork::save(const std::string& path) const
{
    fann_save(ann, path.c_str());
}