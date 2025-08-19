
// yoinked from engine

#include "NeuralNetwork.h"

#include <iostream>
#include <qdebug.h>
#include <qlogging.h>
#include <strstream>
#include <thread>
#include <algorithm>
#include <execution>
void NeuralNetwork::Create(int* Architecture, int archsize)
{
	//layers = new Layer[archsize];
	LayersAmount = archsize;
	sizein = Architecture[0];
	sizeout = Architecture[archsize - 1];


    NN_Size = 0;
    Weights_Size = 0;
	for (int i = 0; i < archsize; i++)
		NN_Size += Architecture[i];

	if (NN_Size == 0)
		std::cerr << "NN err1: Invalid size";


	for (int i = 1; i < archsize; i++)
		Weights_Size += Architecture[i - 1] * Architecture[i];

	if (Weights_Size == 0)
		std::cerr << "NN err2: Invalid weights size";

	outputs = new float[sizeout];
	Nodes = new float[NN_Size];

	biases = new float[NN_Size];
	biasGradients = new float[NN_Size];


	weights = new float[Weights_Size];
	weightGradients = new float[Weights_Size];
	weightOutputs = new float[Weights_Size];


	NodesAmount = NN_Size;
	WeightsAmount = Weights_Size;

	Arch = new int[archsize];
    NodesStep = new unsigned int[archsize];
	WeightsStep = new int[archsize];
	//memcpy(Arch, Architecture, sizeof(int) * archsize);

	int sum = 0;
	for (int i = 0; i < archsize; i++)
	{
		Arch[i] = Architecture[i];
		NodesStep[i] = sum;
		sum += Architecture[i];
	}
	sum = 0;
	WeightsStep[0] = 0;
	for (int i = 1; i < archsize; i++)
	{
		WeightsStep[i] = sum;
		sum += Arch[i - 1] * Arch[i];
	}

	Created = true;


}

void NeuralNetwork::Delete()
{
	delete[] outputs;
	delete[] Nodes;
	delete[] biases;
	delete[] biasGradients;


	delete[] weights;
	delete[] weightGradients;
	delete[] weightOutputs;


	delete[] Arch;
	delete[] NodesStep;
	delete[] WeightsStep;
	Created = false;
}

void NeuralNetwork::CreateGPU(int* Architecture, int archsize)
{
    if(!Created)
        Create(Architecture, archsize);

    glContext = new GlContext();
    glContext->setup();

    runShader = new ComputeShader();
    runShader->setup("./shader/compute_shader_run.csh", glContext);

    nodesBuffer = new ShaderBuffer();
    nodesBuffer->setup(glContext);
    //nodesBuffer->allocate(Nodes, NodesAmount * sizeof(float)); // Now in InitGPUBuffers

    biasesBuffer = new ShaderBuffer();
    biasesBuffer->setup(glContext);
    //biasesBuffer->allocate(biases, NodesAmount * sizeof(float)); // Now in InitGPUBuffers

    weightsBuffer = new ShaderBuffer();
    weightsBuffer->setup(glContext);
    //weightsBuffer->allocate(weights, WeightsAmount * sizeof(float)); // Now in InitGPUBuffers

    archBuffer = new ShaderBuffer();
    archBuffer->setup(glContext);
    //archBuffer->allocate(Arch, LayersAmount * sizeof(int)); // Now in InitGPUBuffers

    nodesStepBuffer = new ShaderBuffer();
    nodesStepBuffer->setup(glContext);
    //nodesStepBuffer->allocate(NodesStep, LayersAmount * sizeof(int)); // Now in InitGPUBuffers

    weightsStepBuffer = new ShaderBuffer();
    weightsStepBuffer->setup(glContext);
    //weightsStepBuffer->allocate(WeightsStep, LayersAmount * sizeof(int)); // Now in InitGPUBuffers

    inputBufferGPU = new ShaderBuffer();
    inputBufferGPU->setup(glContext);
    //inputBufferGPU->allocate(inputs, sizein * sizeof(float)); // Now in InitGPUBuffers

    outputBufferGPU = new ShaderBuffer();
    outputBufferGPU->setup(glContext);
    //outputBufferGPU->allocate(outputs, sizeout * sizeof(float)); // Now in InitGPUBuffers

    targetOutputBufferGPU = new ShaderBuffer();
    targetOutputBufferGPU->setup(glContext);
    //targetOutputBufferGPU->allocate(targetOutput, sizeout * sizeof(float)); // Now in InitGPUBuffers

    biasGradientsBufferGPU = new ShaderBuffer();
    biasGradientsBufferGPU->setup(glContext);
    //biasGradientsBufferGPU->allocate(biasGradients, NodesAmount * sizeof(float)); // Now in InitGPUBuffers

    weightGradientsBufferGPU = new ShaderBuffer();
    weightGradientsBufferGPU->setup(glContext);
    //weightGradientsBufferGPU->allocate(weightGradients, WeightsAmount * sizeof(float)); // Now in InitGPUBuffers

    InitGPUBuffers(); // Call new initialization function
}

