// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Room.h"
#include "RunWFC.h"
#include "AStarPathfind.h"
#include <map>
#include "GeneratedMap.generated.h"


UCLASS()
class HONOURSPROJECT_API AGeneratedMap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGeneratedMap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;	
	void		 AssignRoomTypes(std::vector<ARoom*> rooms);				// Assigns room types to rooms
	void		 CalculateMxScoreFactors(std::vector<ARoom*> rooms);		// Calculates max score factors

	void		 ScoreForBombSites(ARoom* room);							// Scores bomb sites
	void		 ScoreForSpawnPoint(ARoom* room);							// Scores spawn points
	void		 ScoreMap();												// Scores map
	void		 RepeatWFC();												// Repeats WFC	
	void		 TimerCall();												// Timer call
	float		 NormaliseScoreFactors(float factor, float max, float min);	// Normalises score factors

	void		 SaveMap(float score, std::vector<int> map, std::vector<int> spawn1, std::vector<int> spawn2, std::vector<int> bombsite1, std::vector<int> bombsite2, int deadEnds, float avgCorridor); // Save Map

	void		 DisplayLoadedMap();										// Display loaded map

	UFUNCTION(BlueprintCallable)
	void		 LoadMap(FString filename);	// Load Map

	UFUNCTION(BlueprintCallable)
	void		 Score();					// Score

	UFUNCTION(BlueprintCallable)
	void		 GenerateMap();				// Generate Map

	UFUNCTION(BlueprintCallable)
	void		 RunThroughRooms();			// Run through rooms highlights each room with coloured boxes

	UFUNCTION(BlueprintCallable)
	void 		BlueprintSave();

private:
	
	int							ROW_SIZE, COLUMN_SIZE;						// Dimensions of grid		
	int							CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE;			// Chunk dimensions
	float						MAX_AREA, MAX_CORRIDORS, MAX_DISTANCE;		// Max area, corridors and distance

	const int					TILE_AREA = 210 * 210;						// Tile area			
	const int					MAX_SPAWN_POINTS = 2, MAX_BOMB_SITES = 2;	// Max number of spawn points and bomb sites

	int							_tempCounter;						// Temporary counter for debugging
	bool						_demoMode;							// Demo mode flag
	int							_mapCount;							// Number of maps generated
	int							_deadEndCount;						// Number of dead ends in a map
	float						_maxMapScore;						// Max map score used to determine whether maps are worth saving
	float						_averageMapScore;					// Average map score
	
	std::vector<int>			_pathFindMap;						// Map used for pathfinding
	std::vector<int>			_spawnRoomIndexes;					// Spawn room indexes	
	std::vector<int>			_bombSiteRoomIndexes;				// Bomb site room indexes
	std::vector<int>			_siteConnectorRoomIndexes;			// Room indexes for site connectors
	std::vector<float>			_mapScores;							// Map scores
	std::vector<ARoom*>			_spawns;							// Spawn points		
	std::vector<ARoom*>			_bombSites;							// Bomb sites


	ARunWFC*					_wfc;								// Wave Function Collapse
	AStarPathfind				_pathFinder;						// Pathfinding algorithm
	FTimerHandle				_wfcTimerHandle;					// Timer handle

	FString						_mapFilePath;



	float _mapScore;
	int _avgCorridor;
	std::vector<int> _bombsiteOne;
	std::vector<int> _bombsiteTwo;
	std::vector<int> _spawnOne;
	std::vector<int> _spawnTwo;
	std::vector<int> _map;
};

