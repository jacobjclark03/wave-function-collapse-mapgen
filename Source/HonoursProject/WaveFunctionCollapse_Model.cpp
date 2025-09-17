// Fill out your copyright notice in the Description page of Project Settings.

#include "WaveFunctionCollapse_Model.h"
#include <iostream>
#include "Kismet/GameplayStatics.h"
#include "Containers/Array.h"



// Sets default values
AWaveFunctionCollapse_Model::AWaveFunctionCollapse_Model():
	_MX(0), _MY(0), _N(0), _hasNeighbour(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

/// 
///	Run Wave Function Collapse Algorithm
/// 
/// seed - seed for random number generator
/// limit - limit for number of iterations
bool AWaveFunctionCollapse_Model::Run(int seed, int limit)
{
// https://github.com/mxgmn/WaveFunctionCollapse
	Clear();											// Ensure data structures and variables are reset before running WFC 

	std::random_device _rd;								// Seed random number generator
	std::mt19937 _random(_rd()); 						// Create random number generator

	if (_hasNeighbour) BanBasedOnNeighbour();			// If current chunk has any neighbouring chunks, pre-ban based on neighbour edges

	for (int l = 0; l < limit || limit < 0; l++)		// Main loop runs until limit is reached or grid is fully observed
	{
		_node = NextUnobservedNode(_random);			// Find next unobserved node based on heuristic

		if (_node >= 0)									// If selected node is a valid index 
		{
			Observe(_node, _random);					// Observe selected node and selected state 
	
			if (!Propagate())							// Propagate constraints based on previous tile selection
				return false;							// If propagation fails return false as there is no successful output
		}
		else
		{	
			for (int i = 0; i < _wave.size(); i++)		// All nodes observed, store observed state, return true
			{
				for (int t = 0; t < _T; t++)
				{
					if (_wave[i][t])					// Check if current node has t-th state set to true
					{
						_observed[i] = t;				// Update the observed state to the chosen tile state
						break;							// Break from loop
					}
				}
			}
			return true;								// Return true = solution found
		}	
	}

	return false;										// Return false if limit has been exceeded, no solution has been found
}

/// 
/// Set current neighbour chunk for propagation
///
///	neighbour - chunk edges to be used for propagation
void AWaveFunctionCollapse_Model::SetCurrentNeighbourChunks(NeighbourChunk neighbour)
{
	if (!_hasNeighbour) _hasNeighbour = true;	// Set flag to true if neighbour chunk is set
	_neighbourChunk = neighbour;				// Set neighbour chunk
	BanBasedOnNeighbour();					// Ban tiles based on neighbour chunk
}

///
/// Based tile and cell combinations based on the neighbouring edge from generated chunks
/// 
void AWaveFunctionCollapse_Model::BanBasedOnNeighbour()
{
	int _counter = 0;							// Create counter to track current tile in neighbour edge
	if (!_neighbourChunk._downEdge.empty())		// If there is a neighbour below current chunk
	{
		_counter = 0;														// Set counter to zero
		for (int i = 0; i < _MX; i++)										// Loop through bottom row
		{
			CheckNeighbourEdge(i, _counter, 1, _neighbourChunk._downEdge);	// Check validatity of tile states banning based on downwards neighbour
			_counter++;														// Increment counter
		}

	}
	if (!_neighbourChunk._rightEdge.empty())	// If there is a neighbour to the right of current chunk
	{
		_counter = 0;														// Set counter to zero
		for (int i = 0; i < _MX * _MY; i += _MX)								// Loop through first column
		{
			CheckNeighbourEdge(i, _counter, 0, _neighbourChunk._rightEdge);	// Check validatity of tile states banning based on right sided neighbour
			_counter++;														// Increment counter
		}
	}

	// Ban states made incompatible by neighbours 
	if (!_toBan.empty())									// If vector contains states to be banned
		for (auto bannedState : _toBan)						// Loop through states to ban
			Ban(bannedState.first, bannedState.second);		// Ban states

	_toBan.clear();											// Clear to ban vector to avoid endless banning loop if propagation fails
}

/// 
/// Check validity of tile states based on neighbouring edge
/// 
/// i - current tile index
/// counter - current tile in neighbour edge
/// direction - direction of neighbour edge
/// neighbourEdge - neighbour edge to check
void AWaveFunctionCollapse_Model::CheckNeighbourEdge(int i, int counter, int direction, const std::vector<int>& neighbourEdge)
{
	int _tile = neighbourEdge[counter];				// Retrieve current tile from neighbour edge
	for (int t = 0; t < _T; t++)					// Loop through tile types
	{
		bool _validState = false;					// Current validatity of state is false

		for (int allowed : _propagator[direction][_tile])	// Loop through propagator in current direction for current tile
		{
			if (allowed == t)								// If tile type 't' is a valid placement
			{
				_validState = true;							// Set valid state to true
				break;										// Break from loop to avoid unneccessary checks
			}
		}	
		if (!_validState)									// If 't' is not a valid state
		{
			if (!(std::find(_toBan.begin(), _toBan.end(), std::pair<int, int>(i, t)) != _toBan.end()))	// Search through to ban vector to check if 't' is already to be banned at tile 'i'
			{
				_toBan.push_back(std::pair<int, int>(i, t));											// If state is not already banned at tile 'i' push state to be banned
			}
		}
	}
}

/// 
/// Define propagator rules
///
void AWaveFunctionCollapse_Model::TempPropagatorSetup()
{
	// Defintion of propagator - Defines what tiles can be placed next to each other in the left, up, right & down directions

	// Resize propagator for each direction and T (num tiles) in each direction
	_propagator.resize(4, std::vector<std::vector<int>>(_T));

	// Floor
	_propagator[0][Floor] = { Floor, EastWall, BLOutsideCorner, TLOutsideCorner};		// LEFT
	_propagator[1][Floor] = { Floor, SouthWall, TLOutsideCorner, TROutsideCorner };		// UP
	_propagator[2][Floor] = { Floor, WestWall, BROutsideCorner, TROutsideCorner};		// RIGHT
	_propagator[3][Floor] = { Floor, NorthWall, BLOutsideCorner, BROutsideCorner  };		// DOWN

	// East Wall
	_propagator[0][EastWall] = { Blank, WestWall, BRInsideCorner, TRInsideCorner };
	_propagator[1][EastWall] = { EastWall, TLInsideCorner, BLOutsideCorner };
	_propagator[2][EastWall] = { Floor , WestWall, BROutsideCorner, TROutsideCorner };
	_propagator[3][EastWall] = { EastWall, BLInsideCorner, TLOutsideCorner };

	// North Wall
	_propagator[0][NorthWall] = { NorthWall, BLInsideCorner, BROutsideCorner };
	_propagator[1][NorthWall] = { SouthWall, Floor, TLOutsideCorner, TROutsideCorner };
	_propagator[2][NorthWall] = { NorthWall, BRInsideCorner, BLOutsideCorner };
	_propagator[3][NorthWall] = { Blank, SouthWall, TLInsideCorner, TRInsideCorner };

	// West Wall 
	_propagator[0][WestWall] = { Floor, EastWall, TLOutsideCorner, BLOutsideCorner };
	_propagator[1][WestWall] = { WestWall, TRInsideCorner, BROutsideCorner };
	_propagator[2][WestWall] = { Blank, EastWall, BLInsideCorner, TLInsideCorner };
	_propagator[3][WestWall] = { WestWall, BRInsideCorner, TROutsideCorner };

	// South Wall
	_propagator[0][SouthWall] = { SouthWall, TLInsideCorner, TROutsideCorner };
	_propagator[1][SouthWall] = { Blank, NorthWall, BLInsideCorner, BRInsideCorner };
	_propagator[2][SouthWall] = { SouthWall, TRInsideCorner, TLOutsideCorner };
	_propagator[3][SouthWall] = { Floor, NorthWall, BLOutsideCorner, BROutsideCorner };

	// Top Left Inside Corner
	_propagator[0][TLInsideCorner] = { Blank, WestWall, BRInsideCorner, TRInsideCorner };
	_propagator[1][TLInsideCorner] = { Blank, NorthWall, BLInsideCorner, BRInsideCorner };
	_propagator[2][TLInsideCorner] = { SouthWall, TLOutsideCorner };
	_propagator[3][TLInsideCorner] = { EastWall, TLOutsideCorner };

	// Top Right Inside Corner
	_propagator[0][TRInsideCorner] = { SouthWall, TROutsideCorner };
	_propagator[1][TRInsideCorner] = { Blank, NorthWall, BRInsideCorner, BLInsideCorner };
	_propagator[2][TRInsideCorner] = { Blank, EastWall, BLInsideCorner, TLInsideCorner };
	_propagator[3][TRInsideCorner] = { WestWall, TROutsideCorner };

	// Bottom Left Inside Corner
	_propagator[0][BLInsideCorner] = { Blank, WestWall, BRInsideCorner, TRInsideCorner };
	_propagator[1][BLInsideCorner] = { EastWall, BLOutsideCorner };
	_propagator[2][BLInsideCorner] = { NorthWall, BLOutsideCorner };
	_propagator[3][BLInsideCorner] = { Blank, SouthWall, TLInsideCorner, TRInsideCorner };

	// Bottom Right Inside Corner
	_propagator[0][BRInsideCorner] = { NorthWall, BROutsideCorner };
	_propagator[1][BRInsideCorner] = { WestWall, BROutsideCorner };
	_propagator[2][BRInsideCorner] = { Blank, EastWall, BLInsideCorner, TLInsideCorner, };
	_propagator[3][BRInsideCorner] = { Blank, SouthWall, TRInsideCorner, TLInsideCorner };


	// Top Left Outside Corner
	_propagator[0][TLOutsideCorner] = { SouthWall, TLInsideCorner, TROutsideCorner };
	_propagator[1][TLOutsideCorner] = { EastWall, TLInsideCorner, BLOutsideCorner };
	_propagator[2][TLOutsideCorner] = { Floor, WestWall, BROutsideCorner, TROutsideCorner };
	_propagator[3][TLOutsideCorner] = { Floor, NorthWall, BLOutsideCorner, BROutsideCorner };

	// Top Right Outside Corner
	_propagator[0][TROutsideCorner] = { Floor, EastWall, BLOutsideCorner, TLOutsideCorner };
	_propagator[1][TROutsideCorner] = { WestWall, TRInsideCorner, BROutsideCorner };
	_propagator[2][TROutsideCorner] = { SouthWall, TRInsideCorner, TLOutsideCorner };
	_propagator[3][TROutsideCorner] = { Floor, NorthWall, BLOutsideCorner, BROutsideCorner };

	// Bottom Left Outside Corner
	_propagator[0][BLOutsideCorner] = { NorthWall, BLInsideCorner, BROutsideCorner };
	_propagator[1][BLOutsideCorner] = { Floor, SouthWall, TLOutsideCorner, TROutsideCorner};
	_propagator[2][BLOutsideCorner] = { Floor, WestWall, BROutsideCorner, TROutsideCorner };
	_propagator[3][BLOutsideCorner] = { EastWall, BLInsideCorner, TLOutsideCorner };

	// Bottom Right Outside Corner
	_propagator[0][BROutsideCorner] = { Floor, EastWall, BLOutsideCorner, TLOutsideCorner };
	_propagator[1][BROutsideCorner] = { Floor, SouthWall, TLOutsideCorner, TROutsideCorner };
	_propagator[2][BROutsideCorner] = { NorthWall, BRInsideCorner, BLOutsideCorner };
	_propagator[3][BROutsideCorner] = { WestWall, BRInsideCorner, TROutsideCorner };



	// Blank 
	_propagator[0][Blank] = { Blank, WestWall, BRInsideCorner, TRInsideCorner };
	_propagator[1][Blank] = { Blank, NorthWall, BRInsideCorner, BLInsideCorner };
	_propagator[2][Blank] = { Blank, EastWall, TLInsideCorner, BLInsideCorner };
	_propagator[3][Blank] = { Blank, SouthWall, TRInsideCorner, TLInsideCorner };



	

}




///
/// Intialise all variables and data structures required to run Wave Function Collapse
/// 
/// "dimX" - Number of rows in grid
/// "dimY" - Number of columns in grid
void AWaveFunctionCollapse_Model::Init(int dimX, int dimY)
{
	_MX = dimX;				// Grid Dimension X
	_MY = dimY;				// Grid Dimension Y
	_N = 1;					// Number of immediate neigbours - not specifically useful for tiled model
	_periodic = true;		// Flag that determines if the output is meant to wrap around itself i.e. cell on top row will be deemed a neighbour of a cell on the bottom row
	_heuristic = Entropy;	// Method of cell observed within grid: Entropy - least uncertain cell is observed. Scanline - cells are observed sequentially

	_weights =
	{
		20.0f,  // 0 - Floor (slightly more common to encourage larger rooms)

		10.0f,   // 1 - East Wall
		10.0f,   // 2 - North Wall
		10.0f,   // 3 - West Wall
		10.0f,   // 4 - South Wall

		6.0f,   // 5 - TL Inside Corner
		6.0f,   // 6 - TR Inside Corner
		6.0f,   // 7 - BL Inside Corner
		6.0f,   // 8 - BR Inside Corner

		1.0f,   // 9 - TL Outside Corner
		1.0f,   //10 - TR Outside Corner
		1.0f,   //11 - BL Outside Corner
		1.0f,   //12 - BR Outside Corner

		8.0f,   //13 - Blank (less common, but not too rare—useful for room separation)
	};


	_T = _weights.size();				// Set 'T' to number of possible states for a cell to take

	_wave.resize(_MX * _MY);			// Resize wave vector to size of the grid
	_observed.resize(_MX * _MY);		// Resize observed vector to size of the grid
	_compatible.resize(_MX * _MY);		// Resize compatible vector to size of the grid
	 
	for (int i = 0; i < _wave.size(); i++)
	{
		_wave[i].resize(_T);														// Each cell has T possible states
		_compatible[i] = std::vector<std::vector<int>>(_T);							// Track compatibility for each state
		for (int t = 0; t < _T; t++) 
			_compatible[i][t] = std::vector<int>(4);								// Initalise compatibility for 4 directions
	}

	for (int i = 0; i < _wave.size(); i++)											// Loop through wave vector	
		for (int t = 0; t < _T; t++)												// Loop through tile states
			_wave[i][t] = true;														// Initialise the wave to allow all states
		
	_distribution.resize(_T);														// Resize distribution vector to size of T

	_weightLogWeights.resize(_T);													// Resize weightLogWeights vector to size of T
	_sumOfWeights = 0;																// Initialise sum of weights to zero
	_sumOfWeightLogWeights = 0;														// Initialise sum of weightLogWeights to zero

	for (int t = 0; t < _T; t++)													// Loop through tile states
	{
		_weightLogWeights[t] = _weights[t] * log(_weights[t]);						// Calculate weightLogWeights
		_sumOfWeights += _weights[t];												// Calculate sum of weights
		_sumOfWeightLogWeights += _weightLogWeights[t];								// Calculate sum of weightLogWeights
	}

	_startingEntropy = log(_sumOfWeights) - _sumOfWeightLogWeights / _sumOfWeights;	// Calculate initial entropy
	_entropies.resize(_MX * _MY);													// Resize entropies vector to size of grid

	_sumsOfOnes.resize(_MX * _MY);													// Resize sumsOfOnes vector to size of grid
	_sumsOfWeights.resize(_MX * _MY);												// Resize sumsOfWeights vector to size of grid
	_sumsOfWeightLogWeights.resize(_MX * _MY);										// Resize sumsOfWeightLogWeights vector to size of grid	

	_stack.resize(_wave.size() * _T);												// Resize stack vector to size of wave multiplied by the number of possible tile states
	_stackSize = 0;																	// Initialise size of stack to zero

	TempPropagatorSetup();															// Create propagation rules
}

///
/// Find the next unobserved node in grid
///
/// random - random number generator
int AWaveFunctionCollapse_Model::NextUnobservedNode(std::mt19937& random)
{
	if (_heuristic == Scanline)													// Scanline heuristic: go through the grid sequentially
	{
		for (int i = _observedSoFar; i < _wave.size(); i++)						// Loop through grid
		{
			if (!_periodic && (i % _MX + _N > _MX || i / _MX + _N > _MY)) 
				continue;														// Skip if out of bounds

			if (_sumsOfOnes[i] > 1)												// If the cell has more than one possible state
			{
				_observedSoFar = i + 1;											// Increment number of cells/nodes observed 
				return i;														// Return i-th node in grid
			}
		}
		return -1;																// No more valid nodes to observe, return -1 to end current WFC iteration
	}

	// Entropy based
	double _min = 1E+4;															// Set minimum entropy to high value
	int _argmin = -1;															// Set argmin to -1
	// Used to generate sequence of random numbers between 0 & 1 - adds random value to noise value
	std::uniform_real_distribution<double> _noiseDistribution(0.0, 1.0);														
	for (int i = 0; i < _wave.size(); i++)										// Loop through grid
	{
		if (!_periodic && (i % _MX + _N > _MX || i / _MX + _N > _MY))			
			continue;															// Skip if out of bounds

		double _entropy;														// Entropy of cell
		if (_heuristic == Entropy)
		{
			_entropy = _entropies[i];											// Set entropy to current cell
		}
		else
		{
			_entropy = _sumsOfOnes[i];											// Set entropy to number of possible states
		}

		if (_sumsOfOnes[i] > 1 && _entropy <= _min)								// If cell has more than one possible state and entropy is less than or equal to minimum
		{
			double _noise = 1E-6 * _noiseDistribution(random);					// Generate noise value
			if (_entropy + _noise < _min)										// If entropy + noise is less than minimum
			{
				_min = _entropy + _noise;										// Set minimum entropy to current entropy + noise
				_argmin = i;													// Cell with lowest entropy = i
			}
		}
	}
	return _argmin; // Return cell with lowest entropy
}

/// 
/// Observe chosen node in grid
///
/// node - node to observe
/// random - random number generator
void AWaveFunctionCollapse_Model::Observe(int node, std::mt19937& random)
{
    for (int t = 0; t < _T; t++)								// Populate distribution based on weights
		_distribution[t] = _wave[node][t] ? _weights[t] : 0.0f;	// If the state is possible, set the distribution to the weight of the state
    

	std::uniform_real_distribution<double> _dist(0.0, 1.0);		// Generate random number between 0 & 1
	int _r = Random(_distribution, _dist(random));				// Randomly select a state based on the distribution


	for (int t = 0; t < _T; t++)								// Loop through tile states
		if (_wave[node][t] != (t == _r))						// If the state is not equal to the chosen state
			Ban(node, t);										// Ban the state at the current node
        
  
	_observed[node] = _r;										// Set the observed state to the chosen random state
}

/// 
/// Process banned cell and tile type combinations and propagate constraints
///
bool AWaveFunctionCollapse_Model::Propagate()
{
	while (_stackSize > 0)											// Loop until no more nodes to process
	{
		std::pair<int, int> _topStack = _stack[_stackSize - 1];		// Get top element in the stack
		int _currentIndex	= _topStack.first;						// mode or index in grid
		int _currentTile	= _topStack.second;						// tile type
		_stackSize--;												// Decrement stack size

		// Co-ordinates of the cell
		_currentX = _currentIndex % _MX;							// Get x co-ord of current proccesing node
		_currentY = _currentIndex / _MX;							// Get y co-ord of current proccesing node

		for (int d = 0; d < 4; d++)									// Loop through directions
		{
			// Neighbouring tile coords
			int _neighbourX = _currentX + _dx[d];					// Get x co-ord of neighbouring node
			int _neighbourY = _currentY + _dy[d];					// Get y co-ord of neighbouring node

			if (!_periodic && (_neighbourX < 0 || _neighbourY < 0 || // If the neighbour tile index falls outside of the current grid 	
				_neighbourX + _N > _MX || _neighbourY + _N > _MY))
				continue;											// Skip the current iteration

			if (_neighbourX < 0)									// If the neighbour tile x co-ord is less than 0
				_neighbourX += _MX;									// Add the grid width to the x co-ord
			else if (_neighbourX >= _MX)							// If the neighbour tile x co-ord is greater than or equal to the grid width
				_neighbourX -= _MX;									// Subtract the grid width from the x co-ord

			if (_neighbourY < 0)									// If the neighbour tile y co-ord is less than 0
				_neighbourY += _MY;									// Add the grid height to the y co-ord
			else if (_neighbourY >= _MY)							// If the neighbour tile y co-ord is greater than or equal to the grid height
				_neighbourY -= _MY;									// Subtract the grid height from the y co-ord

			int _neighbourIndex = _neighbourX + _neighbourY * _MX;	// Calculate neighbour tile index

			// Update compatibility for each neighboring state
			for (int l = 0; l < _propagator[d][_currentTile].size(); l++)	// Loop through propagator in current direction for current tile
			{
				int _neighbourTile = _propagator[d][_currentTile][l];		// Get the neighbouring tile type
				UpdateCompatibility(_neighbourIndex, _neighbourTile, d);	// Update compatibility based on tile placement
			}
		}
	}
	return _sumsOfOnes[0] > 0;												// Return true if there are still possible states for the first cell
}

///
/// Update compatibility based on tile placement
///
/// i2 - index of the cell
/// t2 - tile type
/// d - direction
void AWaveFunctionCollapse_Model::UpdateCompatibility(int i2, int t2, int d)
{
	if (t2 == -1) return;										// If the tile type is -1, return

	_compatible[i2][t2][d]--;									// Decrement compatibilty count at the cell for the tile type in the given direction
	if (_compatible[i2][t2][d] == 0)							// Ban the state if it's no longer compatible
		Ban(i2, t2);											// Ban the state
}

/// 
/// Ban cell and tile combinations
/// 
/// i - index of tile to ban
/// t - tile type to ban at index of tile
void AWaveFunctionCollapse_Model::Ban(int i, int t)
{
	_wave[i][t] = false;										// Ban the state by marking it as false

	for (int d = 0; d < 4; d++)									// Loop for each direction
		_compatible[i][t][d] = 0;								// Set all compatibility to zero
	

	_stack[_stackSize] = std::pair<int,int>(i, t);				// Add the banned state to the stack
	_stackSize++;												// Increment size of stack

	_sumsOfOnes[i] -= 1;										// Update the sum of remaining possible states
	_sumsOfWeights[i] -= _weights[t];							// Update the sum of weights
	_sumsOfWeightLogWeights[i] -= _weightLogWeights[t];			// Update the sum of weightLogWeights

	if (_sumsOfWeights[i] > 0)																	// If the sum of weights is greater than 0
		_entropies[i] = log(_sumsOfWeights[i]) - _sumsOfWeightLogWeights[i] / _sumsOfWeights[i];// Recompute the entropy
	else
		_entropies[i] = 0;																		// Set entropy to 0			
}

/// 
/// Reset all data structures and variables to initial state 
/// 
void AWaveFunctionCollapse_Model::Clear()
{
	for (int i = 0; i < _wave.size(); i++)									// Loop through wave
	{
		for (int t = 0; t < _T; t++)										// Loop through tile types
		{
			_wave[i][t] = true;												// Reset the wave to allow all states
			for (int d = 0; d < 4; d++)										// Loop for each direction
				_compatible[i][t][d] = _propagator[_opposite[d]][t].size();	// Reset compatibility
		}

		_sumsOfOnes[i] = _weights.size();									// Set the sum of possible states to the number of tile types
		_sumsOfWeights[i] = _sumOfWeights;									// Set the sum of weights to the initial sum of weights
		_sumsOfWeightLogWeights[i] = _sumOfWeightLogWeights;				// Set the sum of weightLogWeights to the initial sum of weightLogWeights
		_entropies[i] = _startingEntropy;									// Set the entropy to the initial entropy
		_observed[i] = -1;													// Mark the cell as unobserved
	}
	_observedSoFar = 0;														// Reset the observation counter
	_stackSize = 0;															// Reset the stack size	
}



// Called when the game starts or when spawned
void AWaveFunctionCollapse_Model::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWaveFunctionCollapse_Model::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