void NeuralNetwork::InitGPUBuffers()
{
    nodesBuffer->allocate(Nodes, NodesAmount * sizeof(float));
    biasesBuffer->allocate(biases, NodesAmount * sizeof(float));
    weightsBuffer->allocate(weights, WeightsAmount * sizeof(float));
    archBuffer->allocate(Arch, LayersAmount * sizeof(int));
    nodesStepBuffer->allocate(NodesStep, LayersAmount * sizeof(int));
    weightsStepBuffer->allocate(WeightsStep, LayersAmount * sizeof(int));
    inputBufferGPU->allocate(nullptr, sizein * sizeof(float)); // Allocate, but don't transfer data yet
    outputBufferGPU->allocate(nullptr, sizeout * sizeof(float)); // Allocate, but don't transfer data yet
    targetOutputBufferGPU->allocate(nullptr, sizeout * sizeof(float)); // Allocate, but don't transfer data yet
    biasGradientsBufferGPU->allocate(nullptr, NodesAmount * sizeof(float)); // Allocate, but don't transfer data yet
    weightGradientsBufferGPU->allocate(nullptr, WeightsAmount * sizeof(float)); // Allocate, but don't transfer data yet

    // Bind buffers to shader binding points (these bindings are consistent across all shaders)
    nodesBuffer->bind_for_shader(0);
    inputBufferGPU->bind_for_shader(1);
    weightsBuffer->bind_for_shader(2);
    biasesBuffer->bind_for_shader(3);
    archBuffer->bind_for_shader(4);
    nodesStepBuffer->bind_for_shader(5);
    weightsStepBuffer->bind_for_shader(6);
    targetOutputBufferGPU->bind_for_shader(7);
    biasGradientsBufferGPU->bind_for_shader(8);
    weightGradientsBufferGPU->bind_for_shader(9);
    outputBufferGPU->bind_for_shader(10); // Changed binding point for outputBufferGPU to 10
}

void NeuralNetwork::UpdateWeightsAndBiases()
{
    nodesBuffer->allocate(Nodes, NodesAmount * sizeof(float));
    biasesBuffer->allocate(biases, NodesAmount * sizeof(float));
    weightsBuffer->allocate(weights, WeightsAmount * sizeof(float));
    archBuffer->allocate(Arch, LayersAmount * sizeof(int));
    nodesStepBuffer->allocate(NodesStep, LayersAmount * sizeof(int));
    weightsStepBuffer->allocate(WeightsStep, LayersAmount * sizeof(int));
}

void NeuralNetwork::UpdateGPUInput(float* inputData)
{
    inputBufferGPU->clear();
    inputBufferGPU->setup(glContext);
    inputBufferGPU->allocate(inputData, sizein * sizeof(float));
}

void NeuralNetwork::ReadGPUOutput()
{
    outputBufferGPU->read_to_cpu(outputs, sizeout * sizeof(float));
}

