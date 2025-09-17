// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneratedMap.h"
#include "Kismet/GameplayStatics.h"
#include <fstream>
#include "Containers/Array.h"

// Sets default values
AGeneratedMap::AGeneratedMap() :
	ROW_SIZE(10), COLUMN_SIZE(10), CHUNK_ROW_SIZE(10), CHUNK_COLUMN_SIZE(10), _demoMode(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	MAX_DISTANCE = (CHUNK_COLUMN_SIZE * COLUMN_SIZE) / 2;

	_pathFinder.Init(ROW_SIZE * CHUNK_ROW_SIZE, COLUMN_SIZE * CHUNK_COLUMN_SIZE, 210, 210, false);

	_tempCounter = 0;

	_mapCount = 0;

	_deadEndCount = 0;
	_maxMapScore = 0.45f;
	_averageMapScore = 0.0f;
}

// Called when the game starts or when spawned
void AGeneratedMap::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		TArray<AActor*> _wfcActor;																// Initialise found actors array
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARunWFC::StaticClass(), _wfcActor);	// Find all instances of AMapGrid class

		if (_wfcActor.Num() > 0)																// If grid actors have been found
		{
			_wfc = Cast<ARunWFC>(_wfcActor[0]);													// Cast found grid to local grid variable
		}
	}
	_mapFilePath = FPaths::Combine(FPlatformProcess::BaseDir(), TEXT("Maps"));
	IFileManager::Get().MakeDirectory(*_mapFilePath, true);

}

// Called every frame
void AGeneratedMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/// 
/// Assign spawn and bomb site types to specific rooms
/// 
/// rooms - vector of rooms defined within the map
void AGeneratedMap::AssignRoomTypes(std::vector<ARoom*> rooms)
{
	_spawnRoomIndexes.clear();				// Clear spawn room indexes
	_bombSiteRoomIndexes.clear();			// Clear bomb site indexes
	_siteConnectorRoomIndexes.clear();		// Clear site connector indexes

	std::sort(rooms.begin(), rooms.end(), [](ARoom* a, ARoom* b) { return a->GetSpawnSiteScore() > b->GetSpawnSiteScore(); });	// Sort rooms based on spawn site score
 
	ARoom* _spawnA = rooms[0];	// Get the highest scoring room
	ARoom* _spawnB = nullptr;	// Set second spawn to a null pointer

	float _maxDistance = 0.0f;	// Set max distance to 0

	for (ARoom* _room : rooms)	// Loop through rooms
	{
		int _numRows = CHUNK_ROW_SIZE * ROW_SIZE;					// Calculate number of rows
		int _numColumns = CHUNK_COLUMN_SIZE * COLUMN_SIZE;			// Calculate number of columns

		_room->CalculateCentre(CHUNK_ROW_SIZE * ROW_SIZE, CHUNK_COLUMN_SIZE * COLUMN_SIZE);	// Calculate centre of room

		float _dist =	abs((_room->GetCentreIndex() % _numRows) - (_spawnA->GetCentreIndex() % _numRows)) +	// Calculate manhatten distance between current room and best scoring spawn
						abs((_room->GetCentreIndex() / _numRows) - (_spawnA->GetCentreIndex() / _numRows));
		if (_dist > _maxDistance)		// If calculated distance is greater than max distance
		{
			_maxDistance = _dist;		// Set max distance to distance
			_spawnB = _room;			// Set second spawn to current room
		}	
	}

	if (_spawnB)											// If second spawn has been set correctly
	{
		_spawnRoomIndexes.push_back(_spawnA->GetRoomID());	// Add first spawn to spawn room indexes
		_spawnRoomIndexes.push_back(_spawnB->GetRoomID());	// Add second spawn to spawn room indexes
	}


	for (auto room : rooms)									// Loop through rooms					
		ScoreForBombSites(room);							// Score each room for bombsite assignment
	

	std::sort(rooms.begin(), rooms.end(), [](ARoom* a, ARoom* b) { return a->GetBombSiteScore() > b->GetBombSiteScore(); });	// Sort room based on bomb site score

	_bombSiteRoomIndexes.push_back(rooms[0]->GetRoomID());	// Push highest scoring room to bomb indexes

	for (auto room : rooms)									// Loop through rooms
		ScoreForBombSites(room);							// Score each room for bombsite assignment

	std::sort(rooms.begin(), rooms.end(), [](ARoom* c, ARoom* d) { return c->GetBombSiteScore() > d->GetBombSiteScore(); });	// Sort rooms based on bomb site score
	
	_bombSiteRoomIndexes.push_back(rooms[0]->GetRoomID());	// Push highest scoring room to bomb indexes
												
	// Code for demo

	if (_wfc->GetFinalGrid())
	{
		for (int i = 0; i < rooms.size(); i++)
		{
			for (int j = 0; j < 2; j++)
			{
				if (rooms[i]->GetRoomID() == _bombSiteRoomIndexes[j])
				{
					for (auto tile : rooms[i]->GetTileIndexes())
					{
						_wfc->GetFinalGrid()->GetTileArray()[tile]->DrawBombBox();
					}
				}
				else if (rooms[i]->GetRoomID() == _spawnRoomIndexes[j])
				{
					for (auto tile : rooms[i]->GetTileIndexes())
					{

						_wfc->GetFinalGrid()->GetTileArray()[tile]->DrawSpawnBox();
					}
				}
			}
		}
	}
	ScoreMap();												// Score entire map
}

