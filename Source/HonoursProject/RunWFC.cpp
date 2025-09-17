// Fill out your copyright notice in the Description page of Project Settings.


#include "RunWFC.h"
#include "Kismet/GameplayStatics.h"
#include <map>
#include <unordered_map>
#include "Containers/Array.h"

ARunWFC::ARunWFC()
	: _gridPos(0),_mapGrid(NULL), _finalGrid(nullptr), ROW_SIZE(10), COLUMN_SIZE(10), CHUNK_ROW_SIZE(5), CHUNK_COLUMN_SIZE(5),
	_demoMode(true), _success(false), _attempts(0)
{
    PrimaryActorTick.bCanEverTick = false;

	WaveFunctionCollapseModel = CreateDefaultSubobject<AWaveFunctionCollapse_Model>(TEXT("WaveFunctionCollapseModel"));

	_gridCells.resize((ROW_SIZE * COLUMN_SIZE) * (CHUNK_ROW_SIZE * CHUNK_COLUMN_SIZE), -1);				// Resize vector that hold store positions of all rooms to size of main grid
}

// Called when the game starts or when spawned
void ARunWFC::BeginPlay()
{
	Super::BeginPlay();

	WaveFunctionCollapseModel->Init(CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE);									// Initialise Wave Function Collapse model with dimensions of each chunk within larger grid
	
	if (GetWorld())																						// If world exists
	{
		TArray<AActor*> FoundGridActors;																// Initialise found actors array
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapGrid::StaticClass(), FoundGridActors);	// Find all instances of AMapGrid class

		if (FoundGridActors.Num() > 0)																	// If grid actors have been found
		{
			_mapGrid = Cast<AMapGrid>(FoundGridActors[0]);												// Cast found grid to local grid variable

			if (_mapGrid)																				// If the grid has been initialised correctly
				_mapGrid->InitGrid(ROW_SIZE, COLUMN_SIZE, CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE);			// Initialise map grid with dimensions and each chunk's dimensions
		}
	}
}

/// 
/// Take in combined grids observed vector and define rooms from tile values contained in array
/// 
/// "observed" - vector containing the combined observed vector<int> from each chunk grid
/// "x" - x co-ordinate of current cell
/// "y" - y co-ordinate of current cell
/// "roomID" - current ID of room that is being defined
/// "room" - instance of room struct for current room being defined 
void ARunWFC::RoomDefinition(std::vector<int>& observed, int x, int y, int roomID, ARoom& room)
{
	int index = y * (ROW_SIZE * CHUNK_ROW_SIZE) + x;							// Calculate index

	if (x < 0 || x >= ROW_SIZE * CHUNK_ROW_SIZE ||								// If x co-ordinate lies outside grid
		y < 0 || y >= COLUMN_SIZE * CHUNK_COLUMN_SIZE)							// Or y co-ordinate lies outside grid
		return;																	// Return from function has co-ordinates are not valid

	if (_gridCells[index] != -1)												// If cell has already been assigned / observed
		return;																	// Return from function has cell has already been assigned / observed 

	if (observed[index] == 0 || (observed[index] > 8 && observed[index] < 13))							// If value in combined observed array contains a floor or any strut tile
	{
		_gridCells[index] = roomID;																		// Assign current room ID to tile
		room.PushTileIndex(index);																		// Push back observed tile to current room's tile vector

		int _neighbourIndex = -1;

		for (int d = 0; d < 4; d++)																		// Loop for every 4 directions
		{
			int _neighbourX = x + _dx[d];																// Calculate neighbour x co-ordinate
			int _neighbourY = y + _dy[d];																// Calculate neighbour y co-ordinate
			_neighbourIndex = _neighbourY * (ROW_SIZE * CHUNK_ROW_SIZE) + _neighbourX;					// Calculate neighbour index

			if (_gridCells[_neighbourIndex] != -1)														// If cell has already been assigned / observed
				continue;

			if (_neighbourX < 0 || _neighbourX > ROW_SIZE * CHUNK_ROW_SIZE ||							// If neighbour cell x co-ordinate lies outside grid
				_neighbourY < 0 || _neighbourY > COLUMN_SIZE * CHUNK_COLUMN_SIZE)						// Or neighbour cell y co-ordinate lies outside grid
				continue;

			if (observed[_neighbourIndex] > 0 && observed[_neighbourIndex] < 9)							// If neighbour cell is a valid room tile
			{
				if (d == 0)																				// If direction is left
				{
					room.SetRowCount(room.GetRowCount() + 1);											// Increment rooms row count
				}
				else if (d == 1)																		// If direction is up
				{
					room.SetColumnCount(room.GetColumnCount() + 1);										// Increment column count
				}
			}
		}
	
		RoomDefinition(observed, x + 1, y, roomID, room);	// Check next cell in positive x direction
		RoomDefinition(observed, x - 1, y, roomID, room);	// Check next cell in negative x direction
		RoomDefinition(observed, x, y + 1, roomID, room);	// Check next cell in positive y direction
		RoomDefinition(observed, x, y - 1, roomID, room);	// Check next cell in negative y direction
	}
}