void NeuralNetwork::UpdateGPUTargetOutput(float* targetOutput)
{
    targetOutputBufferGPU->clear();
    targetOutputBufferGPU->setup(glContext);
    targetOutputBufferGPU->allocate(targetOutput, sizeout * sizeof(float));
}

void NeuralNetwork::ReadGPUWeightsBiases()
{
    weightsBuffer->read_to_cpu(weights, WeightsAmount * sizeof(float));
    biasesBuffer->read_to_cpu(biases, NodesAmount * sizeof(float));
}

void NeuralNetwork::DeleteGPU()
{
    delete glContext;
    delete runShader;
    delete backPropagateShader;
    delete nodesBuffer;
    delete biasesBuffer;
    delete weightsBuffer;
    delete archBuffer;
    delete nodesStepBuffer;
    delete weightsStepBuffer;
    delete inputBufferGPU;
    delete targetOutputBufferGPU;
    delete outputBufferGPU;
    delete biasGradientsBufferGPU;
    delete weightGradientsBufferGPU;
    Delete(); // Call base delete to free CPU memory
}

void NeuralNetwork::Randomize()
{
	for (int i = 0; i < WeightsAmount; i++)
		weights[i] = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f;

	for (int i = 0; i < NodesAmount; i++)
		biases[i] = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f;
	lastCost = -1;

}
void NeuralNetwork::Run(float* inputData)
{
	inputs = inputData;

	for (int i = 0; i < sizein; i++)
		Nodes[i] = inputData[i];

	int PrevNodeStart = 0;
	for (int i = 1; i < LayersAmount; i++)
	{
		int size = Arch[i];
		int weightsPerNode = Arch[i - 1];
		int node = 0;
		for (int n = NodesStep[i]; n < NodesStep[i] + size; n++)
		{//each node of layer

			//WeightsStep[i-1]

			float sum = 0.0f;
			int start = (WeightsStep[i - 1] + node * weightsPerNode);
			int PrevLayerNode = 0;
			for (int w = start; w < start + weightsPerNode; w++)
			{
				weightOutputs[i] = weights[w] * Nodes[PrevNodeStart + PrevLayerNode];
				sum += weightOutputs[i];

				PrevLayerNode++;
            }


            Nodes[n] = sigmoid(sum + biases[n]);



            node++;
		}
		PrevNodeStart = NodesStep[i];
	}
	for (int i = 0; i < sizeout; i++)
		outputs[i] = Nodes[PrevNodeStart + i];
}

void NeuralNetwork::RunGPU(float* inputData)
{
    // Upload input data to GPU
    UpdateGPUInput(inputData);

    // Bind buffers to shader binding points (these bindings are consistent across layers)
    // These are already bound in InitGPUBuffers(), no need to re-bind unless specific buffers change.

    // Set uniforms and dispatch for each layer

    nodesBuffer->bind_for_shader(0);
    inputBufferGPU->bind_for_shader(1);
    weightsBuffer->bind_for_shader(2);
    biasesBuffer->bind_for_shader(3);
    archBuffer->bind_for_shader(4);
    nodesStepBuffer->bind_for_shader(5);
    weightsStepBuffer->bind_for_shader(6);
    targetOutputBufferGPU->bind_for_shader(7);
    biasGradientsBufferGPU->bind_for_shader(8);
    weightGradientsBufferGPU->bind_for_shader(9);
    outputBufferGPU->bind_for_shader(10); // Changed binding point for outputBufferGPU to 10

    runShader->begin();
    runShader->program().setUniformValue("LayersAmount", LayersAmount);
    runShader->program().setUniformValue("sizein", sizein);
    runShader->program().setUniformValue("sizeout", sizeout);

    // First, process the input layer (copy input data to nodes)
    runShader->program().setUniformValue("currentLayerIndex", 0);

    runShader->compute(sizein, 1, 1); // Dispatch for each input node
    glContext->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Ensure writes are visible

    // Process hidden layers and output layer
    for (int i = 1; i < LayersAmount; i++)
    {
        runShader->program().setUniformValue("currentLayerIndex", i);
        runShader->compute(Arch[i], 1, 1); // Dispatch for each node in the current layer
        glContext->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Ensure writes are visible for the next layer
    }
    runShader->end();

    // Read results back to CPU (outputs buffer is already populated by the shader)
    ReadGPUOutput();
}
void NeuralNetwork::Run(float ActFunc(float), float* inputData)
{
	inputs = inputData;

	for (int i = 0; i < sizein; i++)
		Nodes[i] = inputData[i];

	int PrevNodeStart = 0;
	for (int i = 1; i < LayersAmount; i++)
	{
		int size = Arch[i];
		int weightsPerNode = Arch[i - 1];
		int node = 0;
		for (int n = NodesStep[i]; n < NodesStep[i] + size; n++)
		{//each node of layer

			//WeightsStep[i-1]

			float sum = 0.0f;
			int start = (WeightsStep[i - 1] + node * weightsPerNode);
			int PrevLayerNode = 0;
			for (int w = start; w < start + weightsPerNode; w++)
			{
				weightOutputs[i] = weights[w] * Nodes[PrevNodeStart + PrevLayerNode];
				sum += weightOutputs[i];

				PrevLayerNode++;
			}

			Nodes[n] = ActFunc(sum + biases[n]);
			node++;
		}
		PrevNodeStart = NodesStep[i];
	}
	for (int i = 0; i < sizeout; i++)
		outputs[i] = Nodes[PrevNodeStart + i];
}