/// 
/// Assign each room a score based on how effective it would be as a bombsite
/// 
/// room - the room to be scored
void AGeneratedMap::ScoreForBombSites(ARoom* room)
{
	const float _areaWeight = 2.0;		// The weight of the area of the room
	const float _corridorWeight = 0.0;	// The weight of the corridor count
	const float _positionWeight = 2.0;	// The weight of the rooms position

	if (room->GetTileIndexes().size() < 30 || room->GetCorridorCount() < 2) // If the room is too small or has too few corridors
	{
		room->SetBombSiteScore(-20);										// Set the bomb site score to -20 to avoid selection	
		return;																// Return					
	}

	int _numRows = CHUNK_ROW_SIZE * ROW_SIZE;								// Calculate number of rows
	int _numColumns = CHUNK_COLUMN_SIZE * COLUMN_SIZE;						// Calculate number of columns

	room->CalculateCentre(_numRows, _numColumns);													// Calculate the centre of the room

	float _distanceFromCentre = abs((room->GetCentreIndex() % _numRows) - (_numRows / 2)) +			// Calculate the distance from the room to the centre of the map
								abs((room->GetCentreIndex() / _numRows) - (_numColumns / 2));

	float _distanceFactor = 1.0f - NormaliseScoreFactors(_distanceFromCentre, MAX_DISTANCE, 0);		// Normalise the distance factor

	float		_proximityPenalty = 0.0f;															// Set proximity penalty to 0
	const float _proximityPenaltyWeight = -2.5f;													// Set proximity penalty weight to -2.5

	if (!_bombSiteRoomIndexes.empty())																// If there are other bomb sites asigned 
	{
		const float _minSiteDistance = 20.f * (210.f * 210.f);										// Set the minimum distance a bomb site can be from another before incurring a penalty

		for (int _bombSiteIndex : _bombSiteRoomIndexes)												// Loop through the bomb site indexes
		{
			ARoom* _bombSite = nullptr;															// Create a pointer to an existing site

			for (ARoom* _room : _wfc->GetRooms())												// Loop through rooms
			{
				if (_room->GetRoomID() == _bombSiteIndex)										// If the room id is equal to the bomb site index
				{
					_bombSite = _room;															// Set the existing site to the room
					break;																		// Break
				}				
			}

			if (_bombSite == room)																// If the existing site is the same as the room being observed
				_proximityPenalty -= 20.f;														// Set the proximity penalty to -20 to avoid double selection of the same site
		
			if (_bombSite)																		// If the existing site is not null
			{
				int _siteRow = _bombSite->GetCentreIndex() % _numRows;							// Get the row of the existing site
				int _siteCol = _bombSite->GetCentreIndex() / _numRows;							// Get the column of the existing site		

				float _distanceToSite = abs(_siteRow - (room->GetCentreIndex() % _numRows)) +	// Get distance to exisiting site from current room
											abs(_siteCol - (room->GetCentreIndex() / _numRows));

				if (_distanceToSite < _minSiteDistance)											// If the distance to the existing site is less than the min
					_proximityPenalty += _proximityPenaltyWeight;								// Add the penalty per distance to the proximity penalty
				
			}
		}
	}

	float _areaFactor = NormaliseScoreFactors(room->GetRoomArea(), MAX_AREA, 0);				// Normalise the area factor	
	float _corridorFactor = NormaliseScoreFactors(room->GetCorridorCount(), MAX_CORRIDORS, 1);	// Normalise the corridor factor

	room->SetBombSiteScore((_areaFactor * _areaWeight) + (_distanceFactor * _positionWeight) + _proximityPenalty); // Set rooms score eligibility for bombsite assingment 
}

