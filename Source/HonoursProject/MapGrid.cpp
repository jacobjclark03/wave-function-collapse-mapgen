// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGrid.h"
#include <sstream>
#include "HelperClass.h"
// Sets default values
AMapGrid::AMapGrid() :
	GRID_SIZE(0), ROW_SIZE(0), COLUMN_SIZE(0), CHUNK_ROW_SIZE(0), CHUNK_COLUMN_SIZE(0), _currentLocation({0.0f,0.0f,0.0f})
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMapGrid::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMapGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Initialise map grid
void AMapGrid::InitGrid(int rows, int columns, int chunkRows, int chunkColumns)
{
	ROW_SIZE = rows;					// Set num of rows in map grid
	COLUMN_SIZE = columns;				// Set num of columns in map grid
	GRID_SIZE = ROW_SIZE * COLUMN_SIZE;	// Calculate grid size

	CHUNK_ROW_SIZE = chunkRows;			// Set num of rows for each grid in map grid cell
	CHUNK_COLUMN_SIZE = chunkColumns;	// Set num of columnns for each grid in map grid cell

	_chunkGridArray.SetNum(GRID_SIZE);	// Set size of grid array

	CreateMapGrid();					// Create grid
}


/// 
/// Create grid of grids
///
void AMapGrid::CreateMapGrid()
{
	for (int i = 0; i < GRID_SIZE; i++)			// Loop through size of grid
	{
		if (GetWorld())
		{
			AWFC_Grid* _newGrid = GetWorld()->SpawnActor<AWFC_Grid>(AWFC_Grid::StaticClass(), FTransform(FRotator(0.0f, 0.0f, 0.0f), _currentLocation));	// Spawn new grid
			if (_newGrid)																// If new grid has been created
			{
				_newGrid->InitGrid(CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE, _currentLocation);	// Initialise grid
				_chunkGridArray[i] = _newGrid;											// Add new grid to array
			}
		}

		_currentLocation.X += (CHUNK_ROW_SIZE * 400.f);			// Increase position for next grid

		if ((i + 1) % ROW_SIZE == 0)							// If current position is end of the row
		{
			_currentLocation.X = 0.0f;							// Reset x position to 0
			_currentLocation.Y += (CHUNK_ROW_SIZE * 400.f);		// Move position up a row
		}
	}
}

/// 
/// Apply selected tile to meshes
///
/// index - chunk index
/// observedArr - vector containing the values of the placed tiles
void AMapGrid::GenerateChunks(int index, const std::vector<int>& observedArr)
{
	for (int i = 0; i < observedArr.size(); i++)				// Loop through observed vector
	{
		_chunkGridArray[index]->SetTileMesh(i, observedArr[i]);	// Set tile mesh 
	}
	_chunkGridArray[index]->SetFinalTileStates(observedArr);	// Set final states within grid
}

/// 
/// Collect the neighbour edges for a chunk
/// 
/// current chunk - index of current chunk
NeighbourChunk AMapGrid::GetChunkEdges(int currentChunk)
{
	NeighbourChunk _neighbourEdges;				// Create neighbour edges
	
	int _currentX = currentChunk % ROW_SIZE;	// Calculate x co-ord of current chunk
	int _currentY = currentChunk / ROW_SIZE;	// Calculate y co-ord of current chunk

	for (int d = 0; d < 4; d++)					// Loop through all 4 directions
	{
		int _neighbourX = _currentX + _dx[d];	// Calculate x co-ord of neighbour chunk
		int _neighbourY = _currentY + _dy[d];	// Calculate y co-ord of neighbour chunk
		int _neighbourIndex = _neighbourX + _neighbourY * ROW_SIZE; // Neighbour tile index

		if (_neighbourX < 0 || _neighbourX >= ROW_SIZE || _neighbourY < 0 || _neighbourY >= COLUMN_SIZE) // If neighbour co-ords fall outside grid
			continue;																					 // continue to next direction
		
		if (_neighbourIndex < 0 || _neighbourIndex >= _chunkGridArray.Num()) // If index falls outside of grid
			continue;														 // Continue to next direction
		

		switch (d)																				// Switch on direction
		{
		case 0: // Left
			_neighbourEdges._leftEdge = _chunkGridArray[_neighbourIndex]->GetLeftRowGrid();		// Assign left edge
			break;
		case 1: // Up
			_neighbourEdges._upEdge = _chunkGridArray[_neighbourIndex]->GetTopRowGrid();		// Assign top edge
			break;
		case 2: // Right
			_neighbourEdges._rightEdge = _chunkGridArray[_neighbourIndex]->GetRightRowGrid();	// Assign right edge
			break;
		case 3: // Down
			_neighbourEdges._downEdge = _chunkGridArray[_neighbourIndex]->GetBottomRowGrid();	// Assign bottom edge
			break;
		default:
			break;
		}
	}
	return _neighbourEdges;																		// Return neighbour edges
}

/// 
/// Discard of chunks previously generated
/// 
void AMapGrid::DiscardChunks()
{
	for (auto chunk : _chunkGridArray)			// Loop through chunk grid array
	{
		chunk->DiscardMeshes();					// Discard meshes
		std::vector<int> _emptyVec;				// Create empty vector
		chunk->SetFinalTileStates(_emptyVec);	// Set final tile states to empty vector
	}
}