// Backpropagation learning function
void NeuralNetwork::BackPropagate(float* input, float* targetOutput, float learningRate)
{
    // 1. Forward pass to calculate all node outputs
    Run(input);

    // 2. Calculate output layer errors (output gradients)
    // The derivative of the sigmoid function is sigmoid(x) * (1 - sigmoid(x))
    // For ReLu, the derivative is 1 if x > 0, else 0

    // Assuming sigmoid as the activation function for now.
    // A more robust solution would involve passing the activation function and its derivative.

    // For the output layer, the error is (predicted - actual) * derivative_of_activation(output)
    int outputLayerStartNode = NodesStep[LayersAmount - 1];
    for (int i = 0; i < sizeout; i++)
    {
        float predictedOutput = Nodes[outputLayerStartNode + i];
        float error = predictedOutput - targetOutput[i];
        // Sigmoid derivative: sigmoid(x) * (1 - sigmoid(x))
        float derivative = predictedOutput * (1.0f - predictedOutput);
        biasGradients[outputLayerStartNode + i] = error * derivative;
    }

    // 3. Backpropagate errors through hidden layers
    // Iterate backwards from the second to last layer down to the first hidden layer
    for (int l = LayersAmount - 2; l >= 0; l--)
    {
        int currentLayerStartNode = NodesStep[l];
        int nextLayerStartNode = NodesStep[l + 1];

        for (int i = 0; i < Arch[l]; i++) // Iterate through nodes in the current layer
        {
            float error = 0.0f;
            // Sum weighted errors from the next layer
            for (int j = 0; j < Arch[l + 1]; j++) // Iterate through nodes in the next layer
            {
                int weightIndex = WeightsStep[l] + (i * Arch[l + 1]) + j;
                error += biasGradients[nextLayerStartNode + j] * weights[weightIndex];
            }
            // Sigmoid derivative for hidden layer nodes
            float derivative = Nodes[currentLayerStartNode + i] * (1.0f - Nodes[currentLayerStartNode + i]);
            biasGradients[currentLayerStartNode + i] = error * derivative;
        }
    }

    // 4. Update weights and biases
    for (int l = 0; l < LayersAmount; l++)
    {
        int currentLayerStartNode = NodesStep[l];
        int nextLayerStartNode = NodesStep[l + 1];

        for (int i = 0; i < Arch[l]; i++) // Iterate through nodes in current layer
        {
            // Update biases
            if (l > 0) // Only update biases for hidden and output layers, not input
            {
                biases[currentLayerStartNode + i] -= learningRate * biasGradients[currentLayerStartNode + i];
            }

            // Update weights connecting to the next layer
            if (l < LayersAmount - 1) // Do not update weights from the output layer
            {
                for (int j = 0; j < Arch[l + 1]; j++) // Iterate through nodes in next layer
                {
                    int weightIndex = WeightsStep[l] + (i * Arch[l + 1]) + j;
                    weightGradients[weightIndex] = biasGradients[nextLayerStartNode + j] * Nodes[currentLayerStartNode + i];
                    weights[weightIndex] -= learningRate * weightGradients[weightIndex];
                }
            }
        }
    }

    // 5. Clear gradients for next iteration (optional, depending on how gradients are accumulated)
    // For this basic implementation, we calculate gradients fresh each time.
}

