// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include <random>
#include "WFC_Grid.h"
#include "HelperClass.h"
#include "WaveFunctionCollapse_Model.generated.h"



UCLASS()
class HONOURSPROJECT_API AWaveFunctionCollapse_Model : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaveFunctionCollapse_Model();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	enum Heuristic					// Heuristic - Determines in which way the algorithm will select tiles to be observed
	{
		Entropy, MRV, Scanline
	};

	Heuristic _heuristic;								// Heuristic used to select tiles to be observed
	NeighbourChunk _neighbourChunk;						// Neighbour chunk for propagation
	HelperClass* _helperClass = new HelperClass();		// Helper class for utility functions
public:
	bool Run(int seed, int limit);																		// Run Wave Function Collapse algorithm
	void SetCurrentNeighbourChunks(NeighbourChunk neighbour);											// Set current neighbour chunk
	void BanBasedOnNeighbour();																			// Ban tiles based on neighbour chunk
	void CheckNeighbourEdge(int i, int counter, int direction, const std::vector<int>& neighbourEdge);	// Check neighbour edge
	void Init(int dimX, int dimY);																		// Initialise all Wave Function Collapse variables
	void Clear();																						// Clear algorithms variables 

	void SetTileWeights(const TArray<float>& inputWeight)	// Set tile weights
	{
		for (int i = 0; i < _weights.size(); i++)			// Loop through weights
		{
			_weights[i] = inputWeight[i];					// Set weights
		}
	};
	
private:	
	int		NextUnobservedNode(std::mt19937& random);	// Find the next unobserved node in grid
	bool	Propagate();								// Propagate constraints based on previous tile placement
	void	Observe(int node, std::mt19937& random);	// Observe chosen node
	void	UpdateCompatibility(int i2, int t2, int d);	// Update compatibility based on tile placement
	void	Ban(int i, int t);							// Ban any uncompatible tile & mesh combinations added to the stack
	void	TempPropagatorSetup();						// Define propagator/adjacency rules for each tile type

	static int Random(std::vector<double>& weights, double r)	// Random selection based on weights
	{
		double _sum = 0;												// Sum of weights
		for (int i = 0; i < weights.size(); i++) _sum += weights[i];	// Loop through weights and sum
		double _threshold = r * _sum;									// Threshold for selection

		double _partialSum = 0;						// Partial sum of weights
		for (int i = 0; i < weights.size(); i++)	// Loop through weights
		{
			_partialSum += weights[i];				// Add weight to partial sum
			if (_partialSum >= _threshold) 
				return i;							// If partial sum is greater than threshold, return index
		}
		return 0;									// Return 0 if no selection is made
	}
private:
	// Wave vector - possible states of grid - for each cell, list of booleans detailing which tile placement is possible
	std::vector<std::vector<bool>>				_wave;

	// Propogation rules (Adjacency Rules)
	std::vector<std::vector<std::vector<int>>> _propagator;
	// Compatible states for each cell
	std::vector<std::vector<std::vector<int>>> _compatible;
	// States observation for each cell
	std::vector<int>							_observed;

	// Backtracking stack used during propagation - in C# - (int, int)[] - '(int, int)' is a tuple data type - '[]' array, using vectors for C++
	std::vector<std::pair<int, int>>	_stack;
	// Stack management 
	int									_stackSize, _observedSoFar;


	// Model dimensions, 'T' - number of possible states
	int									_MX, _MY, _T, _N;
	// Flags for periodic boundaries and ground constraints
	bool								_periodic, _ground;

	// Weights for state probabilities
	std::vector<double>					_weights;
	// Vectors for weighted probabilities and state distribution
	std::vector<double>					_weightLogWeights, _distribution;

	// Sums and entropies during propagation
	double								_sumOfWeights, _sumOfWeightLogWeights, _startingEntropy;
	std::vector<int>					_sumsOfOnes;
	std::vector<double>					_sumsOfWeights, _sumsOfWeightLogWeights, _entropies;

	// Arrays representing the direction offsets for neighbors
	std::vector<int>					_dx = { 1, 0, -1, 0 };		// Directional offsets for neighbours
	std::vector<int>					_dy = { 0, 1, 0, -1 };		// Directional offsets for neighbours
	std::vector<int>					_opposite = { 2, 3, 0, 1 };	// Opposite directions for compatibility checking

	// Propagation
	int									_currentX, _currentY;
	int									_node;
	bool								_hasNeighbour;

	std::vector<std::pair<int, int>>	_toBan;

public:
	// Getters and setters
	std::vector<int>& GetObserved()		{ return _observed; };	// Get observed states
};
