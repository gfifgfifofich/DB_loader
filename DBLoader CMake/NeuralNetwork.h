#pragma once

// class yoinked from Engine
#include <vector>
#include <string>
#include <fstream>
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
	int* NodesStep;
	int* WeightsStep;



	int NodesAmount;
	int WeightsAmount;

	int sizein;
	int sizeout;
	int LayersAmount;

	float lastCost = -1.0f;

	float rate = 1.0f;
	float c1 = 1.0f;
	float h = 1.0f;
	int gyms = 96;// used only in a method of randomizing everything and choosing best. better to be allignet to trheadcount of cpu

	void Create(int* Architecture, int archsize);
	void Randomize();
	void Run(float* inputData);
	void Run(float ActFunc(float), float* inputData);

	// inputs an array through a NN and calculates diviation from output 
	float Cost(float* input, float* output, int amount);

	// Uses Cost() to minimize the diviation between outputs array and NN.outputs . Method - finite difirences. For more variety see CustomLearn()
	void learn(float rate, float* input, float* output, int amount);

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