void NeuralNetwork::BackPropagateGPU(float* input, float* targetOutput, float learningRate)
{
    // Ensure forward pass is done to populate Nodes buffer
    RunGPU(input);

    // Upload target output to GPU
    UpdateGPUTargetOutput(targetOutput);

    // These buffers should already be allocated in InitGPUBuffers(), just transfer data if needed.
    // No need to check for null and new, just re-allocate/copy data.
    biasGradientsBufferGPU->clear();
    weightGradientsBufferGPU->clear();
    biasGradientsBufferGPU->setup(glContext);
    weightGradientsBufferGPU->setup(glContext);
    biasGradientsBufferGPU->allocate(biasGradients, NodesAmount * sizeof(float)); // Re-allocate to clear/reset gradients
    weightGradientsBufferGPU->allocate(weightGradients, WeightsAmount * sizeof(float)); // Re-allocate to clear/reset gradients

    // Bind buffers to shader binding points (these bindings are consistent across all shaders)
    // These are already bound in InitGPUBuffers(), no need to re-bind unless specific buffers change.

    // Initialize backpropagation shader if not already done
    if (!backPropagateShader)
    {
        backPropagateShader = new ComputeShader();
        backPropagateShader->setup("./shader/compute_shader_backpropagate.csh", glContext);
    }

    // Bind buffers to shader binding points (these bindings are consistent across all shaders)
    nodesBuffer->bind_for_shader(0);
    inputBufferGPU->bind_for_shader(1);
    weightsBuffer->bind_for_shader(2);
    biasesBuffer->bind_for_shader(3);
    archBuffer->bind_for_shader(4);
    nodesStepBuffer->bind_for_shader(5);
    weightsStepBuffer->bind_for_shader(6);
    targetOutputBufferGPU->bind_for_shader(7);
    biasGradientsBufferGPU->bind_for_shader(8);
    weightGradientsBufferGPU->bind_for_shader(9);
    outputBufferGPU->bind_for_shader(10); // Changed binding point for outputBufferGPU to 10
    backPropagateShader->begin();
    backPropagateShader->program().setUniformValue("LayersAmount", LayersAmount);
    backPropagateShader->program().setUniformValue("sizein", sizein);
    backPropagateShader->program().setUniformValue("sizeout", sizeout);
    backPropagateShader->program().setUniformValue("learningRate", learningRate);
    backPropagateShader->program().setUniformValue("weights_size", Weights_Size);
    backPropagateShader->program().setUniformValue("biases_size", NodesAmount);

    // Phase 1: Calculate output layer errors
    backPropagateShader->program().setUniformValue("currentLayerIndex", LayersAmount - 1);

    backPropagateShader->compute(sizeout, 1, 1); // Dispatch for each output node
    glContext->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); 

    // Phase 2: Backpropagate errors through hidden layers
    for (int l = LayersAmount - 2; l >= 1; l--) // Iterate backwards from second to last down to first hidden layer
    {
        backPropagateShader->program().setUniformValue("currentLayerIndex", l);
        backPropagateShader->compute(Arch[l], 1, 1); // Dispatch for each node in the current hidden layer
        glContext->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); 
    }


    // Phase 3: Update weights and biases
    for (int l = 0; l < LayersAmount - 1; l++) // Iterate from input layer up to second to last layer
    {
        backPropagateShader->program().setUniformValue("currentLayerIndex", l);
        backPropagateShader->compute(Arch[l], 1, 1); // Dispatch for each node in the current layer (responsible for outgoing weights)
        glContext->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); 
    }
    backPropagateShader->end();

    // Read updated weights and biases back to CPU
    //ReadGPUWeightsBiases();
}


