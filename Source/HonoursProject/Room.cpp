// Fill out your copyright notice in the Description page of Project Settings.


#include "Room.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/// 
/// Calculate the area of the room
/// 
void ARoom::CalculateArea()
{
	if (_tileIndexes.empty())						// If no tiles return 
		return;

	_roomArea = _tileIndexes.size() * _tileArea;	// Calculate area of room
}

/// 
/// Calculate the centre of the room
/// 
/// numRows - number of rows in room
/// numColumns - number of columns in room
void ARoom::CalculateCentre(int numRows, int numColumns)
{
	int _minRow = INT_MAX, _maxRow = INT_MIN;	// Initialize min and max row and column values
	int _minCol = INT_MAX, _maxCol = INT_MIN;	// Initialize min and max row and column values

	for (int _tileIndex : _tileIndexes)			// Loop through each tile index
	{
		int _tileRow = _tileIndex % numRows;	// Calculate row of tile
		int _tileCol = _tileIndex / numRows;	// Calculate column of tile

		_minRow = std::min(_minRow, _tileRow);	// Update min row values
		_maxRow = std::max(_maxRow, _tileRow);	// Update max row values
		_minCol = std::min(_minCol, _tileCol);	// Update min column values
		_maxCol = std::max(_maxCol, _tileCol);	// Update max column values
	}

	int roomCentreRow = (_minRow + _maxRow) / 2;	// Calculate centre row of room
	int roomCentreCol = (_minCol + _maxCol) / 2;	// Calculate centre column of room

	_roomCentreIndex = roomCentreCol * numRows + roomCentreRow; // Calculate centre index of room
}