void AGeneratedMap::ScoreForSpawnPoint(ARoom* room)
{
	const float _areaWeight = 0.0;
	const float _corridorWeight = 0.0;
	const float _positionWeight = 2.5;

	int		_approxRoomIndex = room->GetTileIndexes()[room->GetTileIndexes().size() / 2];
	float	_roomArea = room->GetTileIndexes().size() * (210.f * 210.f);

	int _numRows = CHUNK_ROW_SIZE * ROW_SIZE;
	int _numColumns = CHUNK_COLUMN_SIZE * COLUMN_SIZE;
	int _roomRow = _approxRoomIndex % _numRows;
	int _roomColumn = _approxRoomIndex / _numRows;

	int minRow = INT_MAX, maxRow = INT_MIN;
	int minCol = INT_MAX, maxCol = INT_MIN;

	for (int tileIndex : room->GetTileIndexes())
	{
		int tileRow = tileIndex % _numRows;
		int tileCol = tileIndex / _numRows;

		minRow = std::min(minRow, tileRow);
		maxRow = std::max(maxRow, tileRow);
		minCol = std::min(minCol, tileCol);
		maxCol = std::max(maxCol, tileCol);
	}

	int roomCenterRow = (minRow + maxRow) / 2;
	int roomCenterCol = (minCol + maxCol) / 2;

	int distanceToLeft = roomCenterCol; 
	int distanceToRight = _numColumns - roomCenterCol; 
	int distanceToTop = roomCenterRow; 
	int distanceToBottom = _numRows - roomCenterRow; 


	int distanceToNearestEdge = std::min({ distanceToLeft, distanceToRight, distanceToTop, distanceToBottom });

	
	float _edgeFactor = NormaliseScoreFactors(distanceToNearestEdge, MAX_DISTANCE, 0);
	_edgeFactor = 1.0f - _edgeFactor; 

	
	float _areaFactor = NormaliseScoreFactors(_roomArea, MAX_AREA, 0);
	float _corridorFactor = NormaliseScoreFactors(room->GetCorridorCount(), MAX_CORRIDORS, 1);

	
	float finalScore = (_areaFactor * _areaWeight) +
		(_corridorFactor * _corridorWeight) +
		(_edgeFactor * _positionWeight);

	room->SetSpawnSiteScore(finalScore);
}