void NeuralNetwork::ApplyDiff(float* inputData, float* outputFiff, float rate)
{
    inputs = inputData;

    for (int i = 0; i < sizein; i++)
        Nodes[i] = inputData[i];

    int PrevNodeStart = 0;
    for (int i = 1; i < LayersAmount; i++)
    {
        int size = Arch[i];
        int weightsPerNode = Arch[i - 1];
        int node = 0;
        for (int n = NodesStep[i]; n < NodesStep[i] + size; n++)
        {//each node of layer
            //WeightsStep[i-1]

            float sum = 0.0f;
            int start = (WeightsStep[i - 1] + node * weightsPerNode);
            int PrevLayerNode = 0;
            for (int w = start; w < start + weightsPerNode; w++)
            {
                weights[w] += outputFiff[n -  NodesStep[i]] * rate * Nodes[PrevNodeStart + PrevLayerNode];

                PrevLayerNode++;
            }
            node++;
        }
        PrevNodeStart = NodesStep[i];
    }
}

std::vector<int> rand_ids;
// inputs an array through a NN and calculates diviation from output 
float NeuralNetwork::Cost(float* input, float* output, int amount)
{
	float sum = 0.0f;



    // take random from amount
    float* arr = new float[sizein];

    for (int i=0;i < amount;i++)
    {

		for (int iter = 0; iter < sizein; iter++)
			arr[iter] = input[i * sizein + iter];
        Run(arr);
        float tmp = 0.0f;
		for (int a = 0; a < sizeout; a++)
		{
			tmp = outputs[a] - output[i * sizeout + a];

			tmp = tmp * tmp;
			sum += tmp;
		}
	}
    delete[] arr;

    //sum /= amount;
	return sum;
}
void NeuralNetwork::ApplyGrad()
{
	for (int i = 0; i < WeightsAmount; i++)
		weights[i] -= weightGradients[i];
	for (int i = 0; i < NodesAmount; i++)
		biases[i] -= biasGradients[i];


}
void NeuralNetwork::DeApplyGrad()
{
	for (int i = 0; i < WeightsAmount; i++)
		weights[i] += weightGradients[i];
	for (int i = 0; i < NodesAmount; i++)
		biases[i] += biasGradients[i];
}

void NeuralNetwork::SetupLearing()
{
	for (int i = 0; i < WeightsAmount; i++)
        weightGradients[i] = weights[i] * h * ((rand() % 100000) / 100000.0f - 0.5f) * 2.0f + h * ((rand() % 100000) / 10000000.0f - 0.5f) * 2.0f;
	for (int i = 0; i < NodesAmount; i++)
        biasGradients[i] = biases[i] * h * ((rand() % 100000) / 100000.0f - 0.5f) * 2.0f +  h * ((rand() % 100000) / 10000000.0f - 0.5f) * 2.0f;
	ApplyGrad();
}
void NeuralNetwork::ApplyLearing(float cost)
{
	if(lastCost < cost)
		DeApplyGrad();
}

