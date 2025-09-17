// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC_Grid.h"

// Sets default values
AWFC_Grid::AWFC_Grid() :
	 _currentLocation({0.0f,0.0f,0.0f})
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Load floor mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Floor(TEXT("StaticMesh'/Game/DungeonExpansion/floor_tile_large.floor_tile_large'"));
	_floor = Floor.Object;		

	// Load wall meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NorthWall(TEXT("StaticMesh'/Game/DungeonExpansion/North_Wall.North_Wall'"));
	_northWall = NorthWall.Object; 
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EastWall(TEXT("StaticMesh'/Game/DungeonExpansion/East_Wall.East_Wall'"));
	_eastWall = EastWall.Object; 
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SouthWall(TEXT("StaticMesh'/Game/DungeonExpansion/South_Wall.South_Wall'"));
	_southWall = SouthWall.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WestWall(TEXT("StaticMesh'/Game/DungeonExpansion/West_Wall.West_Wall'"));
	_westWall = WestWall.Object;
	
	//
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TLInsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/TL_Inside_Corner.TL_Inside_Corner'"));
	_topLeftCorner = TLInsideCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TRInsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/TR_Inside_Corner.TR_Inside_Corner'"));
	_topRightCorner = TRInsideCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BLInsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/BL_Inside_Corner.BL_Inside_Corner'"));
	_bottomLeftCorner = BLInsideCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BRInsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/BR_Inside_Corner.BR_Inside_Corner'"));
	_bottomRightCorner = BRInsideCorner.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TLOutsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/TL_Outside_Corner.TL_Outside_Corner'"));
	_topLeftOutCorner = TLOutsideCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TROutsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/TR_Outside_Corner.TR_Outside_Corner'"));
	_topRightOutCorner = TROutsideCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BLOutsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/BL_Outside_Corner.BL_Outside_Corner'"));
	_bottomLeftOutCorner = BLOutsideCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BROutsideCorner(TEXT("StaticMesh'/Game/DungeonExpansion/BR_Outside_Corner.BR_Outside_Corner'"));
	_bottomRightOutCorner = BROutsideCorner.Object;


	static ConstructorHelpers::FObjectFinder<UStaticMesh> ETopWall(TEXT("StaticMesh'/Game/DungeonExpansion/East_TopWall.East_TopWall'"));
	_eastTopWall = ETopWall.Object;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NTopWall(TEXT("StaticMesh'/Game/DungeonExpansion/North_TopWall.North_TopWall'"));
	_northTopWall = NTopWall.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WTopWall(TEXT("StaticMesh'/Game/DungeonExpansion/West_TopWall.West_TopWall'"));
	_westTopWall = WTopWall.Object;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> STopWall(TEXT("StaticMesh'/Game/DungeonExpansion/South_TopWall.South_TopWall'"));
	_southTopWall = STopWall.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TLTopCorner(TEXT("StaticMesh'/Game/DungeonExpansion/TL_TopCorner.TL_TopCorner'"));
	_topLeftTopCorner = TLTopCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TRTopCorner(TEXT("StaticMesh'/Game/DungeonExpansion/TR_TopCorner.TR_TopCorner'"));
	_topRightTopCorner = TRTopCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BLTopCorner(TEXT("StaticMesh'/Game/DungeonExpansion/BL_TopCorner.BL_TopCorner'"));
	_bottomLeftTopCorner = BLTopCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BRTopCorner(TEXT("StaticMesh'/Game/DungeonExpansion/BR_TopCorner.BR_TopCorner'"));
	_bottomRightTopCorner = BRTopCorner.Object;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TopFloor(TEXT("StaticMesh'/Game/DungeonExpansion/TopFloor.TopFloor'"));
	_topFloor = TopFloor.Object;
	
	/*/ Load corner meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BottomLeftCorner(TEXT("StaticMesh'/Game/DungeonExpansion/bottom_left_wall_corner.bottom_left_wall_corner'"));
	_bottomLeftCorner = BottomLeftCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TopLeftCorner(TEXT("StaticMesh'/Game/DungeonExpansion/top_left_wall_corner.top_left_wall_corner'"));
	_topLeftCorner = TopLeftCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TopRightCorner(TEXT("StaticMesh'/Game/DungeonExpansion/to_right_wall_corner.to_right_wall_corner'"));
	_topRightCorner = TopRightCorner.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BottomRightCorner(TEXT("StaticMesh'/Game/DungeonExpansion/bottom_right_wall_corner.bottom_right_wall_corner'"));
	_bottomRightCorner = BottomRightCorner.Object;*/