void AGeneratedMap::ScoreMap()
{
	_bombSites.clear();
	_spawns.clear();

	for (int i = 0; i < _wfc->GetRooms().size(); i++)
	{
		for (int j = 0; j < _bombSiteRoomIndexes.size(); j++)
			if (_wfc->GetRooms()[i]->GetRoomID() == _bombSiteRoomIndexes[j])
				_bombSites.push_back(_wfc->GetRooms()[i]);

		for (int j = 0; j < _spawnRoomIndexes.size(); j++)
			if (_wfc->GetRooms()[i]->GetRoomID() == _spawnRoomIndexes[j])
				_spawns.push_back(_wfc->GetRooms()[i]);
	}


	_mapScore = 0.0f;
	Node _start;
	Node _end;
	std::vector<Node> _path;
	// 1. score based on distance from each spawn to each bombsite and get average path length
	// 2. Score based on whether or not each spawn has similar path lengths to the same site.
	float _avgPathLength = 0.0f;
	std::vector<float> _spawnOnePathLength;
	std::vector<float> _spawnTwoPathLength;
	_pathFindMap = _wfc->GetPathfindGrid();
	for (int i = 0; i < _spawns.size(); i++)
	{
		_start._x = _spawns[i]->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
		_start._y = _spawns[i]->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
		_start._tileType = 0;

		for (int j = 0; j < _bombSites.size(); j++)
		{
			_end._x = _bombSites[j]->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
			_end._y = _bombSites[j]->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
			_end._tileType = 0;
		
			_path = _pathFinder.AStar(_start, _end, _pathFindMap);
			_avgPathLength += _path.size();

			if (i == 0)
				_spawnOnePathLength.push_back(_path.size());
			else
				_spawnTwoPathLength.push_back(_path.size());
		}
	}

	_avgPathLength /= 4.f;
	float _spawnOneAvgPath = (_spawnOnePathLength.front() + _spawnOnePathLength.back()) / 2.f;
	float _spawnTwoAvgPath = (_spawnTwoPathLength.front() + _spawnTwoPathLength.back()) / 2.f;

	for (int i = 0; i < _spawnOnePathLength.size(); i++)
		if (_spawnOnePathLength[i] < 25)
		{
		//	UE_LOG(LogTemp, Log, TEXT("Bombsite too close"));
			_mapScore -= 0.5f;
		}
	
	for (int i = 0; i < _spawnTwoPathLength.size(); i++)
		if (_spawnTwoPathLength[i] < 25)
		{
		//	UE_LOG(LogTemp, Log, TEXT("Bombsite too close"));
			_mapScore -= 0.5f;
		}

//	UE_LOG(LogTemp, Log, TEXT("Average path length %f"), _avgPathLength);
//	UE_LOG(LogTemp, Log, TEXT("Spawn one average path length %f"), _spawnOneAvgPath);
//	UE_LOG(LogTemp, Log, TEXT("Spawn two average path length %f"), _spawnTwoAvgPath);

	float _pathLengthDiff = fabs(_spawnOneAvgPath - _spawnTwoAvgPath);
//	UE_LOG(LogTemp, Log, TEXT("Difference between path lengths %f"), _pathLengthDiff);
	// 3. Penalize dead ends? - basically go through rooms and get corridor count, any room with 1 connection is a dead end most of the time
	_deadEndCount = 0;
	for (auto room : _wfc->GetRooms())
	{
		if (room->GetCorridorCount() <= 1)
			_deadEndCount++;
	}

//	UE_LOG(LogTemp, Log, TEXT("Dead End Count %d"), _deadEndCount);

	// 4. Pathfind between both bombsites, too close = less score
	_start._x = _bombSites.front()->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._y = _bombSites.front()->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._tileType = 0;

	_end._x = _bombSites.back()->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._y = _bombSites.back()->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._tileType = 0;

	_path = _pathFinder.AStar(_start, _end, _pathFindMap);
	int _distBetweenSites = _path.size();
//	UE_LOG(LogTemp, Log, TEXT("Distance between sites %d"), _distBetweenSites);


	// 5. Pathfind spawn to spawn, if this is reachable before both spawn to site just disqualify map.
	_start._x = _spawns.front()->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._y = _spawns.front()->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._tileType = 0;

	_end._x = _spawns.back()->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._y = _spawns.back()->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._tileType = 0;

	_path = _pathFinder.AStar(_start, _end, _pathFindMap);
	int _distBetweenSpawns = _path.size();
//	UE_LOG(LogTemp, Log, TEXT("Distance between spawns %d"), _distBetweenSpawns);

	bool _siteReachedFirst = false;

	if (_distBetweenSpawns < _spawnOneAvgPath || _distBetweenSpawns < _spawnTwoAvgPath)
	{
		_siteReachedFirst = true; // change name ???
		_mapScore -= 0.5f;
//		UE_LOG(LogTemp, Log, TEXT("Enemy spawn reachable before bomb sites"));
	}
	else
	{
		_siteReachedFirst = false;
	//	UE_LOG(LogTemp, Log, TEXT("Bombsites reachable before enemy spawn sites"));
	}


	float _avgPathScore = 1.0f - NormaliseScoreFactors(_avgPathLength, 100, 0);
	float _avgSpawnOnePathScore = 1.0f - NormaliseScoreFactors(_spawnOneAvgPath, 100, 0);
	float _avgSpawnTwoPathScore = 1.0f - NormaliseScoreFactors(_spawnTwoAvgPath, 100, 0);
	float _pathDifferenceScore = 1.0f - NormaliseScoreFactors(_pathLengthDiff, 40, 0);
	float _deadEndsScore = 1.0f - NormaliseScoreFactors(_deadEndCount, 30, 0);
	float _distBetweenSiteScore = NormaliseScoreFactors(_distBetweenSites, 100, 0);
	float _distBetweenSpawnScore = NormaliseScoreFactors(_distBetweenSpawns, 150, 0);
	float _siteReachableFirstScore = _siteReachedFirst ? 1.0f : 0.0f;


	_mapScore += (_avgPathScore * 0.2f) +
		(_avgSpawnOnePathScore * 0.1f) +
		(_avgSpawnTwoPathScore * 0.1f) +
		(_pathDifferenceScore * 0.15f) +
		(_deadEndsScore * 0.1f) +
		(_distBetweenSiteScore * 0.1f) +
		(_distBetweenSpawnScore * 0.15f) +
		(_siteReachableFirstScore * 0.1f);


	_mapScores.push_back(_mapScore);
	_mapScore = 0;
}

