/*****************************************************************************
 * ==> Single perceptron demo -----------------------------------------------*
 *****************************************************************************
 * Description : A demo showing how to resolve a problem using a perceptron  *
 * Developer   : Jean-Milost Reymond                                         *
 * Note        : Based on the following article: http://blog.refu.co/?p=935  *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

//-----------------------------------------------------------------------------
#define M_INPUT_PATTERNS          13
#define M_LEAST_MEAN_SQUARE_ERROR 0.001f
#define M_TEACHING_STEP           0.01f
#define M_CLASS_BLUE              1
#define M_CLASS_RED               0
#define M_Normalize(x)            (float)((float)x / 255.0f)
//-----------------------------------------------------------------------------
// for this example let's have a few RGB color values
// as our input as can be seen in this array
int inputTable[] =
{
    // RED GREEN BLUE CLASS
    0,     0,    255, M_CLASS_BLUE,
    0,     0,    192, M_CLASS_BLUE,
    243,   80,   59,  M_CLASS_RED,
    255,   0,    77,  M_CLASS_RED,
    77,    93,   190, M_CLASS_BLUE,
    255,   98,   89,  M_CLASS_RED,
    208,   0,    49,  M_CLASS_RED,
    67,    15,   210, M_CLASS_BLUE,
    82,    117,  174, M_CLASS_BLUE,
    168,   42,   89,  M_CLASS_RED,
    248,   80,   68,  M_CLASS_RED,
    128,   80,   255, M_CLASS_BLUE,
    228,   105,  116, M_CLASS_RED,
};
//-----------------------------------------------------------------------------
/**
* Activation functions enumeration
*/
typedef enum
{
    E_AF_Thresold = 0,
    E_AF_Sigmoid,
    E_AF_Hyperbolic_Tangent,
} EActivationFuncs;
//-----------------------------------------------------------------------------
/**
* Perceptron neuron
*/
typedef struct
{
    float*           m_pInputs;
    float*           m_pWeights;
    int              m_Count;
    EActivationFuncs m_ActivationFunction;
} QR_Perceptron;
//-----------------------------------------------------------------------------
/**
* Creates and initializes a perceptron
*@param inputNumber - number of inputs to create and initialize
*@param function - net function to apply
*@return perceptron
*/
QR_Perceptron* Create(int inputNumber, EActivationFuncs function)
{
    int            i;
    QR_Perceptron* pPerceptron;

    // seed the random function with the current time
    srand((unsigned)time(0));

    // create a perceptron
    pPerceptron = (QR_Perceptron*)malloc(sizeof(QR_Perceptron));

    // create memory for inputs and weights, ans set function to use
    pPerceptron->m_Count              = inputNumber;
    pPerceptron->m_pInputs            = (float*)malloc(inputNumber * sizeof(float));
    pPerceptron->m_pWeights           = (float*)malloc(inputNumber * sizeof(float));
    pPerceptron->m_ActivationFunction = function;

    // populate the weight vector with values between -0.5 and 0.5
    for (i = 0; i < inputNumber; ++i)
        pPerceptron->m_pWeights[i] =
                (((float)rand() / ((float)RAND_MAX + 1.0f)) - 0.5f);

    return pPerceptron;
}
//-----------------------------------------------------------------------------
/**
* Releases a previously created perceptron
*@param pPerceptron - perceptron to release
*/
void Release(QR_Perceptron* pPerceptron)
{
    free(pPerceptron->m_pInputs);
    free(pPerceptron->m_pWeights);
    free(pPerceptron);
}
//-----------------------------------------------------------------------------
/**
* Sets perception input
*@param index - input index to set
*@param value - input value
*/
void SetInput(QR_Perceptron* pPerceptron, int index, float value)
{
    pPerceptron->m_pInputs[index] = value;
}
//-----------------------------------------------------------------------------
/**
* Calculates perceptron result, that can be connected to a net
*@param pPerceptron - perceptron to evaluate
*@return perceptron result to connect to the net
*/
float CalculateNet(QR_Perceptron* pPerceptron)
{
    int   i;
    float action = 0.0f;

    // get the potential action for this input pattern
    for (i = 0; i < pPerceptron->m_Count; ++i)
        action += pPerceptron->m_pInputs[i] * pPerceptron->m_pWeights[i];

    // decide what to do depending on the used activation function
    switch (pPerceptron->m_ActivationFunction)
    {
        case E_AF_Thresold:
            // the treshhold function is simple. The result is 1 if x >= 0,
            // and 0 if it is less than zero
            if (action >= 0.0f)
                action = 1.0f;
            else
                action = 0.0f;

            break;

        case E_AF_Sigmoid:
            // the sigmoid function is a little more complicated. It forms a
            // sigma "S" in a x-y graph, hence its name. It smoothens the output
            action = 1.0f / (1.0f + exp(-action));
            break;

        case E_AF_Hyperbolic_Tangent:
            // the hyperbolic tangent function is sometimes used instead of the sigmoid one
            action = (exp(action) - exp(-action)) / (exp(action) + exp(-action));
            break;
    }

    return action;
}
//-----------------------------------------------------------------------------
/**
* Adjusts perceptron weights
*@param pPerceptron - perceptron containing weights to adjust
*@param teachingStep - teaching step
*@param output - output that the perceptron found
*@param target - correct output the perceptron should find
*/
void AdjustWeights(QR_Perceptron* pPerceptron,
                   float          teachingStep,
                   float          output,
                   float          target)
{
    int i;

    // for all weights, adjust the weight vector to achieve network training
    for (i = 0; i < pPerceptron->m_Count; ++i)
        pPerceptron->m_pWeights[i] +=
                teachingStep * (target - output) * pPerceptron->m_pInputs[i];
}
//-----------------------------------------------------------------------------
/**
* Query perceptron about a value
*@param pPerceptron - perceptron to recall
*@param red - red value
*@param green - green value
*@param blue - blue value
*@return perceptron result
*/
float Recall(QR_Perceptron* pPerceptron, float red, float green, float blue)
{
    pPerceptron->m_pInputs[0] = red;
    pPerceptron->m_pInputs[1] = green;
    pPerceptron->m_pInputs[2] = blue;

    return CalculateNet(pPerceptron);
}
//-----------------------------------------------------------------------------
/**
* Checks if a stream contains only numeric values
*@param input - input stream
*@return 1 if stream contains only numeric values, otherwise 0
*/
int IsNumeric(char input[1024])
{
    int i;

    for (i = 0; i < 1024; ++i)
    {
        // found string terminating character?
        if (input[i] == '\0')
           break;

        // is digit?
        if (input[i] >= '0' && input[i] <= '9')
            continue;

        return 0;
    }

    return 1;
}
//-----------------------------------------------------------------------------
/**
* Checks if perceptron is trained
*@param mse - mean square error
*@param funcNum - function number to use
*@return 1 if perceptron is trained, otherwise 0
*/
int IsTrained(float mse, int funcNum)
{
    // dispatch function to use
    switch (funcNum)
    {
        case 0: return (mse <= 0.0f && mse != 999.0f);
        case 1: return (fabs(mse - M_LEAST_MEAN_SQUARE_ERROR) <= 0.0001f);
        case 2: return (fabs(mse - M_LEAST_MEAN_SQUARE_ERROR) <= 0.0001f);
    }

    return 0;
}
//-----------------------------------------------------------------------------
/**
* Main entry point
*@return application result
*/
int main()
{
    float          output;
    float          result;
    float          mse;
    float          error;
    int            j;
    int            k;
    int            r;
    int            g;
    int            b;
    int            epochs;
    int            inputCounter;
    int            funcNum;
    QR_Perceptron* pAnn = 0;

    // to hold the console input
    char input[1024];

    funcNum = -1;

    // query user about algorithm to use
    while (funcNum < 0 || funcNum > 2)
    {
        printf("Which algorithm to use to evaluate the perceptron output?\r\n");
        printf("1 - Thresold\r\n");
        printf("2 - Sigmoid\r\n");
        printf("3 - Hyperbolic tangent (NOT WORKING YET)\r\n");
        scanf("%s", input);

        if (!IsNumeric(input))
            continue;

        funcNum = atoi(input) - 1;

        if (funcNum < 0 || funcNum > 2)
            continue;
    }

    // dispatch function to use
    switch (funcNum)
    {
        case 0:
            // create a perceptron with 3 inputs, using the thresold as
            // activation function
            pAnn = Create(3, E_AF_Thresold);
            break;

        case 1:
            // create a perceptron with 3 inputs, using the sigmoid as
            // activation function
            pAnn = Create(3, E_AF_Sigmoid);
            break;

        case 2:
            // create a perceptron with 3 inputs, using the hyprrbolic
            // tangent as activation function
            pAnn = Create(3, E_AF_Hyperbolic_Tangent);
            break;
    }

    mse    = 999.0f;
    epochs = 0;

    // the training of the neural network
    while (!IsTrained(mse, funcNum))
    {
        error        = 0.0f;
        mse          = 0.0f;
        inputCounter = 0;

        // run through all 13 input patterns, what we call an EPOCH
        for (j = 0; j < M_INPUT_PATTERNS; ++j)
        {
            // give the 3 RGB values to the network
            for (k = 0; k < 3; ++k)
            {
                SetInput(pAnn, k, M_Normalize(inputTable[inputCounter]));
                ++inputCounter;
            }

            // get the output of this particular RGB pattern
            output = CalculateNet(pAnn);

            // add the error for this iteration to the total error
            error += fabs((float)inputTable[inputCounter] - output);

            // adjust the weights according to that error
            AdjustWeights(pAnn, M_TEACHING_STEP, output, inputTable[inputCounter]);

            // next pattern
            ++inputCounter;
        }

        // compute the mean square error for this epoch
        mse = error / (float)M_INPUT_PATTERNS;

        printf("The mean square error of %d epoch is %.8f\r\n", epochs, mse);
        ++epochs;
    }

    r = -1;
    g = -1;
    b = -1;

    while (true)
    {
        // get red input
        while (r < 0 || r > 255)
        {
            printf("Give a RED value (0-255)\r\n");
            scanf("%s", input);

            if (!IsNumeric(input))
                continue;

            r = atoi(input);
        }

        // get green input
        while (g < 0 || g > 255)
        {
            printf("Give a GREEN value (0-255)\r\n");
            scanf("%s", input);

           if (!IsNumeric(input))
                continue;

            g = atoi(input);
        }

        // get blue input
        while (b < 0 || b > 255)
        {
            printf("Give a BLUE value (0-255)\r\n");
            scanf("%s", input);

            if (!IsNumeric(input))
                continue;

            b = atoi(input);
        }

        // recall the neural network
        result = Recall(pAnn, M_Normalize(r), M_Normalize(g), M_Normalize(b));

        if (result > 0.5f)
            printf("The value you entered belongs to the BLUE CLASS\r\n");
        else
            printf("The value you entered belongs to the RED CLASS\r\n");

        printf("Do you want to continue with trying to recall values from the perceptron?\r\n");
        printf("Press any key for YES and 'N' for no, to exit the program\r\n");
        scanf("%s", input);

        // do quit?
        if (input[0] == 'n' || input[0] == 'N')
            break;

        // reset for next run
        r = g = b = -1;
    }

    Release(pAnn);
    return 0;
}
//-----------------------------------------------------------------------------