/// 
/// Combine all generated grids into one larger vector for the final output of WFC to be used for room definition and A* pathfinding
/// 
void ARunWFC::CombineGrids()
{
	_combinedGrids.resize((ROW_SIZE * COLUMN_SIZE) * (CHUNK_ROW_SIZE * CHUNK_COLUMN_SIZE)); // Resize Combined grid vector to size of all tiles
		
	for (int gC = 0; gC < COLUMN_SIZE; gC++)												// Loop through large grid's columns
	{
		for (int gR = 0; gR < ROW_SIZE; gR++)												// Loop through large grid's rows
		{
			int _chunkIndex = gC * ROW_SIZE + gR;											// Calculate index of the chunk currently being observed

			if (_chunkIndex >= _chunkGrids.size())											// If index falls outside of the size of the grid array
				continue;																	// Continue 
		
			for (int y = 0; y < CHUNK_COLUMN_SIZE; y++)										// Loop through columns in generated chunks
			{
				for (int x = 0; x < CHUNK_ROW_SIZE; x++)									// Loop through rows in generated chunks
				{
					int _newX = gR * CHUNK_ROW_SIZE + x;									// Calculate new x co-ordinate for tile
					int _newY = gC * CHUNK_COLUMN_SIZE + y;									// Calculate new y co-ordinate for tile

					int _newIndex = _newY * (ROW_SIZE * CHUNK_COLUMN_SIZE) + _newX;			// Calculate new index for tile

					if (_newIndex >= _combinedGrids.size())									// If new index falls outside the size of the combined grid array continue to next 
						continue;

					int _oldIndex = y * CHUNK_ROW_SIZE + x;									// Calculate the old index of the current tile

					if (_oldIndex >= _chunkGrids[_chunkIndex].size())						// If the old index falls outside of the current grid's size continue to next
						continue;

					_combinedGrids[_newIndex] = _chunkGrids[_chunkIndex][_oldIndex];		// Assign tile to new position in larger grid vector
				}
			}
		}
	}
	
}

/// 
/// Reconstruct room tiles to define the walls and pathways
///
/// observed - map vector containing tile type assignments
/// x - x co-ord of cell
/// y - y co-ord of cell
void ARunWFC::RoomRebuild(std::vector<int>& observed, int x, int y)
{
	int _index = y * (ROW_SIZE * CHUNK_ROW_SIZE) + x;							// Calculate index

	if (x < 0 || x >= ROW_SIZE * CHUNK_ROW_SIZE ||								// If x co-ordinate lies outside grid
		y < 0 || y >= COLUMN_SIZE * CHUNK_COLUMN_SIZE)							// Or y co-ordinate lies outside grid
		return;																	// Return from function has co-ordinates are not valid

	if (observed[_index] == Path || observed[_index] == Floor)					// If value in combined observed array contains a floor or any strut tile
	{
		bool _northOpen		= true,		_eastOpen		= true,		_southOpen		= true,		_westOpen		= true;									

		int _neighbourIndex = -1;																		// Initialise neighbour index

		for (int d = 0; d < 4; d++)																		// Loop for every 4 directions
		{
			int _neighbourX = x + _dx[d];																// Calculate neighbour x co-ordinate
			int _neighbourY = y + _dy[d];																// Calculate neighbour y co-ordinate
			_neighbourIndex = _neighbourY * (ROW_SIZE * CHUNK_ROW_SIZE) + _neighbourX;					// Calculate neighbour index

			if (_neighbourX < 0 || _neighbourX > ROW_SIZE * CHUNK_ROW_SIZE ||							// If neighbour cell x co-ordinate lies outside grid
				_neighbourY < 0 || _neighbourY > COLUMN_SIZE * CHUNK_COLUMN_SIZE)						// Or neighbour cell y co-ordinate lies outside grid
				continue;

			if (observed[_neighbourIndex] == -1 || observed[_neighbourIndex] == Blank)					// If neighbour cell is unassigned
			{
				if		(d == 0) _eastOpen	= false;													// If direction is east set east open to false
				else if (d == 1) _northOpen = false;													// If direction is north set north open to false
				else if (d == 2) _westOpen	= false;					 								// If direction is west set west open to false
				else if (d == 3) _southOpen = false;													// If direction is south set south open to false

				_pathfindGrid[_neighbourIndex] = -1;													// Assign -1 to pathfinding grid as it is unwalkable
			}
		
			// Boundary Checks 
			if (x == 0)										_westOpen = false;							// If x co-ordinate is 0 set west open to false
			if (x == (ROW_SIZE * CHUNK_ROW_SIZE) - 1)		_eastOpen = false;							// If x co-ordinate is at the end of the grid set east open to false
			if (y == 0)										_southOpen = false;							// If y co-ordinate is 0 set south open to false
			if (y == (COLUMN_SIZE * CHUNK_COLUMN_SIZE) - 1) _northOpen = false;							// If y co-ordinate is at the end of the grid set north open to false
		}

		// Assign tile type based on the neighbouring cells
		if ( _northOpen && !_southOpen &&  _eastOpen &&  _westOpen)		observed[_index] = NorthWall;
		if (!_northOpen &&  _southOpen &&  _eastOpen &&  _westOpen)		observed[_index] = SouthWall;
		if ( _northOpen &&  _southOpen && !_eastOpen &&  _westOpen)		observed[_index] = EastWall;
		if ( _northOpen &&  _southOpen &&  _eastOpen && !_westOpen)		observed[_index] = WestWall;
		//if (!_northOpen && !_southOpen &&  _eastOpen &&  _westOpen)		observed[_index] = HorizontalCorridor;
		//if ( _northOpen &&  _southOpen && !_eastOpen && !_westOpen)		observed[_index] = VerticalCorridor;
		if (!_northOpen &&  _southOpen && !_eastOpen &&  _westOpen)		observed[_index] = TLInsideCorner;
		if (!_northOpen &&  _southOpen &&  _eastOpen && !_westOpen)		observed[_index] = TRInsideCorner;
		if ( _northOpen && !_southOpen && !_eastOpen &&  _westOpen)		observed[_index] = BLInsideCorner;
		if ( _northOpen && !_southOpen &&  _eastOpen && !_westOpen)		observed[_index] = BRInsideCorner;
		//if (!_northOpen &&  _southOpen && !_eastOpen && !_westOpen)		observed[_index] = NorthDeadEnd;
		//if ( _northOpen && !_southOpen && !_eastOpen && !_westOpen)		observed[_index] = SouthDeadEnd;
		//if (!_northOpen && !_southOpen && !_eastOpen &&  _westOpen)		observed[_index] = EastDeadEnd;
		//if (!_northOpen && !_southOpen &&  _eastOpen && !_westOpen)		observed[_index] = WestDeadEnd;
		UE_LOG(LogTemp, Warning, TEXT("Tile at (%d, %d) is assigned value: %d"), x, y, observed[_index]);	// Log tile assignment
	}
}