void AGeneratedMap::CalculateMxScoreFactors(std::vector<ARoom*> rooms)
{
	for (const auto& _room : rooms)
	{
		_room->CalculateArea();
		if (_room->GetRoomArea() > MAX_AREA)
			MAX_AREA = _room->GetRoomArea();

		if (_room->GetCorridorCount() > MAX_CORRIDORS)
			MAX_CORRIDORS = _room->GetCorridorCount();
	}
}

float AGeneratedMap::NormaliseScoreFactors(float factor, float max, float min)
{
	if (max == min)  
		return 0.0f;  

	return (factor - min) / (max - min);
}

void AGeneratedMap::SaveMap(float score, std::vector<int> map, std::vector<int> spawn1, std::vector<int> spawn2, std::vector<int> bombsite1, std::vector<int> bombsite2, int deadEnds, float avgCorridor)
{
	UE_LOG(LogTemp, Log, TEXT("Saving map"));

	const std::vector<int>& _mapSave = _wfc->GetCombinedGrids();
	FString _filename = FString::Printf(TEXT("%.4f"), score);
	FString _fullPath = FPaths::ProjectSavedDir() + _filename;	// Editor File path
//	FString _fullPath = _mapFilePath + _filename;					// .exe file path
	std::ofstream _fs(TCHAR_TO_UTF8(*_fullPath), std::ios::binary);

	// Write Map score to file
	_fs.write(reinterpret_cast<const char*>(&score), sizeof(score));

	// Write map to file
	size_t _mapSize = _mapSave.size();
	_fs.write(reinterpret_cast<const char*>(&_mapSize), sizeof(_mapSize));
	_fs.write(reinterpret_cast<const char*>(_mapSave.data()), _mapSize * sizeof(int));

	// Write first spawn
	size_t _spawnOneSize = spawn1.size();
	_fs.write(reinterpret_cast<const char*>(&_spawnOneSize), sizeof(_spawnOneSize));
	_fs.write(reinterpret_cast<const char*>(spawn1.data()), _spawnOneSize * sizeof(int));

	// Write second spawn
	size_t _spawnTwoSize = spawn2.size();
	_fs.write(reinterpret_cast<const char*>(&_spawnTwoSize), sizeof(_spawnTwoSize));
	_fs.write(reinterpret_cast<const char*>(spawn2.data()), _spawnTwoSize * sizeof(int));

	// Write first bombsite
	size_t _bombSiteOneSize = bombsite1.size();
	_fs.write(reinterpret_cast<const char*>(&_bombSiteOneSize), sizeof(_bombSiteOneSize));
	_fs.write(reinterpret_cast<const char*>(bombsite1.data()), _bombSiteOneSize * sizeof(int));

	// Write second bombsite
	size_t _bombSiteTwoSize = bombsite2.size();
	_fs.write(reinterpret_cast<const char*>(&_bombSiteTwoSize), sizeof(_bombSiteTwoSize));
	_fs.write(reinterpret_cast<const char*>(bombsite2.data()), _bombSiteTwoSize * sizeof(int));

	// Write deadends to file
	_fs.write(reinterpret_cast<const char*>(&deadEnds), sizeof(deadEnds));

	// Write average corridor count per room to file
	_fs.write(reinterpret_cast<const char*>(&avgCorridor), sizeof(avgCorridor));

	_fs.close();
	UE_LOG(LogTemp, Log, TEXT("Map saved"));
}

