#include "stdafx.h"
#include "Network.h"
using namespace std;


// ctor
Network::Network(initializer_list<int> initializerList)
{
	Init(initializerList);
	RandomizeWeights(); // Calculate weights
}

Network::Network(initializer_list<int> initializerList, NetworkTopology& topology)
{
	Init(initializerList);
	FillWeights(topology); // Calculate weights
}

// dector
Network::~Network()
{
	// cleanup
	DeleteWeights();
	delete[] this->hiddenNeuronsCount;
	delete this->topology;
}

// Train network and adjust weights to expectedOutput
double Network::Train(double* inputValues, int length, double expectedOutput)
{
	double output = Feed(inputValues, length);

	// TODO: Adjust weights and biasses here

	return output;
}

// Feed the network information and return the output
double Network::Feed(double* inputValues, int length)
{
	int lindex = length - 1; // Last index of inputValues (and eff. weights)

	double* values = inputValues; // Values of current layer
	int* valuesLength = &length;
	for (int hiddenIndex = 0; hiddenIndex < this->hiddenLayersCount; hiddenIndex++) // Loop through each hidden layer
	{
		double* nextValues = ToNextLayer(values, *valuesLength, hiddenIndex, *valuesLength);

		if (values != inputValues)
			delete[] values; // Cleanup old to-be-overridden values
		values = nextValues;
	}

	double sum = 0;
	for (int i = 0; i < length; i++) // Loop through each neuron in output layer
	{
		double value = Rectify(values[i]); // ReLU it (keep if positive, 0 if negative; uint)
		sum += Squash(value); // Squash the result
	}

	// Cleanup
	return sum;
}

// This function focuses on only one layer, so in theory we have 1 input layer, the layer we focus on, and 1 output
double* Network::ToNextLayer(double* inputValues, int inputLength, int layerIndex, int& outLength)
{
	int nCount = this->hiddenNeuronsCount[layerIndex]; // Count of neurons in the given layer (w/ layerIndex)
	double** weights = this->weights[layerIndex]; // ptr to weights of neurons in this layer
	double* biases = this->biases[layerIndex]; // ptr to biases of neurons in the layer before this
	double* output = new double[nCount];

	for (int n = 0; n < nCount; n++) // Loop through each neuron "n" on the following layer
	{
		double value = 0; // Current layer's value

		// Loop through each value in the inputs (every input broadcasts to all neurons in this layer)
		for (int ii = 0; ii < inputLength; ii++) // ii = input index
		{
			value += (inputValues[ii] * weights[ii][n]) + biases[n]; // Add Value * Weight to that neuron
		}

		double squashed = Squash(value); // The final squashed neuron's value
		this->layers[layerIndex][n] = squashed;
		output[n] = squashed; // Add value to output layer
	}

	outLength = nCount;
	return output;
}

// TODO: Randomizing should not respect input layer, just go from 0 to hiddenlayercount+1
void Network::RandomizeWeights()
{
	this->topology = new NetworkTopology();

	int totalLayerCount = hiddenLayersCount + 2; // Input Layer + Hidden Layers + Output Layer


	Layer inputLayer;
	// Fill input layer -> first hidden layer
	for (int n = 0; n < inputNeuronsCount; n++) // Loop through each neuron "n" on input layer
	{
		Neuron neuron;
		neuron.Bias = (double(rand() % 200) / 100) - 1; // Random number between 0 and 2, minus 1 (so between -1 and 1)
		int nextn = this->hiddenNeuronsCount[0]; // Count of neurons in first hidden layer
		for (int c = 0; c < nextn; c++) // Loop through each connection "c" of this neuron
		{
			Connection connection; // Build up connection with random weight
			connection.Weight = (double(rand() % 200) / 100) - 1; // Random number between 0 and 2, minus 1 (so between -1 and 1)
			neuron.AddConnection(connection); // add connection
		}

		inputLayer.AddNeuron(neuron); // Add the neuron with connections to first hidden layer
	}
	this->topology->AddLayer(inputLayer);

	// Fill all hidden layers
	for (int l = 0; l < hiddenLayersCount; l++) // Loop through each layer
	{
		Layer layer;

		for (int n = 0; n < hiddenNeuronsCount[l]; n++) // Loop through each neuron
		{
			Neuron neuron;
			neuron.Bias = (double(rand() % 200) / 100) - 1; // Random number between 0 and 2, minus 1 (so between -1 and 1)

			int nextNeurons;
			int next = l + 1;
			if (next == hiddenLayersCount) // Check if out of bounds
				nextNeurons = outputNeuronsCount; // It's last layer; Use output layer
			else
				nextNeurons = hiddenNeuronsCount[next]; // Just use next layer

			for (int c = 0; c < nextNeurons; c++) // Loop through each Connection
			{
				Connection connection;
				connection.Weight = (double(rand() % 200) / 100) - 1; // Random number between 0 and 2, minus 1 (so between -1 and 1)

				neuron.AddConnection(connection); // Add Connection from neuron `n`
			}

			layer.AddNeuron(neuron); // Add Neuron from layer `l`
		}

		topology->AddLayer(layer); // Add Layer
	}

	FillWeights(*this->topology);
}

