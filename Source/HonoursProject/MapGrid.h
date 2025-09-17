// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WFC_Grid.h"
#include <vector>
#include "HelperClass.h"
#include "MapGrid.generated.h"

UCLASS()
class HONOURSPROJECT_API AMapGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void	Tick(float DeltaTime) override;
	void			InitGrid(int rows, int columns, int chunkRows, int chunkColumns);	// Initialise grid
	void			CreateMapGrid();													// Create grid
	void			GenerateChunks(int index, const std::vector<int>& observedArr);		// Apply meshes to chunks
	void			DiscardChunks();													// Discard all generated chunks

private:
	int					GRID_SIZE;								// Size of chunk grid
	int					ROW_SIZE, COLUMN_SIZE;					// Dimensions of grid
	int					CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE;		// Dimensions of each chunk

	TArray<AWFC_Grid*>	_chunkGridArray;						// Array of chunks

	std::vector<int>	_dx = { 1, 0, -1, 0 };					// Change of direction in x plane
	std::vector<int>	_dy = { 0, 1, 0, -1 };					// Change of direction in y plane

	FVector				_currentLocation;						// Current location of each chunk

public:
	// Getters and setters
	TArray<AWFC_Grid*>	GetGridArray() { return _chunkGridArray; };
	NeighbourChunk		GetChunkEdges(int currentChunk);
};