void AGeneratedMap::DisplayLoadedMap()
{
	Node _start;
	Node _end;
	std::vector<Node> _path;


	_pathFindMap = _map;
	for (int i = 0; i < _pathFindMap.size(); i++)
	{
		if (_pathFindMap[i] >= Floor)
		{
			_pathFindMap[i] = 0;
		}
	}

	ARoom* _spawnOneRoom = GetWorld()->SpawnActor<ARoom>();
	_spawnOneRoom->SetTileIndexes(_spawnOne);
	_spawnOneRoom->CalculateCentre(ROW_SIZE * CHUNK_ROW_SIZE, COLUMN_SIZE * CHUNK_COLUMN_SIZE);
	ARoom* _spawnTwoRoom = GetWorld()->SpawnActor<ARoom>();
	_spawnTwoRoom->SetTileIndexes(_spawnTwo);
	_spawnTwoRoom->CalculateCentre(ROW_SIZE * CHUNK_ROW_SIZE, COLUMN_SIZE * CHUNK_COLUMN_SIZE);

	ARoom* _bombsiteOneRoom = GetWorld()->SpawnActor<ARoom>();
	_bombsiteOneRoom->SetTileIndexes(_bombsiteOne);
	_bombsiteOneRoom->CalculateCentre(ROW_SIZE * CHUNK_ROW_SIZE, COLUMN_SIZE * CHUNK_COLUMN_SIZE);
	ARoom* _bombsiteTwoRoom = GetWorld()->SpawnActor<ARoom>();
	_bombsiteTwoRoom->SetTileIndexes(_bombsiteTwo);
	_bombsiteTwoRoom->CalculateCentre(ROW_SIZE * CHUNK_ROW_SIZE, COLUMN_SIZE * CHUNK_COLUMN_SIZE);

	// 1. score based on distance from each spawn to each bombsite and get average path length
	// 2. Score based on whether or not each spawn has similar path lengths to the same site.
	float _avgPathLength = 0.0f;
	std::vector<float> _spawnOnePathLength;
	std::vector<float> _spawnTwoPathLength;

	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			_start._x = _spawnOneRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
			_start._y = _spawnOneRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
		}
		else
		{
			_start._x = _spawnTwoRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
			_start._y = _spawnTwoRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
		}
	
		_start._tileType = 0;

		for (int j = 0; j < 2; j++)
		{
			if (i == 0)
			{
				_end._x = _bombsiteOneRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
				_end._y = _bombsiteOneRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
			}
			else
			{
				_end._x = _bombsiteTwoRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
				_end._y = _bombsiteTwoRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
			}
			
			
			_end._tileType = 0;

			_path = _pathFinder.AStar(_start, _end, _pathFindMap);
			_avgPathLength += _path.size();

			if (i == 0)
				_spawnOnePathLength.push_back(_path.size());
			else
				_spawnTwoPathLength.push_back(_path.size());
		}
	}

	_avgPathLength /= 4.f;
	float _spawnOneAvgPath = (_spawnOnePathLength.front() + _spawnOnePathLength.back()) / 2.f;
	float _spawnTwoAvgPath = (_spawnTwoPathLength.front() + _spawnTwoPathLength.back()) / 2.f;


	UE_LOG(LogTemp, Log, TEXT("Average path length %f"), _avgPathLength);
	UE_LOG(LogTemp, Log, TEXT("Spawn one average path length %f"), _spawnOneAvgPath);
	UE_LOG(LogTemp, Log, TEXT("Spawn two average path length %f"), _spawnTwoAvgPath);

	float _pathLengthDiff = fabs(_spawnOneAvgPath - _spawnTwoAvgPath);
	UE_LOG(LogTemp, Log, TEXT("Difference between path lengths %f"), _pathLengthDiff);
		
	UE_LOG(LogTemp, Log, TEXT("Dead End Count %d"), _deadEndCount);


	// 4. Pathfind between both bombsites, too close = less score
	_start._x = _bombsiteOneRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._y = _bombsiteOneRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._tileType = 0;

	_end._x = _bombsiteTwoRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._y = _bombsiteTwoRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._tileType = 0;

	_path = _pathFinder.AStar(_start, _end, _pathFindMap);
	int _distBetweenSites = _path.size();
	UE_LOG(LogTemp, Log, TEXT("Distance between sites %d"), _distBetweenSites);

	// 5. Pathfind spawn to spawn, if this is reachable before both spawn to site just disqualify map.
	_start._x = _spawnOneRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._y = _spawnOneRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_start._tileType = 0;

	_end._x = _spawnTwoRoom->GetCentreIndex() % (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._y = _spawnTwoRoom->GetCentreIndex() / (ROW_SIZE * CHUNK_ROW_SIZE);
	_end._tileType = 0;

	_path = _pathFinder.AStar(_start, _end, _pathFindMap);
	int _distBetweenSpawns = _path.size();
	UE_LOG(LogTemp, Log, TEXT("Distance between spawns %d"), _distBetweenSpawns);
}