void NeuralNetwork::CopyInto(NeuralNetwork* target)
{
	if(target->Created)
		target->Delete();
	target->Create(Arch,LayersAmount);

	for (int i = 0; i < WeightsAmount; i++)
		target->weights[i] = weights[i];

	for (int i = 0; i < NodesAmount; i++)
		target->biases[i] = biases[i];
		
}
void NeuralNetwork::CopyIntoWithGradient(NeuralNetwork* target)
{
	if(target->Created)
		target->Delete();
	target->Create(Arch,LayersAmount);

	for (int i = 0; i < WeightsAmount; i++)
		{
			target->weights[i] = weights[i] + weightGradients[i] * h * (((rand() % 100000) / 100000.0f) * 2.0f - 1.0f);
			target->weightGradients[i] = weightGradients[i];
		}

	for (int i = 0; i < NodesAmount; i++)
		{
			target->biases[i] = biases[i] + biasGradients[i] * h * (((rand() % 100000) / 100000.0f) * 2.0f - 1.0f);
			target->biasGradients[i] = biasGradients[i];
		}
}
// Uses Cost() to minimize the diviation. Method - finite difirences
void NeuralNetwork::learn(float rate, float* input, float* output, int amount)
{
    float h = 0.00001f;
    float c1 = Cost(input, output, amount);

    // reshuffle training data;
    rand_ids.clear();
    for(int i=0;i < 10;i++)
        rand_ids.push_back(rand()%amount);

    qDebug() <<  WeightsAmount <<  " bisaes:" << NodesAmount  << "data" << amount << " estimated datacount in " << amount * sizein;
	for (int i = 0; i < WeightsAmount; i++)
	{
		weights[i] += h;

		float c2 = Cost(input, output, amount);
		weights[i] -= h;
		float slope = (c2 - c1) / h;

		weightGradients[i] = slope * rate;
        //qDebug() <<  "weight["<<i << "]" <<weightGradients[i] << weights[i];
	}
	for (int i = 0; i < NodesAmount; i++)
	{
		biases[i] += h;

		float c2 = Cost(input, output, amount);
		biases[i] -= h;
		float slope = (c2 - c1) / h;

		biasGradients[i] = slope * rate;
    }

    qDebug() <<"Cost "<<c1;
	ApplyGrad();
	float cst = Cost(input, output, amount);

    qDebug() <<"Cost "<<cst;
	float cst2 = cst;
	bool run = true;
	while (run)
	{
		run = false;
		ApplyGrad();
		cst2 = Cost(input, output, amount);
        qDebug() <<"End Cost "<<cst2;
		if (cst2 < cst)
		{
			run = true;
            cst = cst2;
            qDebug() <<"End Cost "<<cst;
            lastCost = cst;
		}
		else
        {
			DeApplyGrad();
        }
    }
}
//std::string throutputs[12];

void NeuralNetwork::SaveTo(std::string filename)
{
	std::ofstream SaveFile(filename);
	SaveFile << LayersAmount;
	SaveFile << " " << WeightsAmount;
	SaveFile << " " << NodesAmount;
	SaveFile << "\n";

	for (int i = 0; i < LayersAmount; i++)
		SaveFile << Arch[i] << "\n";
	for (int i = 0; i < WeightsAmount; i++)
		SaveFile << weights[i] << "\n";
	for (int i = 0; i < NodesAmount; i++)
		SaveFile << biases[i] << "\n";
	SaveFile.close();

}
void NeuralNetwork::LoadFrom(std::string filename)
{
	Delete();

	std::ifstream f(filename);
	if (!f.is_open())
	{
		std::cerr << "ERROR LOADING NN: Unable to open " << filename;
		return;
	}
	int i = 0;
	int state = 0;
	while (!f.eof())
	{

		char junk;
		char line[32];
		f.getline(line, 32);
		std::strstream s;
		s << line;
		if (state == 0)
		{
			s >> LayersAmount >> WeightsAmount >> NodesAmount;
			Arch = new int[LayersAmount];
			state = 1;
		}
		else if (state == 1)
		{
			s >> Arch[i];

			i++;
			if (i >= LayersAmount)
			{
				i = 0;
				state = 2;

				int* Archbuff = Arch;

				Create(Arch, LayersAmount);
				delete[] Archbuff;
			}
		}
		else if (state == 2)
		{
			s >> weights[i];
			i++;
			if (i >= WeightsAmount)
			{
				i = 0;
				state = 3;
			}
		}
		else if (state == 3)
		{
			s >> biases[i];

			i++;
			if (i >= NodesAmount)
			{
				i = 0;
				state = 4;
			}
		}
	}
	f.close();
}
