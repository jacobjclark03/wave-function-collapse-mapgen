// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <vector>
#include <sstream>
#include "CoreMinimal.h"

/**
 * 
 */


struct NeighbourChunk			// Neighbour Struct - passes information to next in line chunk to propagate 
{
	std::vector<int> _leftEdge;
	std::vector<int> _upEdge;
	std::vector<int> _rightEdge;
	std::vector<int> _downEdge;
};

struct Edge						// Edge struct - used in kruskal's 
{
	int _tile0;
	int _tile1;
	int _weight;

	Edge(int tile0, int tile1, int weight) // Constructor
	{
		_tile0 = tile0;
		_tile1 = tile1;
		_weight = weight;
	};

};


enum RoomType					// Room types - used for room identification
{
	Unassigned		= 0,
	SpawnPoint		= 1,
	BombSite		= 2,
	SiteConnector	= 3,
};

enum TileType					// Tile type - Used for propagator definition to increase clarity
{
	/*Floor = 0,
	NorthWall			= 1,
	EastWall			= 2,
	SouthWall			= 3,
	WestWall			= 4,
	TopLeftCorner		= 5,
	TopRightCorner		= 6, 
	BottomLeftCorner	= 7,
	BottomRightCorner	= 8,
	TopLeftStrut		= 9,
	TopRightStrut		= 10,
	BottomLeftStrut		= 11,
	BottomRightStrut	= 12,
	VerticalCorridor	= 13,
	HorizontalCorridor	= 14,
	NorthDeadEnd		= 15,
	EastDeadEnd			= 16,
	SouthDeadEnd		= 17,
	WestDeadEnd			= 18,
	Path				= 19*/

	Floor = 0,
	EastWall = 1,
	NorthWall = 2,
	WestWall = 3,
	SouthWall = 4,
	TLInsideCorner = 5,
	TRInsideCorner = 6,
	BLInsideCorner = 7,
	BRInsideCorner = 8,
	TLOutsideCorner = 9,
	TROutsideCorner = 10,
	BLOutsideCorner = 11,
	BROutsideCorner = 12,
	Blank = 13,
	EastTopWall = 14,
	NorthTopWall = 15,
	WestTopWall = 16,
	SouthTopWall = 17,
	TLTopCorner = 18,
	TRTopCorner = 19,
	BLTopCorner = 20,
	BRTopCorner = 21,
	TopFloor = 22,
	Path = 30
};


class HONOURSPROJECT_API HelperClass
{
public:
	HelperClass();
	~HelperClass();

	// Debug helper functions
	void DebugOutputWave(std::vector<int>& vecToPrint);


};