// TODO: Check if this works
void Network::FillWeights(NetworkTopology& topology)
{
	this->topology = &topology;

	// layer weights has a reference on the heap
	if (this->weights != nullptr)
	{
		// delete the reference
		DeleteWeights();
	}

	int count = this->hiddenLayersCount + 1; // Count of layers with connections
	this->weights = new double**[count]; // init first dimension; count of layers with connections

	
	int lcount = topology.Size; // Count of layers
	this->biases = new double*[lcount];
	this->weights = new double**[lcount];
	for (int l = 0; l < lcount; l++) // Loop through each layer
	{
		Layer& layer = topology.LayerAt(l);
		int ncount = layer.Size; // Count of neurons in this layer
		this->biases[l] = new double[ncount];
		this->weights[l] = new double*[ncount];
		for (int n = 0; n < ncount; n++) // Loop through each neuron in this layer
		{
			Neuron& neuron = layer.NeuronAt(n);

			this->biases[l][n] = neuron.Bias;
			int ccount = neuron.Size; // Count of connection on this neuron
			this->weights[l][n] = new double[ccount];
			for (int c = 0; c < neuron.Size; c++) // Loop through each connection on this neuron
			{
				this->weights[l][n][c] = neuron.ConnectionAt(c).Weight;
			}
		}
	}
}

void Network::Adjust(double expected, double actual)
{
	double error = GetError(expected, actual); // Error on output layer

}

void Network::Save(string path)
{
	// TODO: Serialize NetworkTopology and save it
}

void Network::Load(string path)
{
	// TODO: Deserialize NetworkTopology and load it
}

void Network::Init(initializer_list<int>& initializerList)
{
	if (initializerList.size() < 3)
		throw "Initializer List can't contain less than 3 elements. E.g: { 2, 3, 4, 1 }: 2 Input, 3 Hidden, 4 Hidden, 1 Output";

	vector<int> inputVector; // clone initializer list to vector
	inputVector.insert(inputVector.end(), initializerList.begin(), initializerList.end());

	this->inputNeuronsCount = inputVector[0]; // First element in vector -> input
	this->outputNeuronsCount = inputVector.back(); // Last element in vector -> output
	this->hiddenLayersCount = inputVector.size() - 2; // Count of hidden layers = total items in vector minus end and start
	this->layers = new double*[hiddenLayersCount];
	this->hiddenNeuronsCount = new int[hiddenLayersCount]; // elements except first and last = hidden layers

	int hiddenIndex = 1; // index on input vector
	for (int i = 0; hiddenIndex <= hiddenLayersCount; i++) // Loop from [1] to [last-1] (all hidden layers)
	{
		int layerSize = inputVector[hiddenIndex]; // Layer size of this layer (Containing neurons)
		this->hiddenNeuronsCount[i] = layerSize; // Set neuron count on this hidden layer
		this->layers[i] = new double[layerSize];

		hiddenIndex++;
	}
}

void Network::DeleteWeights()
{
	for (int i = 0; i < this->hiddenLayersCount + 1; i++) // Loop through each layer
	{
		int lneurons = i; // Layer neurons
		if (lneurons == 0)
			lneurons = inputNeuronsCount; // First element; input neuron count
		else
			lneurons = this->hiddenNeuronsCount[i]; // Else; neuron count from size array

		for (int n = 0; n < lneurons; n++) // Loop through each neuron in this layer
		{
			delete[] this->weights[i][n]; // Delete all connections on this neuron
		}
		delete[] this->weights[i]; // Delete all neurons on this layer
	}
	delete[] this->weights; // Delete layer
}