///
///	Asssess whether or not rooms can be combined, if so combine them 
/// 
void ARunWFC::CombineRooms()
{
	// Room combinations
	float _ratioThreshold = 2.5f;				// Initialise ratio threshold
	float _densityThreshold = 0.45f;			// Initialise density threshold
	for (int i = 0; i < _rooms.size(); i++)		// Loop through rooms
	{
		float _ratio = std::max(_rooms[i]->GetRowCount(), _rooms[i]->GetColumnCount()) / std::min(_rooms[i]->GetRowCount(), _rooms[i]->GetColumnCount());	// Calculate ratio of room
		_rooms[i]->SetRoomDimensionRatio(_ratio);																											// Set room dimension ratio

		float _density = _rooms[i]->GetTileIndexes().size() / (_rooms[i]->GetRowCount() * _rooms[i]->GetColumnCount());										// Calculate room density
		_rooms[i]->SetRoomDensity(_density);																												// Set room density

		if ((_ratio > _ratioThreshold || _density < _densityThreshold) && _rooms[i]->GetTileIndexes().size() <= 20)											// If ratio or density is below threshold and room size is less than 20
			_rooms[i]->SetCombinable(true);																													// Set room to be combinable
		else                                                                                                                                                // else room is not combinable
			_rooms[i]->SetCombinable(false);																												// Set room to not be combinable
	}


	Graph _graph(_rooms.size(), ROW_SIZE, COLUMN_SIZE, CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE);			// Create instance of Graph
	ConvexHull _convexHull;																			// Create instance of Convex Hull							


	for (int i = 0; i < _rooms.size(); i++)															// Loop through rooms
	{
		int _closestIndex = -1;																		// Initialise closest index
		float _closestDist = FLT_MAX;																// Initialise closest distance
		if (_rooms[i]->GetCombinable())																// If room is combinable		
		{
			for (int j = 0; j < _rooms.size(); j++)													// Loop through rooms						
			{
				if (i == j)																			// If rooms are the same continue						
					continue;			

				Edge _edge = _graph.ShortestDistancePair(_rooms[i]->GetTileIndexes(), _rooms[j]->GetTileIndexes());											// Calculate shortest distance between rooms		

				FVector _start	= _finalGrid->GetTileArray()[_rooms[i]->GetTileIndexes()[_edge._tile0]]->GetActorLocation() + FVector(105, 105, 0.f);		// Calculate start position
				FVector _end	= _finalGrid->GetTileArray()[_rooms[j]->GetTileIndexes()[_edge._tile1]]->GetActorLocation() + FVector(105, 105, 0.f);		// Calculate end position

				if (FVector::Dist(_start, _end) < _closestDist)										// If distance between rooms is less than closest distance
				{
					_closestDist = FVector::Dist(_start, _end);										// Set closest distance to new distance	
					_closestIndex = j;																// Set closest index to index of closest room					
				}
			}

			if (_closestIndex > -1 && _closestDist <= (210.f * 5))									// If closest index is valid and distance is less than 5 tiles
			{
				_rooms[i]->GetTileIndexes().insert(_rooms[i]->GetTileIndexes().end(), _rooms[_closestIndex]->GetTileIndexes().begin(), _rooms[_closestIndex]->GetTileIndexes().end());	// Combine tile indexes of rooms

				std::set<std::pair<int, int>> _edges;					// Create set of edges - set for unique edges as duplicate edges were causing problems with convex hull
				for (int index : _rooms[i]->GetTileIndexes())			// Loop through current rooms tile indexes		
				{
					int _x = index % (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate x co-ordinate
					int _y = index / (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate y co-ordinate
					if (IsEdgeTile(_x, _y, _combinedGrids))			// If tile is an edge tile
					{
						_edges.insert({ _x, _y });					// Insert edge into edges set
					}
				}

				std::vector<std::pair<int, int>> edgePoints(_edges.begin(), _edges.end());					// Convert set to vector
				std::vector<std::pair<int, int>> ans = _convexHull.Divide(edgePoints);						// Calculate convex hull of edge points


				for (int y = 0; y < COLUMN_SIZE * CHUNK_COLUMN_SIZE; ++y)									// Loop through each column in grid
				{
					for (int x = 0; x < ROW_SIZE * CHUNK_ROW_SIZE; ++x)										// Loop through each row in grid
					{
						int _index = y * (ROW_SIZE * CHUNK_ROW_SIZE) + x;									// Calculate index of currently observed cell

						if (PointInPolygon(std::make_pair(x, y), ans))										// If (x, y) falls within the convex hull
						{
							_combinedGrids[_index] = Floor;													// Assign floor tile to cell	
							_rooms[i]->PushTileIndex(_index);												// Push back observed tile to current room's tile vector
						}
					}
				}
			}
			_rooms.erase(_rooms.begin() + _closestIndex);													// Erase closest room from rooms vector as it has been combined 
		}
	}

	
}


/// 
/// Create a pathway around the convex hull of the map
/// 
void ARunWFC::ConvexHullPathCreation()
{
	std::set<std::pair<int, int>> _edges;					// Create set of edges
	for (int i = 0; i < _rooms.size(); i++)					// Loop through rooms
	{
		for (int index : _rooms[i]->GetTileIndexes())		// Loop through tile indexes
		{
			int _x = index % (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate x co-ordinate
			int _y = index / (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate y co-ordinate
			if (IsEdgeTile(_x, _y, _combinedGrids))			// If tile is an edge tile
			{
				_edges.insert({ _x, _y });					// Insert edge into edges set
			}
		}
	}

	ConvexHull _convexHull;											
	std::vector<std::pair<int, int>> _roomEdges(_edges.begin(), _edges.end());				// Convert set to vector
	std::vector<std::pair<int, int>> _convexHullofMap = _convexHull.Divide(_roomEdges);		// Calculate convex hull of edge points of all rooms

	for (int k = 0; k < _convexHullofMap.size(); k++)										// Loop through convex hull of map			
	{
		const auto& _pointOne = _convexHullofMap[k];										// Get first point of convex hull
		const auto& _pointTwo = _convexHullofMap[(k + 1) % _convexHullofMap.size()];		// Get second point of convex hull

		FVector _start	= _finalGrid->GetTileArray()[_pointOne.second * (ROW_SIZE * CHUNK_ROW_SIZE) + _pointOne.first]->GetActorLocation();	// Get start point
		FVector _end	= _finalGrid->GetTileArray()[_pointTwo.second * (ROW_SIZE * CHUNK_ROW_SIZE) + _pointTwo.first]->GetActorLocation();	// Get end point

		if (_pointOne.second < (COLUMN_SIZE * CHUNK_COLUMN_SIZE) / 2)		// If first point is in the top half of the grid
			_start += FVector(0.f, 210.f, 100.f);							// Add offset to start point
		else
			_start -= FVector(0.f, 210.f, 100.f);							// Subtract offset from start point	

		if (_pointTwo.second < (COLUMN_SIZE * CHUNK_COLUMN_SIZE) / 2)		// If second point is in the top half of the grid
			_end += FVector(0.f, 210.f, 00.f);								// Add offset to end point
		else
			_end -= FVector(0.f, 210.f, 00.f);								// Subtract offset from end point

		if (_pointOne.first < (ROW_SIZE * CHUNK_ROW_SIZE) / 2)				// If first point is in the left half of the grid
			_start += FVector(210.f, 0.f, 00.f);							// Add offset to start point
		else
			_start -= FVector(210.f, 0.f, 00.f);							// Subtract offset from start point

		if (_pointTwo.first < (ROW_SIZE * CHUNK_ROW_SIZE) / 2)				// If second point is in the left half of the grid
			_end += FVector(210.f, 0.f, 00.f);								// Add offset to end point
		else
			_end -= FVector(210.f, 0.f, 00.f);								// Subtract offset from end point	

		FHitResult _hitResult;					// Create hit result
		FCollisionQueryParams _queryParams;		// Create query params

		TArray<FHitResult> _hitActors;			// Create hit actors array

		float _corridorSize = 100.0f;			// Set corridor size

		bool _bHit = GetWorld()->SweepMultiByObjectType(_hitActors, _start, _end, FQuat::Identity, 
			FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeCapsule(_corridorSize, 50.0f), _queryParams);		// Trace between start and end points for tile actors

		if (_bHit)																	// If trace returns a hit
		{	
			for (int i = 0; i < _hitActors.Num(); i++)								// Loop through the hit actors
			{
				AWFC_Tile* _tempTile = Cast<AWFC_Tile>(_hitActors[i].GetActor());	// Cast hit actor to tile

				int _tileIndex = _finalGrid->GetTileArray().Find(_tempTile);		// Get index of hit tile

				_combinedGrids[_tileIndex] = Path;									// Set tile to path
				_pathfindGrid[_tileIndex] = Path;									// Set tile to path in pathfinding grid
			}
		}
	}
}

/// 
/// Run WFC algorithm across each chunk. Once all chunks have been generated combine chunks outputs and define the rooms within the final output 
/// 
bool ARunWFC::RunAlgorithm()
{										
	if (!WaveFunctionCollapseModel || _gridPos > (ROW_SIZE * COLUMN_SIZE) - 1)							// If WFC model was not correctly initialised or all grids have been generated
		return true;																					// Return after combining grids and defining rooms or lack of WFC model
	
	_success = WaveFunctionCollapseModel->Run(80, 1800);												// Set success to the return output of running WFC model passing seed value and limits 

	if (_success)																						// If WFC model outputs a succesful solution
	{				
		if (_demoMode)
			_mapGrid->GenerateChunks(_gridPos, WaveFunctionCollapseModel->GetObserved());					// Assign meshes to cells in grid according to WFC output 

		int Width = 10 * 5;
		int Height = 10 * 5;

		for (int y = 0; y < Height; ++y)
		{
			FString LineOutput;
			for (int x = 0; x < Width; ++x)
			{
				int Index = y * Width + x;
				int ObservedValue = WaveFunctionCollapseModel->GetObserved()[Index];
				LineOutput += FString::Printf(TEXT("%3d"), ObservedValue);
			}
			UE_LOG(LogTemp, Warning, TEXT("%s"), *LineOutput);
		}

		_gridPos++;																						// Increment position in larger grid
		_success	= false;																			// Reset success value to false
		_attempts	= 0;																				// Reset attempts to 0
		_chunkGrids.push_back(WaveFunctionCollapseModel->GetObserved());								// Push observed vector containing final tile placements to chunk vector
		WaveFunctionCollapseModel->SetCurrentNeighbourChunks(_mapGrid->GetChunkEdges(_gridPos));		// Of next chunk set their neighbour tiles
		UE_LOG(LogTemp, Warning, TEXT("Attempts : %d"), _attempts);
	}

	if (_attempts > 50)																					// If current chunk has over 300 unsuccesful attempts
		ResetGrid();																					// Reset grid to start over

	_attempts++;																						// Increment attempts
	
	return false;
}

///
/// Clean up grids, create final grid for visual representation and define the rooms
/// 
void ARunWFC::Cleanup()
{
	for (auto grid : _mapGrid->GetGridArray())		// Loop through each grid in grid array
		grid->DiscardMeshes();						// Discard meshes for each grid

	CombineGrids();								// Combine all grids into one larger vector

	if (GetWorld() && !_finalGrid)					// If grid has not been created
	{
		_finalGrid = GetWorld()->SpawnActor<AWFC_Grid>(AWFC_Grid::StaticClass(), FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(00.0f, 0.0f, 0.0f)));		// Spawn grid actor

		if (_finalGrid)																																		// If grid has been spawned succesfully
		{
			_finalGrid->InitGrid(ROW_SIZE * CHUNK_ROW_SIZE, COLUMN_SIZE * CHUNK_COLUMN_SIZE, FVector(00.0f, 0.0f, 0.0f));									// Initialise grid with dimensions and location

			for (int i = 0; i < _combinedGrids.size(); i++)				// Loop through combined grids
			{
			

				_finalGrid->GetTileArray()[i]->EnableCollision();		// Enable Collisions for tile

				UE_LOG(LogTemp, Warning, TEXT("Tile at index %d has value: %d"), i, _combinedGrids[i]);
			}
			_finalGrid->SetFinalTileStates(_combinedGrids);				// Pass combined grids to final grid actor
		}
	}

	//CombineRooms();														// Combine rooms						

	if (_demoMode)														// If project is in demo mode
		for (int i = 0; i < _combinedGrids.size(); i++)					// Loop through combined grids
			_finalGrid->SetTileMesh(i, _combinedGrids[i]);				// Set tile meshes for visual representation

	for (int y = 0; y < COLUMN_SIZE * CHUNK_COLUMN_SIZE; ++y)									// Loop through each column in grid
	{
		for (int x = 0; x < ROW_SIZE * CHUNK_ROW_SIZE; ++x)										// Loop through each row in grid
		{
			int index = y * (ROW_SIZE * CHUNK_ROW_SIZE) + x;									// Calculate index of currently observed cell

			if (_combinedGrids[index] == -1)													// If tile has not been placed or the cell has already been observed
				continue;																		// Continue to next cell
			else
			{
				RoomRebuild(_combinedGrids, x, y);												// Rebuild room
			}
		}
	}

	for (int i = 0; i < _combinedGrids.size(); i++)											// Loop through combined grids
		_finalGrid->SetTileMesh(i, _combinedGrids[i]);										// Set tile meshes for visual representation

}

/// 
/// Reset grid to start proccess over again
/// 
void ARunWFC::ResetGrid()	
{
	_gridPos = 0;																			// Reset grid position
	_attempts = 0;																			// Reset attempts
	_gridCells.clear();																		// Clear grid cells vector
	_gridCells.resize((ROW_SIZE * COLUMN_SIZE) * (CHUNK_ROW_SIZE * CHUNK_COLUMN_SIZE), -1);	// Resize grid cells vector to size of all rooms
	_chunkGrids.clear();																	// Clear chunk grids vector
	_combinedGrids.clear();																	// Clear combined grids vector

	_mapGrid->DiscardChunks();																// Discard chunks
	WaveFunctionCollapseModel->SetCurrentNeighbourChunks(_mapGrid->GetChunkEdges(_gridPos));// Set current neighbour chunks
	_pathfindGrid.clear();																	// Clear pathfinding grid

	TArray<AActor*> _roomActors;															// Initialise room actors array
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoom::StaticClass(), _roomActors);	// Find all instances of ARoom class

	for (AActor* _room : _roomActors)														// Loop through room actors
		if (_room)																			// If room actor is valid
			_room->Destroy();																// Destroy room actor
	
	_rooms.clear();																			// Clear rooms vector

	if (_finalGrid)																			// If final grid exists
	{
		_finalGrid->GetStates().clear();													// Clear states
		_finalGrid->DiscardMeshes();														// Discard meshes	
		_finalGrid->DestroyTiles();															// Destroy tiles
		_finalGrid->Destroy();																// Destroy final grid
		_finalGrid = nullptr;																// Set final grid to null
	}


	for (auto grid : _mapGrid->GetGridArray())												// Loop through each grid in grid array
	{
		grid->DiscardMeshes();																// Discard meshes
		grid->GetStates().clear();															// Clear states
	}
}

/// 
///	Send user input tile weights to WFC model
/// 
/// inputWeights - vector containing weights for each tile type
void ARunWFC::SetTileWeights(const TArray<float>& inputWeight)
{
	WaveFunctionCollapseModel->SetTileWeights(inputWeight);		// Set tile weights in WFC model
}

///
/// Generate and verify pathways between rooms
/// 
void ARunWFC::RunPathFind()
{
	Graph _graph(_rooms.size(), ROW_SIZE, COLUMN_SIZE, CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE);		// Create instance of Graph
	Edge _edge(0, 0, 0);																		// Create instance of Edge

	for (int i = 0; i < _rooms.size(); i++)					// Loop through rooms
	{
		for (int j = i + 1; j < _rooms.size(); j++)			// Loop through rooms
		{
			_edge = _graph.ShortestDistancePair(_rooms[i]->GetTileIndexes(), _rooms[j]->GetTileIndexes());	// Get the shortest distance between rooms
			_graph.AddEdge(i, j, _edge._weight);															// Add edge to graph
		}
	}

	_graph.Kruskals();		// Run Kruskal's algorithm

	std::set<std::pair<int, int>> _extraEdges;						// Create set to store extra edges to increase room connectivity

	std::vector<std::vector<int>> _edges = _graph.GetEdgeList();	// Get edge list from graph
	std::sort(_edges.begin(), _edges.end());						// Sort edges

	int _extraEdgesCount = _rooms.size() / 2;						// Set extra edges count to half the number of rooms

	for (const auto& edge : _edges)									// Loop through edges
	{
		int _roomOne = edge[1];										// Get room one
		int _roomTwo = edge[2];										// Get room two

		if (_extraEdges.find({ _roomOne, _roomTwo }) == _extraEdges.end() && _extraEdgesCount > 0)			// If extra edges have not been added and extra edges count is greater than 0
		{
			_graph.GetOutput().push_back(edge);																// Add edge to graph output
			_extraEdges.insert({ _roomOne, _roomTwo });														// Insert edge into extra edges set
			_extraEdgesCount--;																				// Decrement extra edges count
		}
	}

	for (const auto& edge : _graph.GetOutput())					// Loop through graph output
	{
		int _roomOne = edge[1];									// Get room one
		int _roomTwo = edge[2];									// Get room two	

		_edge = _graph.ShortestDistancePair(_rooms[_roomOne]->GetTileIndexes(), _rooms[_roomTwo]->GetTileIndexes());		// Get shortest distance between rooms

		FVector _start = _finalGrid->GetTileArray()[_rooms[_roomOne]->GetTileIndexes()[_edge._tile0]]->GetActorLocation() + FVector(105, 105, 60.f);	// Set start point 
		FVector _end = _finalGrid->GetTileArray()[_rooms[_roomTwo]->GetTileIndexes()[_edge._tile1]]->GetActorLocation() + FVector(105, 105, 60.f);		// Set end point

		FHitResult _hitResult;					// Create hit result
		FCollisionQueryParams _queryParams;		// Create query params

		TArray<FHitResult> _hitActors;			// Create hit actors array
		float _corridorSize = 200.0f;

		bool _bHit = GetWorld()->SweepMultiByObjectType(_hitActors, _start, _end, FQuat::Identity,
			FCollisionObjectQueryParams(ECC_WorldDynamic), FCollisionShape::MakeCapsule(_corridorSize, 50.0f), _queryParams);		// Trace between start and end points

		if (_bHit)																													// If trace returns a hit
		{
			for (int i = 0; i < _hitActors.Num(); i++)																				// Loop through the hit actos
			{
				AWFC_Tile* _tempTile = Cast<AWFC_Tile>(_hitActors[i].GetActor());													// Cast hit actor to tile

				if (_tempTile != _finalGrid->GetTileArray()[_rooms[_roomOne]->GetTileIndexes()[_edge._tile0]]
					&& _tempTile != _finalGrid->GetTileArray()[_rooms[_roomTwo]->GetTileIndexes()[_edge._tile1]])					// If the hit tile is not the start or end tile
				{
					int _tileIndex = _finalGrid->GetTileArray().Find(_tempTile);													// Get index of hit tile

					_combinedGrids[_tileIndex] = Path;																				// Set tile to path
					_pathfindGrid[_tileIndex] = Path;																				// Set tile to path in pathfinding grid
				}
			}
		}
	}

	//ConvexHullPathCreation();	// Create convex hull path

	AStarPathfind _pathfinder;																		// Create instance of A* pathfinding										
	_pathfinder.Init(ROW_SIZE * CHUNK_ROW_SIZE, COLUMN_SIZE * CHUNK_COLUMN_SIZE, 210, 210, false);	// Initialise pathfinder
	_pathfindGrid = _combinedGrids;																	// Set pathfinding grid to combined grids

	for (const auto& edge : _graph.GetOutput())													// Loop through graph output
	{
		int _roomOne = edge[1];																	// Get room one
		int _roomTwo = edge[2];																	// Get room two

		int _roomOneIndex	= _rooms[_roomOne]->GetTileIndexes().size() / 2;					// Calculate index of room one
		int _roomOneCenter	= _rooms[_roomOne]->GetTileIndexes()[_roomOneIndex];				// Calculate center of room one

		int _roomTwoIndex	= _rooms[_roomTwo]->GetTileIndexes().size() / 2;					// Calculate index of room two
		int _roomTwoCenter	= _rooms[_roomTwo]->GetTileIndexes()[_roomTwoIndex];				// Calculate center of room two

		_rooms[_roomOne]->SetCentreIndex(_roomOneCenter);										// Set centre index of room one	
		_rooms[_roomOne]->SetCorridorCount(_rooms[_roomOne]->GetCorridorCount() + 1);			// Increment corridor count of room one

		_rooms[_roomTwo]->SetCentreIndex(_roomTwoCenter);										// Set centre index of room two
		_rooms[_roomTwo]->SetCorridorCount(_rooms[_roomTwo]->GetCorridorCount() + 1);			// Increment corridor count of room two		

		Node _start;																			// Create start instance of Node	
		_start._x = _roomOneCenter % (ROW_SIZE * CHUNK_ROW_SIZE);								// Set x co-ordinate of start node
		_start._y = _roomOneCenter / (ROW_SIZE * CHUNK_ROW_SIZE);								// Set y co-ordinate of start node
		_start._tileType = 0;																	// Set tile type of start node
		Node _end;																				// Create end instance of Node
		_end._x = _roomTwoCenter % (ROW_SIZE * CHUNK_ROW_SIZE);									// Set x co-ordinate of end node	
		_end._y = _roomTwoCenter / (ROW_SIZE * CHUNK_ROW_SIZE);									// Set y co-ordinate of end node
		_end._tileType = 0;

		std::vector<Node> _path = _pathfinder.AStar(_start, _end, _pathfindGrid);				// Calculate path between start and end nodes using A* algorithm
		
	}

	for (int i = 0; i < _combinedGrids.size(); i++)
	{
		if (_combinedGrids[i] >= Floor)
		{
			_combinedGrids[i] = Floor;
		}
	}

	for (int y = 0; y < COLUMN_SIZE * CHUNK_COLUMN_SIZE; ++y)									// Loop through each column in grid
	{
		for (int x = 0; x < ROW_SIZE * CHUNK_ROW_SIZE; ++x)										// Loop through each row in grid
		{
			int index = y * (ROW_SIZE * CHUNK_ROW_SIZE) + x;									// Calculate index of currently observed cell

			if (_combinedGrids[index] == -1)													// If tile has not been placed or the cell has already been observed
				continue;																		// Continue to next cell
			else
			{
				RoomRebuild(_combinedGrids, x, y);												// Rebuild room
			}
		}
	}
	if (_demoMode)																				// If project is in demo mode
		for (int i = 0; i < _combinedGrids.size(); i++)											// Loop through combined grids
			_finalGrid->SetTileMesh(i, _combinedGrids[i]);										// Set tile meshes for visual representation
	
	 

	//_finalGrid->SetFinalTileStates(_combinedGrids);											// Pass combined grids to final grid actor
}

/// 
/// Traverse final grid and build rooms
/// 
void ARunWFC::EvaluateRooms()
{
	CombineGrids();																	// Combine grids to create final grid

	int _roomCounter = 0;																		// Create counter for room definitions
	_rooms.clear();																				// Clear rooms vector
	_gridCells.resize((ROW_SIZE * COLUMN_SIZE) * (CHUNK_ROW_SIZE * CHUNK_COLUMN_SIZE), -1);		// Resize grid cells vector to size of map

	for (int y = 0; y < COLUMN_SIZE * CHUNK_COLUMN_SIZE; ++y)									// Loop through each column in grid
	{
		for (int x = 0; x < ROW_SIZE * CHUNK_ROW_SIZE; ++x)										// Loop through each row in grid
		{
			int index = y * (ROW_SIZE * CHUNK_ROW_SIZE) + x;									// Calculate index of currently observed cell

			if (_combinedGrids[index] == -1)													// If tile has not been placed or the cell has already been observed
				continue;																		// Continue to next cell
			else
			{
				ARoom* _room = GetWorld()->SpawnActor<ARoom>();									// Create temp room - this is very inefficient need to fix
				_room->SetRoomID(_roomCounter);													// Set roomID to current counter
				_room->SetRowCount(0);															// Set row count to 0
				_room->SetColumnCount(0);														// Set column count to 0
			
				RoomDefinition(_combinedGrids, x, y, _roomCounter, *_room);						// Define room - flood fill algorithm

				if (_room->GetRowCount() > 1 && _room->GetColumnCount() > 1 && _room->GetTileIndexes().size() >= 0)
				{
					_rooms.push_back(_room);													// Add new room to the vector
					_roomCounter++;																// Increment room counter 
				}
				else                                                                            // else room does not meet criteria
				{
					for (int i = 0; i < _room->GetTileIndexes().size(); i++)					// Loop through tile indexes of unused room
					{
						_gridCells[_room->GetTileIndexes()[i]] = -1;							// Set grid cell to -1
					}
					if (_room)
					{
						_room->Destroy();														// Destroy room if it has not been applied
						_room->MarkComponentsAsGarbage();										// Mark components as garbage
					}
				}
			
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Number of rooms defined: %d"), _rooms.size());	// Log number of rooms defined


	
	
	int Width = 10 * 5;
	int Height = 10 * 5;

	for (int y = 0; y < Height; ++y)
	{
		FString LineOutput;
		for (int x = 0; x < Width; ++x)
		{
			int Index = y * Width + x;
			int ObservedValue = _combinedGrids[Index];
			LineOutput += FString::Printf(TEXT("%3d"), ObservedValue);
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LineOutput);
	}

	_pathfindGrid = _combinedGrids;									// Set pathfinding grid to combined grids
}

/// 
/// Takes in co-ordinates of a cell and determines if it is on the edge of a room
/// 
/// x - x co-ordinate of cell
/// y - y co-ordinate of cell
/// observed - vector containing tile placemtns that make up the map
bool ARunWFC::IsEdgeTile(int x, int y, std::vector<int>& observed)
{
	for (int d = 0; d < 4; d++)															// Loop through each direction	
	{
		int _neighbourX = x + _dx[d];													// Calculate x co-ordinate of neighbour
		int _neighbourY = y + _dy[d];													// Calculate y co-ordinate of neighbour
		int _neighbourIndex = _neighbourY * (ROW_SIZE * CHUNK_ROW_SIZE) + _neighbourX;	// Calculate index of neighbour

		if (_neighbourX < 0 || _neighbourX >= ROW_SIZE * CHUNK_ROW_SIZE ||	
			_neighbourY < 0 || _neighbourY >= COLUMN_SIZE * CHUNK_COLUMN_SIZE)			// If neighbour is out of bounds
			return true;																// Return true if cell is an edge cell	

		if (observed[_neighbourIndex] != 0 && !(observed[_neighbourIndex] > 8			// If neighbour is not empty and not a wall
			&& observed[_neighbourIndex] < 13))	
			return true;																// Return true if cell is an edge cell									
	}
	return false;																		// Return false if cell is not an edge cell
}

//https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
bool ARunWFC::PointInPolygon(std::pair<int, int> point, std::vector<std::pair<int, int>> polygon)
{
	int num_vertices = polygon.size();
	double x = point.first, y = point.second;
	bool inside = false;

	// Store the first point in the polygon and initialize
	// the second point
	std::pair<int,int> p1 = polygon[0], p2;

	// Loop through each edge in the polygon
	for (int i = 1; i <= num_vertices; i++) 
	{
		// Get the next point in the polygon
		p2 = polygon[i % num_vertices];

		// Check if the point is above the minimum y
		// coordinate of the edge
		if (y > std::min(p1.second, p2.second)) 
		{
			// Check if the point is below the maximum y
			// coordinate of the edge
			if (y <= std::max(p1.second, p2.second)) 
			{
				// Check if the point is to the left of the
				// maximum x coordinate of the edge
				if (x <= std::max(p1.first, p2.first)) 
				{
					// Calculate the x-intersection of the
					// line connecting the point to the edge
					double x_intersection = (y - p1.second) * (p2.first - p1.first) / (p2.second - p1.second) + p1.first;

					// Check if the point is on the same
					// line as the edge or to the left of
					// the x-intersection
					if (p1.first == p2.first || x <= x_intersection) 
					{
						// Flip the inside flag
						inside = !inside;
					}
				}
			}
		}

		// Store the current point as the first point for
		// the next iteration
		p1 = p2;
	}

	// Return the value of the inside flag
	return inside;
}

// Called every frame
void ARunWFC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