void AGeneratedMap::LoadMap(FString filename)
{
	FString _fullPath = FPaths::ProjectSavedDir() + filename;       // Editor File path
	//FString _fullPath = _mapFilePath + filename;					// .exe file path//
	std::ifstream _fs(TCHAR_TO_UTF8(*_fullPath), std::ios::binary);

	// Read score
	_mapScore = 0.0f;
	_fs.read(reinterpret_cast<char*>(&_mapScore), sizeof(_mapScore));

	// Read in map
	size_t _mapSize;
	_fs.read(reinterpret_cast<char*>(&_mapSize), sizeof(_mapSize));
	_map.resize(_mapSize);
	_fs.read(reinterpret_cast<char*>(_map.data()), _mapSize * sizeof(int));

	// Read in spawn one 
	size_t _spawnOneSize;
	_fs.read(reinterpret_cast<char*>(&_spawnOneSize), sizeof(_spawnOneSize));
	_spawnOne.resize(_spawnOneSize);
	_fs.read(reinterpret_cast<char*>(_spawnOne.data()), _spawnOneSize * sizeof(int));

	// Read in spawn two
	size_t _spawnTwoSize;
	_fs.read(reinterpret_cast<char*>(&_spawnTwoSize), sizeof(_spawnTwoSize));
	_spawnTwo.resize(_spawnTwoSize);
	_fs.read(reinterpret_cast<char*>(_spawnTwo.data()), _spawnTwoSize * sizeof(int));

	// Read in bombsite one
	size_t _bombSiteOneSize;
	_fs.read(reinterpret_cast<char*>(&_bombSiteOneSize), sizeof(_bombSiteOneSize));
	_bombsiteOne.resize(_bombSiteOneSize);
	_fs.read(reinterpret_cast<char*>(_bombsiteOne.data()), _bombSiteOneSize * sizeof(int));

	// Read in bombsite two
	size_t _bombSiteTwoSize;
	_fs.read(reinterpret_cast<char*>(&_bombSiteTwoSize), sizeof(_bombSiteTwoSize));
	_bombsiteTwo.resize(_bombSiteTwoSize);
	_fs.read(reinterpret_cast<char*>(_bombsiteTwo.data()), _bombSiteTwoSize * sizeof(int));

	// Read dead ends
	_deadEndCount = 0;
	_fs.read(reinterpret_cast<char*>(&_deadEndCount), sizeof(_deadEndCount));

	_avgCorridor = 0;
	_fs.read(reinterpret_cast<char*>(&_avgCorridor), sizeof(_avgCorridor));

	_fs.close();

	_wfc->Cleanup();

	_wfc->GetFinalGrid()->SetFinalTileStates(_map);
	for (int i = 0; i < _map.size(); i++)
	{
		_wfc->GetFinalGrid()->SetTileMesh(i, _map[i]);
	}

	for (auto _spawnIndex : _spawnOne)
		_wfc->GetFinalGrid()->GetTileArray()[_spawnIndex]->DrawSpawnBox();
	
	for (auto _spawnIndex : _spawnTwo)
		_wfc->GetFinalGrid()->GetTileArray()[_spawnIndex]->DrawSpawnBox();
	
	for (auto _bombIndex : _bombsiteOne)
		_wfc->GetFinalGrid()->GetTileArray()[_bombIndex]->DrawBombBox();

	for (auto _bombIndex : _bombsiteTwo)
		_wfc->GetFinalGrid()->GetTileArray()[_bombIndex]->DrawBombBox();

	DisplayLoadedMap();
}

