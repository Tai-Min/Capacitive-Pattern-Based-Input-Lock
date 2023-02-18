#include "ssie.hpp"

#include "helpers.hpp"
#include <cmath>
#include <limits>

namespace
{
    enum class Operation
    {
        INPUT = 0,
        DENSE = 1,
        OUTPUT = 255
    };

    enum class Activation
    {
        RELU = 0,
        SOFTMAX = 1
    };
}

#include "ssie_model.hpp"

namespace ssie
{
    uint8_t run(const uint8_t *inputBuf);

    namespace
    {
        void inputOp(const uint8_t *inputBuf);
        void denseOp();
        void outputOp();

        float activation(Activation act, float neuronResult);
        void softmaxOp();
    }

    namespace
    {
        uint8_t actCursor = 0;
        uint8_t neuronCursor = 0;
        uint16_t modelCursor = 0;

        bool currInputBuffer = 0;
        bool currOutputBuffer = 1;

        uint16_t activationLength = 0;

        float buffer[2][ssie_model::intermediateBufferSize];

        uint8_t result;
    };

    uint8_t run(const uint8_t *inputBuf)
    {
        actCursor = 0;
        neuronCursor = 0;
        modelCursor = 0;

        currInputBuffer = 0;
        currOutputBuffer = 1;

        for(uint8_t i = 0; i < ssie_model::numOps; i++)
        {
            Operation currOp = ssie_model::ops[i];

            switch (currOp)
            {
            case Operation::INPUT:
                inputOp(inputBuf);
                break;
            case Operation::DENSE:
                denseOp();
                break;
            case Operation::OUTPUT:
                outputOp();
                break;
            }
        }

        return result;
    }

    namespace
    {
        void inputOp(const uint8_t *inputBuf)
        {
            for (uint8_t i = 0; i < ssie_model::numInputs; i++)
            {
                buffer[currOutputBuffer][i] = inputBuf[i] / 255.0;
            }

            currInputBuffer = !currInputBuffer;
            currOutputBuffer = !currOutputBuffer;
            activationLength = ssie_model::numInputs;
        }

        void denseOp()
        {
            Activation act = ssie_model::acts[actCursor++];
            uint8_t numNeurons = ssie_model::numNeurons[neuronCursor++];

            for (uint8_t neuron = 0; neuron < numNeurons; neuron++)
            {
                // MatMul
                float neuronResult = 0;
                for (uint8_t activation = 0; activation < activationLength; activation++)
                {
                    neuronResult += ssie_model::buffer[modelCursor++] * buffer[currInputBuffer][activation];
                }

                // BiasAdd
                float bias = ssie_model::buffer[modelCursor++];
                neuronResult += bias;

                // Activation and zero point
                if (act != Activation::SOFTMAX)
                {
                    buffer[currOutputBuffer][neuron] = activation(act, neuronResult);
                }
                else
                {
                    buffer[currOutputBuffer][neuron] = neuronResult;
                }
            }

            if (act == Activation::SOFTMAX)
            {
                
                softmaxOp();
            }

            currInputBuffer = !currInputBuffer;
            currOutputBuffer = !currOutputBuffer;
            activationLength = numNeurons;
        }

        void outputOp()
        {
            float maxVal = -1 * std::numeric_limits<float>::infinity();
            result = 255;
            for (uint8_t i = 0; i < ssie_model::numClasses; i++)
            {
                if (buffer[currInputBuffer][i] > maxVal)
                {
                    maxVal = buffer[currInputBuffer][i];
                    result = i;
                }
            }
        }

        float activation(Activation act, float neuronResult)
        {
            if (act == Activation::RELU)
            {
                return (neuronResult > 0 ? neuronResult : 0);
            }
            return 0;
        }

        void softmaxOp()
        {
            float maxVal = -1 * std::numeric_limits<float>::infinity();
            for (uint8_t i = 0; i < ssie_model::numClasses; i++)
            {
                if (buffer[currOutputBuffer][i] > maxVal)
                {
                    maxVal = buffer[currOutputBuffer][i];
                }
            }

            float sum = 0;
            for (uint8_t i = 0; i < ssie_model::numClasses; i++)
            {
                buffer[currOutputBuffer][i] = std::exp(buffer[currOutputBuffer][i] - maxVal);
                sum += buffer[currOutputBuffer][i];
            }

            for (uint8_t i = 0; i < ssie_model::numClasses; i++)
            {
                buffer[currOutputBuffer][i] = buffer[currOutputBuffer][i] / sum;
            }
        }
    };
};