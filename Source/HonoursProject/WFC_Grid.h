// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WFC_Tile.h"
#include <vector>
#include "WFC_Grid.generated.h"

UCLASS()
class HONOURSPROJECT_API AWFC_Grid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWFC_Grid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void		InitGrid(int rows, int columns, FVector location);		// Initialise grid
	void		CreateGrid();											// Spawn tiles and create grid
	void		SetTileMesh(int index, int tile);						// Sets individual cell meshes after WFC completion
	void		DiscardMeshes();										// Discard meshes applied to tiles in grid
	void		DestroyTiles();											// Destroy tile actors
private:
	// Variables for grid creation
	int					GRID_SIZE;					// Grid Size
	int					ROW_SIZE, COLUMN_SIZE;		// Dimensions of grid

	FVector				_currentLocation;			// Holds current location of each new tile - used for placement calculations
	FVector				_baseLocation;
	TArray<AWFC_Tile*>	_tileArray;					// Tile array stores all created tiles 
	std::vector<int>	_finalTileStates;

	const FVector		TILE_SIZE = { 400, 400, 210.0 };	// Dimension of each cell/tile within grid
private:
	// Static Mesh assets for each tile type
	// Floor Meshes
	UStaticMesh*	_floor;

	// Doorway Meshes
	UStaticMesh*	_northDoorway;
	UStaticMesh*	_eastDoorway;
	UStaticMesh*	_southDoorway;
	UStaticMesh*	_westDoorway;
	
	// All walls
	UStaticMesh*	_northWall;
	UStaticMesh*	_eastWall;
	UStaticMesh*	_southWall;
	UStaticMesh*	_westWall;

	// Corner Meshes
	UStaticMesh*	_topLeftCorner;
	UStaticMesh*	_topRightCorner;
	UStaticMesh*	_bottomLeftCorner;
	UStaticMesh*	_bottomRightCorner;

	// Outside Corner
	UStaticMesh*	_topLeftOutCorner;
	UStaticMesh*	_topRightOutCorner;
	UStaticMesh*	_bottomLeftOutCorner;
	UStaticMesh*	_bottomRightOutCorner;

	// Strut Meshes
	UStaticMesh*	_topLeftStrut;
	UStaticMesh*	_topRightStrut;
	UStaticMesh*	_bottomLeftStrut;
	UStaticMesh*	_bottomRightStrut;

	// Map rebuild meshes
	// Corridor Meshes
	UStaticMesh*	_verticalCorridor;
	UStaticMesh*	_horizontalCorridor;

	// Dead End Meshes
	UStaticMesh*	_northDeadEnd;
	UStaticMesh*	_eastDeadEnd;
	UStaticMesh*	_southDeadEnd;
	UStaticMesh*	_westDeadEnd;

	UStaticMesh*	_eastTopWall;
	UStaticMesh*	_northTopWall;
	UStaticMesh*	_westTopWall;
	UStaticMesh*	_southTopWall;

	UStaticMesh*	_topLeftTopCorner;
	UStaticMesh*	_topRightTopCorner;
	UStaticMesh*	_bottomLeftTopCorner;
	UStaticMesh*	_bottomRightTopCorner;

	UStaticMesh*	_topFloor;

public:
	// Getters and setters
	int					GetGridSize()			{ return GRID_SIZE; };
	int					GetRowSize()			{ return ROW_SIZE; };
	int					GetColumnSize()			{ return COLUMN_SIZE; };
	FVector				GetTileSize()			{ return TILE_SIZE; };
	

	std::vector<int>&	GetStates()				{ return _finalTileStates; };
	TArray<AWFC_Tile*>	GetTileArray()			{ return _tileArray; };


	void				SetFinalTileStates(const std::vector<int>& states)
	{
		_finalTileStates = states;
	};
	void				SetFinalTileStates(int index, int newValue)
	{
		if (_finalTileStates.empty() || index >= _finalTileStates.size())
			return;

		_finalTileStates[index] = newValue;
	};

	std::vector<int>	GetTopRowGrid()
	{
		std::vector<int> _topRow;
		if (_finalTileStates.empty()) return _topRow;
		for (int i = 0; i < ROW_SIZE; i++)
		{
			_topRow.push_back(_finalTileStates[i]);
		}
		return _topRow;
	};
	std::vector<int>	GetBottomRowGrid()
	{
		std::vector<int> _bottomRow;
		if (_finalTileStates.empty()) return _bottomRow;
		for (int i = GRID_SIZE - ROW_SIZE; i < GRID_SIZE; i++)
		{
			_bottomRow.push_back(_finalTileStates[i]);
		}
		return _bottomRow;
	};
	std::vector<int>	GetLeftRowGrid()
	{
		std::vector<int> _leftRow;
		if (_finalTileStates.empty()) return _leftRow;
		for (int i = 0; i < GRID_SIZE; i += ROW_SIZE)
		{
			_leftRow.push_back(_finalTileStates[i]);
		}
		return _leftRow;
	};
	std::vector<int>	GetRightRowGrid()
	{
		std::vector<int> _rightRow;
		if (_finalTileStates.empty()) return _rightRow;
		for (int i = ROW_SIZE - 1; i < GRID_SIZE; i += ROW_SIZE)
		{
			_rightRow.push_back(_finalTileStates[i]);
		}
		return _rightRow;
	};
};	
