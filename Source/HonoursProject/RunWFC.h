// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveFunctionCollapse_Model.h"
#include "WFC_Grid.h"
#include "MapGrid.h"
#include "AStarPathfind.h"
#include "Graph.h"
#include "Room.h"
#include "HelperClass.h"
#include "ConvexHull.h"
#include "RunWFC.generated.h"

UCLASS()
class HONOURSPROJECT_API ARunWFC : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARunWFC();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void		CombineGrids();																		// Combine grid chunk vectors to create final map vector		
	void		CombineRooms();																		// Combine rooms that match a certain criteria
	void		ConvexHullPathCreation();															// Create a path from the convex hull of the rooms
	void		RoomRebuild(std::vector<int>& observed, int x, int y);								// Rebuild rooms after pathfinding
	void		RoomDefinition(std::vector<int>& observed, int x, int y, int roomID, ARoom& room);	// Recursive flood fill function to define rooms

	bool		IsEdgeTile(int x, int y, std::vector<int>& observed);								// Check if a tile is an edge tile of a room				
	bool		PointInPolygon(std::pair<int, int> point, std::vector<std::pair<int, int>> polygon);// Check if a point is within a polygon
	UFUNCTION(BlueprintCallable)
	void		EvaluateRooms();																	// Search map space to define rooms
	UFUNCTION(BlueprintCallable)
	bool		RunAlgorithm();																		// Run the WFC algorithm
	UFUNCTION(BlueprintCallable)
	void		Cleanup();																			// Clean up the grid
	UFUNCTION(BlueprintCallable)
	void		ResetGrid();																		// Reset the grid
	UFUNCTION(BlueprintCallable)
	void		SetTileWeights(const TArray<float>& inputWeight);									// Set the tile weights
	UFUNCTION(BlueprintCallable)
	void		RunPathFind();																		// Run the pathfinding algorithm
private: 
	AWaveFunctionCollapse_Model*	WaveFunctionCollapseModel;										// WFC model

	AWFC_Grid*						_finalGrid;										// Final grid that represents the map
	AMapGrid*						_mapGrid;										// Map grid contains each chunk of map to be generated	

	int								ROW_SIZE, COLUMN_SIZE;                          // Dimensions of the grid of chunks
	int								CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE;				// Dimensions of the chunks
	int								_gridPos;										// Current grid position	
	int								_attempts;										// Number of attempts to generate a chunk

	bool							_success;										// Flag to indicate if the algorithm was successful
	bool							_demoMode;										// Flag to indicate if the algorithm is in demo mode

	std::vector<ARoom*>				_rooms;											// Vector of rooms

	std::vector<int>				_gridCells;										// Vector of grid cells
	std::vector<int>				_combinedGrids;									// Stores the final map 
	std::vector<int>				_pathfindGrid;									// Stores the pathfinding grid
	std::vector<std::vector<int>>	_chunkGrids;									// Contains output vector for each WFC chunk

	std::vector<int>				_dx = { 1, 0, -1, 0 };							// Direction vectors
	std::vector<int>				_dy = { 0, 1, 0, -1 };							// Direction vectors
public:
	// Getters and setters
	int						GetGridPosition()	{ return _gridPos; };
	AWFC_Grid*				GetFinalGrid()		{ return _finalGrid; };

	std::vector<ARoom*>&	GetRooms()			{ return _rooms; };
	std::vector<int>		GetCombinedGrids()	{ return _combinedGrids; };
	std::vector<int>		GetPathfindGrid()	{ return _pathfindGrid; };
	
};
