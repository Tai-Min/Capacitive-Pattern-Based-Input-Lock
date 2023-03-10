#include <cmath>
#include <cstdint>
#include <iostream>
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

namespace ssie_model {
constexpr uint8_t intermediateBufferSize = 50;

constexpr uint8_t numInputs = 50;
constexpr uint8_t numClasses = 5;

constexpr uint8_t numOps = 5;
constexpr Operation ops[] = {
    Operation::INPUT,
    Operation::DENSE,
    Operation::DENSE,
    Operation::DENSE,
    Operation::OUTPUT,
};

constexpr Activation acts[] = {
    Activation::RELU,
    Activation::RELU,
    Activation::SOFTMAX,
};

constexpr uint8_t numNeurons[] = {
    10,
    20,
    5,
};

constexpr uint16_t length = 835;
constexpr float buffer[length] = {
-0.364011, -0.424963, 0.342720, 0.751269, 0.224845, 0.008038, 0.104422, 0.697863, 0.049296, 0.034230, 0.588535,
-0.519248, 0.649606, -0.262142, 0.020597, 0.205294, -0.092586, 0.488652, 0.509300, 0.056563, 0.014105, 0.375824,
0.114988, 0.228123, 0.047024, -0.210180, 0.748166, -0.354776, -0.152787, 0.237082, 0.230384, 0.108994, -0.349628,
-0.103753, -0.149387, -0.203209, -0.214462, -0.900913, 0.338526, -0.065012, -0.200787, 0.138340, -0.272033, 1.286009,
0.097188, -0.007557, 0.349709, -0.177329, 0.797012, -0.253732, -0.688728, 1.654845, -0.957436, -0.865752, 0.166031,
-0.426114, 0.066354, -0.738272, -0.841579, 0.372529, -0.379047, -0.694130, -0.701527, -1.390344, 0.248173, -0.638733,
-0.253549, 0.069809, -0.365269, -0.330599, 0.074734, -0.106963, 0.159999, 0.339822, -0.602838, 0.479502, -0.314455,
0.384571, 0.623735, -0.661816, 1.249738, -0.180615, 0.755473, 0.545207, -0.021741, 0.780970, 0.474811, 0.366391,
0.110040, -0.117759, 0.902296, 0.550281, -0.400223, -0.016563, 0.135443, -0.480243, 0.076222, -0.344855, -0.128310,
0.886921, -0.186802, 0.198635, -0.275997, -0.407018, 0.037382, -0.884041, -0.218684, -0.318456, -0.286835, 0.413708,
-0.808804, -0.161246, -0.591733, -0.535785, 0.623081, -0.500128, -0.218331, -0.342241, -0.275418, -0.181845, 0.821118,
-0.231910, 0.544588, -0.795917, -0.390678, 0.466742, -0.142445, 0.329222, -0.429239, -0.702948, 0.817834, -0.325120,
0.339580, -0.789899, -0.178103, 0.333598, -0.302207, 0.265403, -0.728761, 0.317627, -0.125917, -0.164223, 0.075819,
0.348556, 0.579944, -0.963578, 0.407782, -0.281465, 0.148596, 0.470981, -0.401146, 0.466863, 0.759014, -0.553031,
0.312493, 0.435200, 1.044780, -0.175565, 0.914155, 0.787427, 0.813045, 1.556293, 0.036643, 1.898018, 0.954832,
0.469725, 0.350790, 0.602220, 0.577661, -0.089517, 0.891711, 0.508293, 0.350589, 0.161255, 0.880851, 0.293236,
-0.635148, 0.280387, -0.244762, 0.620706, 0.097694, -0.526248, 0.480201, -0.233208, 0.569806, 0.180952, -0.546471,
-0.203800, -0.063160, 0.251104, -1.172258, 0.670529, -0.468284, -0.172777, 0.378952, -0.624640, 1.838546, -0.129966,
-0.457991, 0.479338, -0.173906, 0.662015, -0.429817, -0.357296, -0.125063, 0.089174, 0.122211, -0.004849, 0.304432,
-0.167037, 0.349224, 0.512279, -0.604491, 0.131088, -0.295620, 0.663626, 0.362238, -0.312527, -0.005769, -0.301295,
0.525904, -0.459679, -0.029676, 0.309410, -0.139938, -0.383300, -0.447017, 0.070229, 0.002816, 0.261185, 0.041037,
-0.606485, 0.270985, -0.830324, -0.001184, 0.182910, -0.080679, -0.015701, -0.511215, -0.121066, -0.091110, 0.049216,
-0.632848, -0.380582, -0.016490, 0.064774, -0.186958, -0.494332, 0.354161, 0.390086, 0.241418, 0.319264, 0.461874,
0.137608, 0.653988, 0.141217, -0.019824, -0.564909, -0.561708, 0.034803, -0.407811, -0.389861, -0.521263, -0.235671,
-0.270898, -0.968872, 0.137376, -0.115638, -0.436854, -0.207827, -0.475204, 0.530189, -0.486890, -0.654727, -0.172539,
-0.214395, 0.446961, 0.142941, -0.151671, 0.043993, -0.210298, 0.418389, -0.254909, -0.388278, -0.307103, -0.567819,
0.162936, 0.525498, -0.112448, -0.048427, -0.390099, -0.105160, 0.087373, -0.259411, -0.141402, 0.158490, 0.155249,
-0.329423, 0.205763, -0.186910, 0.339475, 0.253756, -0.129141, 0.214573, -0.239944, 0.513657, -0.046948, 1.313175,
0.476119, -0.145273, 1.396491, -0.173951, 0.575275, 0.576773, 0.356434, 0.103193, 0.503291, 0.772236, 0.441952,
0.103949, 0.345934, -0.106596, -0.279127, 0.212193, -0.970122, -0.164326, -0.317276, -0.460901, 0.181177, -0.326112,
-0.144283, -0.263499, 0.071061, 0.248393, -0.357779, -0.151369, -0.353509, 0.927107, -1.028374, 0.259494, -0.304279,
0.004784, 0.365966, 0.146718, 0.110953, 0.105969, 0.301068, 0.553958, 0.396438, -0.132960, -0.086160, 0.627940,
-0.473118, 0.148788, -0.685733, -0.083970, -0.084155, 0.527386, 0.219409, -0.907320, 0.174561, 0.304158, -0.111649,
0.694095, -0.106167, -0.646399, 0.159824, -0.215780, 0.485222, -0.219581, -1.467234, 0.187963, -0.049901, 0.535461,
-0.150456, -1.255749, 0.156156, -0.181722, 0.198598, 0.160722, -0.480942, 0.254717, -0.638015, -0.108163, 0.570887,
-0.490522, 0.461871, -0.089245, 0.221217, -0.251435, -0.266694, -0.235092, -0.466189, -0.244807, -0.124925, 0.212930,
-0.170409, 0.903356, -0.002779, -0.563061, 0.977879, -0.311266, 1.325616, -0.019641, -0.286731, 0.392483, -0.294846,
0.370812, 0.112943, -0.261231, 0.184304, 0.261487, 0.104390, 0.102626, 0.188851, -0.054944, 0.007999, -0.001661,
0.069696, -0.250452, -0.050764, -0.075067, -0.125826, 0.019662, 0.037901, -0.274805, -0.007610, -0.217754, -0.197440,
-0.325012, 0.088118, 0.234863, 0.014800, -0.069134, 0.218126, -0.047022, 0.444357, -0.132795, -0.353567, -0.186225,
0.159460, 0.187907, 0.140686, -0.068339, -0.165114, -0.083566, 0.230132, 0.263183, 0.190899, 0.328662, -0.139472,
-0.198617, 0.169805, 0.091059, -0.140221, -0.078720, -0.189681, 0.343866, -0.265512, 0.254217, -0.079556, -0.409995,
-0.030267, 0.087526, -0.377717, -0.607363, -0.205976, 0.068694, 0.018259, -0.310610, -0.306133, -0.270499, -0.172475,
-0.119360, -0.445467, -0.158224, -0.364281, 0.001609, -0.183025, -0.024441, 0.673689, 0.070322, -0.049428, -0.038301,
0.225802, -0.324519, 0.174724, -0.116914, 0.379400, 0.125506, 0.022412, 0.176187, 0.607965, 0.464960, 0.380425,
-0.270199, -0.197710, -0.375429, 0.260808, -0.180388, 0.321878, 0.401201, -0.656133, 0.327467, -0.362563, 0.036925, 
-0.239280, -0.036257, -0.093804, 0.214253, -0.348471, 1.341734, 0.624760, -0.506978, -0.922772, -0.070770, 0.247403,
-1.341764, 0.064926, 0.943880, 1.101568, 0.864162, 0.121657, -0.092476, 1.647228, -0.577507, -0.500343, -0.462797,
-0.990898, -0.194712, -0.009064, 0.748904, 0.795864, -0.712296, -0.207674, 0.998181, -0.436260, -0.504767, -0.947297,
-0.967910, 0.377482, -0.218237, -0.376279, 0.084789, -0.242567, 0.133439, 0.093308, 0.354115, -0.156573, -0.464694,
0.094469, -0.255353, -0.311687, -0.776954, 0.155099, -0.223892, -0.275399, -0.067646, -0.133758, -0.048075, -0.262222,
-0.375724, 0.097052, -0.383408, -0.613529, -0.032730, 0.112796, 0.158790, -0.341984, -0.026336, -0.150541, -0.365397,
-0.092737, 0.028889, 0.127413, -0.334386, -1.240939, 0.300879, -0.789393, -0.462808, -0.320998, 0.513253, 1.712300,
1.186863, 0.014850, -0.065826, 0.979218, -0.318380, -0.363021, -0.356545, -0.105816, 0.084276, -0.439769, -0.080123,
0.001485, 0.382409, 0.074127, -0.613900, 0.281268, -0.343367, 0.017648, -0.112828, -0.146986, 0.246206, 0.284796,
-0.251842, 0.307709, -0.153717, -0.428255, -0.363932, -0.060751, -0.229911, 0.093436, -0.443260, -0.328195, -0.083038,
0.145176, -0.433698, -0.125327, -0.441361, 0.137031, -0.066957, -0.092804, -0.015751, -0.160728, 0.055500, -0.029375,
-0.210686, -0.260316, 0.285383, -0.235266, 0.658528, -0.975267, -1.467823, 0.680313, 0.971429, 0.595605, 1.489829,
1.157205, -0.338622, -0.639887, 0.457455, -0.176801, -0.384412, -0.399183, -0.431166, -0.369750, 0.365206, 0.032610,
-0.411798, 0.324792, 0.086317, -0.336822, -0.519285, -1.478350, 1.317972, -1.944033, 1.279633, 0.420651, -0.549952,
-0.687976, -0.028356, -0.096995, 1.256900, 0.241115, 0.117729, 0.097174, -0.071358, -0.286999, 0.044935, 0.144559,
-0.427394, 0.443029, -0.208668, -0.307580, -0.231887, -0.399725, -0.267313, 0.004039, -0.210835, 0.250570, -0.440106,
-0.015512, 0.284444, -0.034967, -0.385583, -0.038984, 0.068293, -0.349067, -0.130696, -0.357481, 0.243275, 0.382895,
-0.158855, 0.145701, 0.100230, -0.383720, -0.241283, -0.140942, 0.074603, -0.235527, -0.427394, 0.064851, 0.219499,
-0.026233, 0.280560, -0.371619, -0.344048, -0.543663, 2.147722, -0.232169, 0.280381, 0.019164, 1.378723, -1.704314,
1.565416, 0.029653, 0.728870, 0.095694, 0.134696, 0.203105, -0.394804, -0.222215, 0.228542, -0.174808, 0.115622,
-0.264254, -0.173880, 0.246932, -0.607574, 0.082418, -0.455868, -1.187463, -0.511506, 0.239143, -0.314140, -0.925608,
0.044680, -0.075425, -0.126270, -0.012156, -1.020639, 0.293399, 2.429046, -0.229815, 0.107259, 0.027719, -0.190523,
-1.433556, 0.347968, 0.680759, 1.949534, 0.649733, -0.462905, -0.381956, 0.286910, 0.262345, 0.477329, 0.128461,
-0.082842, -0.168625, 0.213939, -2.262958, -0.354493, -0.266484, 0.454371, 0.357496, -0.244061, 0.232886, 1.605388,
0.164747, 0.563598, -0.635005, 1.768645, 1.726703, 0.346790, 0.171608, -0.108877, -1.839330, -0.016613, 0.103331,
-0.116977, -0.202747, 0.680986, 0.330443, -1.530361, 0.252303, -0.088257, 0.202030, -0.381243, 0.113175, 0.388503,
-0.665242, -1.275025, -1.077320, 0.837308, 0.855203, 0.028433, 0.215817, 1.400937, 0.147704, 0.182903, -0.147991,
0.251329, 0.731264, 0.436896, -0.035618, 0.427623, 0.192049, -0.185187, 0.203635, 2.239982, -0.194194, -0.221698,
-0.157434, -0.073909, -1.250363, -0.408609, -0.452744, -0.330340, 1.474704, -0.227501, 0.059544, 0.159216, 0.355384,
2.037440, 0.034912, -0.676066, -0.405610, -0.023475, -0.029114, 0.019564, -2.127137, -0.068582, -0.357427, };
};

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
            std::cout << "max" << std::endl;
            float maxVal = -1 * std::numeric_limits<float>::infinity();
            result = 255;
            for (uint8_t i = 0; i < ssie_model::numClasses; i++)
            {
                std::cout << buffer[currInputBuffer][i] << std::endl;
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

int main()
{
    uint8_t buf[] = {0, 0, 48, 0, 0, 2, 0, 0, 0, 0, 4, 0, 0, 0, 4, 32, 0, 0, 0, 1, 0, 1, 0, 64, 0, 0, 0, 0, 64, 0, 0, 17, 0, 0, 0, 32, 64, 0, 0, 16, 8, 0, 0, 0, 32, 2, 0, 0, 0, 0};
    int r = ssie::run(buf);

    std::cout << r << std::endl;
    return 0;
}