void AGeneratedMap::Score()
{
	if (_wfc->GetRooms().size() > 0)
	{
		CalculateMxScoreFactors(_wfc->GetRooms());
		for (int i = 0; i < _wfc->GetRooms().size(); i++)
		{
			ScoreForSpawnPoint(_wfc->GetRooms()[i]);

		}
		AssignRoomTypes(_wfc->GetRooms());
	}
}

void AGeneratedMap::GenerateMap()
{
	_mapScores.clear();

	TimerCall(); 
}

void AGeneratedMap::RunThroughRooms()
{
	ARoom* bombRoom = _wfc->GetRooms()[_tempCounter];
	UE_LOG(LogTemp, Log, TEXT("Room %d Bomb Score: %f"), bombRoom->GetRoomID(), bombRoom->GetBombSiteScore());
	for (int i = 0; i < bombRoom->GetTileIndexes().size(); i++)
	{
		for (int tileIndex : bombRoom->GetTileIndexes())
		{
			_wfc->GetFinalGrid()->GetTileArray()[tileIndex]->DrawColourBox();
		}
	}

	_tempCounter++;
}

void AGeneratedMap::BlueprintSave()
{
	SaveMap(_mapScores[_mapCount], _wfc->GetCombinedGrids(), _spawns.front()->GetTileIndexes(), _spawns.back()->GetTileIndexes(), _bombSites.front()->GetTileIndexes(), _bombSites.back()->GetTileIndexes(), _deadEndCount, _avgCorridor);
}

void AGeneratedMap::RepeatWFC()
{
	if (_wfc->GetGridPosition() < ROW_SIZE * COLUMN_SIZE)
	{
		_wfc->RunAlgorithm();
		
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("current map %d"), _mapCount);
		GetWorldTimerManager().ClearTimer(_wfcTimerHandle);
		_wfc->CombineGrids();
		UE_LOG(LogTemp, Log, TEXT("combining grids..."));

		if (_demoMode)
			return;

		_wfc->EvaluateRooms();
		UE_LOG(LogTemp, Log, TEXT("evaluating rooms..."));

		_wfc->Cleanup();
		UE_LOG(LogTemp, Log, TEXT("cleaning up map..."));

		_wfc->RunPathFind();
		UE_LOG(LogTemp, Log, TEXT("running pathfind..."));

		_pathFindMap = _wfc->GetPathfindGrid();
		//Score();
		UE_LOG(LogTemp, Log, TEXT("scoring map..."));
		
		
	}
}

void AGeneratedMap::TimerCall()
{
	if (_wfc)
	{
		GetWorldTimerManager().SetTimer(_wfcTimerHandle, this, &AGeneratedMap::RepeatWFC, 0.01f, true);
	}
} 