/*	// Load doorway meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NorthDoorway(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/NorthDoorway.NorthDoorway'"));
	_northDoorway = NorthDoorway.Object;	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EastDoorway(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/EastDoorway.EastDoorway'"));
	_eastDoorway = EastDoorway.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SouthDoorway(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/SouthDoorway.SouthDoorway'"));
	_southDoorway = SouthDoorway.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WestDoorway(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/WestDoorway.WestDoorway'"));
	_westDoorway = WestDoorway.Object;

	// Load Strut Meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TopLeftStrut(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/TopLeftStrut.TopLeftStrut'"));
	_topLeftStrut = TopLeftStrut.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TopRightStrut(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/TopRightStrut.TopRightStrut'"));
	_topRightStrut = TopRightStrut.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BottomLeftStrut(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/BottomLeftStrut.BottomLeftStrut'"));
	_bottomLeftStrut = BottomLeftStrut.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BottomRightStrut(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/BottomRightStrut.BottomRightStrut'"));
	_bottomRightStrut = BottomRightStrut.Object;

	// Load Corridor Meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> VerticalCorridor(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/VerticalCorridor.VerticalCorridor'"));
	_verticalCorridor = VerticalCorridor.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HorizontalCorridor(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/HorizontalCorridor.HorizontalCorridor'"));
	_horizontalCorridor = HorizontalCorridor.Object;

	// Dead End Meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> NorthDeadEnd(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/NorthDeadEnd.NorthDeadEnd'"));
	_northDeadEnd = NorthDeadEnd.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EastDeadEnd(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/EastDeadEnd.EastDeadEnd'"));
	_eastDeadEnd = EastDeadEnd.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SouthDeadEnd(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/SouthDeadEnd.SouthDeadEnd'"));
	_southDeadEnd = SouthDeadEnd.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WestDeadEnd(TEXT("StaticMesh'/Game/WaveFunctionCollapse/Tileset31_01_25/WestDeadEnd.WestDeadEnd'"));
	_westDeadEnd = WestDeadEnd.Object;*/
}

// Called when the game starts or when spawned
void AWFC_Grid::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWFC_Grid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWFC_Grid::InitGrid(int rows, int columns, FVector location)
{
	ROW_SIZE	= rows;						// Set row size
	COLUMN_SIZE = columns;					// Set column size
	GRID_SIZE	= ROW_SIZE * COLUMN_SIZE;	// Determine grid size

	_tileArray.SetNum(GRID_SIZE);			// Set size of tile array to size of grid

	_currentLocation	= location;			// Set current location of the grid
	_baseLocation		= location;			// Set starting location to be used as a reference

	CreateGrid();							// Create grid
}
 
/// 
/// Spawn tiles within game world and store them in array to create grid 
/// 
void AWFC_Grid::CreateGrid()
{
	for (int i = 0; i < GRID_SIZE; i++)		// Loop for size of grid
	{
		if (GetWorld())						// If game world exists
		{
			AWFC_Tile* newTile = GetWorld()->SpawnActor<AWFC_Tile>(AWFC_Tile::StaticClass(), FTransform(FRotator(0.0f, 0.0f, 0.0f), _currentLocation)); // Spawn new tile
			if (newTile)					// If tile has been spawned
			{
				_tileArray[i] = newTile; 	// Store tile in tile array
			}
		}

		_currentLocation.X += TILE_SIZE.X;	// Move location for new tile along row

		if ((i + 1) % ROW_SIZE == 0)		// Check if index is divisible by row size with no remainder (new row)
		{
			_currentLocation.X = _baseLocation.X;	// Reset X position to start of new row
			_currentLocation.Y += TILE_SIZE.Y;		// Move Y position up for the new row
		}
	}
}

///
/// Set the mesh to the assigned tile type for an index
/// 
/// index - index of tile within grid
/// tile - assigned tile type for the index
void AWFC_Grid::SetTileMesh(int index, int tile)
{
	// Set cell mesh based on the assigned tile index
	switch (tile)
	{
	case -1:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(NULL);
		break;
	case Floor:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_floor);
		break;
	case EastWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_eastWall);
		break;
	case NorthWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_northWall);
		break;
	case WestWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_westWall);
		break;
	case SouthWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_southWall);
		break;
	case TLInsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_topLeftCorner);
		break;
	case TRInsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_topRightCorner);
		break;
	case BLInsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_bottomLeftCorner);
		break;
	case BRInsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_bottomRightCorner);
		break;
	case TLOutsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_topLeftOutCorner);
		break;
	case TROutsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_topRightOutCorner);
		break;
	case BLOutsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_bottomLeftOutCorner);
		break;
	case BROutsideCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_bottomRightOutCorner);
		break;
	case EastTopWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_eastTopWall);
		break;
	case NorthTopWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_northTopWall);
		break;
	case WestTopWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_westTopWall);
		break;
	case SouthTopWall:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_southTopWall);
		break;
	case TLTopCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_topLeftTopCorner);
		break;
	case TRTopCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_topRightTopCorner);
		break;
	case BLTopCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_bottomLeftTopCorner);
		break;
	case BRTopCorner:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_bottomRightTopCorner);
		break;
	case TopFloor:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_topFloor);
		break;
	case Blank:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(NULL);
		break;
	case Path:
		_tileArray[index]->GetTileMesh()->SetStaticMesh(_floor);
		break;
	default:
		break;
	}
}

/// 
/// Set all tile meshes in grid to -1
/// 
void AWFC_Grid::DiscardMeshes()
{
	for (int i = 0; i < _finalTileStates.size(); i++)
		SetTileMesh(i, -1);
}

/// 
/// Destroy all tile actors associated with grid
///
void AWFC_Grid::DestroyTiles()
{
	for (auto tile : _tileArray)
	{
		tile->Destroy();
	}
}

