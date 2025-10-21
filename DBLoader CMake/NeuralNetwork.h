#pragma once

// class yoinked from Engine
#include <vector>
#include <string>
#include <fstream>
#include "qtcomputeshader/glwrappers.h"
class NeuralNetwork
{
public:

	bool Created = false;
	float* outputs;
	float* inputs;



	float* Nodes;//s

	float* biases;//s
	float* biasGradients;//s

	float* weights;//s*(s-1)
	float* weightGradients;//s*(s-1)
	float* weightOutputs;//s*(s-1)


	int* Arch;
    unsigned int* NodesStep;
	int* WeightsStep;



	int NodesAmount;
	int WeightsAmount;

	int sizein;
	int sizeout;
	int LayersAmount;

    int NN_Size = 0;
    int Weights_Size = 0;
	float lastCost = -1.0f;

	float rate = 1.0f;
	float c1 = 1.0f;
	float h = 1.0f;
	int gyms = 96;// used only in a method of randomizing everything and choosing best. better to be allignet to trheadcount of cpu

	void Create(int* Architecture, int archsize);
	void Randomize();
	void Run(float* inputData);
	void Run(float ActFunc(float), float* inputData);
    void ApplyDiff(float* inputData, float* outputFiff, float rate = 1.0f);


	// inputs an array through a NN and calculates diviation from output 
	float Cost(float* input, float* output, int amount);

	// Uses Cost() to minimize the diviation between outputs array and NN.outputs . Method - finite difirences. For more variety see CustomLearn()
	void learn(float rate, float* input, float* output, int amount);

    // Backpropagation learning function
    void BackPropagate(float* input, float* targetOutput, float learningRate);

    // GPU functions
    GlContext* glContext = nullptr;
    ComputeShader* runShader = nullptr;
    ComputeShader* backPropagateShader = nullptr;
    ShaderBuffer* nodesBuffer = nullptr;
    ShaderBuffer* biasesBuffer = nullptr;
    ShaderBuffer* weightsBuffer = nullptr;
    ShaderBuffer* archBuffer = nullptr;
    ShaderBuffer* nodesStepBuffer = nullptr;
    ShaderBuffer* weightsStepBuffer = nullptr;
    ShaderBuffer* inputBufferGPU = nullptr;
    ShaderBuffer* targetOutputBufferGPU = nullptr;
    ShaderBuffer* outputBufferGPU = nullptr;
    ShaderBuffer* biasGradientsBufferGPU = nullptr;
    ShaderBuffer* weightGradientsBufferGPU = nullptr;
    bool gpuCreated = false;

    void CreateGPU(int* Architecture, int archsize);
    void RunGPU(float* inputData, int amount = 1);
    void BackPropagateGPU(float* input, float* targetOutput, float learningRate, int amount = 1);
    void DeleteGPU();

    // New GPU memory management functions
    void UpdateWeightsAndBiases();
    void InitGPUBuffers(int amount = 1);
    void UpdateGPUInput(float* inputData,int amount = 1);
    void ReadGPUOutput(int amount = 1);
    void UpdateGPUTargetOutput(float* targetOutput,int amount = 1);
    void ReadGPUWeightsBiases();

	//Setups nn for future learning. After the learning iteration compleated, call ApplyLearning
	void SetupLearing();
	void ApplyLearing(float cost);

	//Copies itself into other nn;
	void CopyInto(NeuralNetwork* target);
	//Copies itself into other nn;
	void CopyIntoWithGradient(NeuralNetwork* target);

	void ApplyGrad();
	void DeApplyGrad();

	// unimplemented, use to make a custom NN learning behaviour
	std::vector<NeuralNetwork*> DataStorage;
	

	void SaveTo(std::string filename);
	void LoadFrom(std::string filename);

	void Delete();
	~NeuralNetwork()
	{
		Delete();
	}
};

inline float sigmoid(float x)
{
    return 1 / (1 + pow(2.71828, -x));
}

inline float sigmoidApprox(float x)
{
    //if (x > -2.05f && x < 2.05f)
    //    return sin((x * 0.2f + 0.5f)*3.14159f);

    //if (x < -5)
    //    return sin(0.0f);

    //if (x > 5)
    //    return sin(3.14159f);

    //if (x < -2.05f)
    //    return sin((x * 0.03f + 0.15f)*3.14159f);

    //if (x > 2.05f)
    //    return sin((x * 0.03f + 0.85f)*3.14159f);

    if(x >= -1.0f && x <= 1.0f)
        return (sin(((x)*0.5f)*3.14159f) + 1.0f) * 0.5f;
    else if (x > 1.0f)
        return 0.95f + x * 0.05f;
    else
        return 0.05 + x * 0.05f ;
}
inline float ReLu(float x)
{
    return x >= 0.0f ? x : 0.0f;
}
