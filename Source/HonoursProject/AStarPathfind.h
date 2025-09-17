// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include <vector>
#include <array>

struct Node
{
	int		_y;			// Node X location
	int		_x;			// Node Y location
	int		_parentX;	// Parent Node X location
	int		_parentY;	// Parent Node Y location
	float	_gCost;		// Cost to reach node from start
	float	_hCost;		// Cost from node to end
	float	_fCost;		// Total cost of node
	int		_tileType;	// Tile type of node
};

// Compares cost of passed Nodes
inline bool operator < (const Node& lhs, const Node& rhs)
{
	return lhs._fCost < rhs._fCost;
}

class HONOURSPROJECT_API AStarPathfind
{
public:
	AStarPathfind();
	~AStarPathfind();

	void				Init(int rows, int columns, int xtile, int ytile, bool flag);// Initialise pathfinding algorithm

	bool				IsValid(int x, int y, int tileType);						// Validates tile type at location
	bool				IsDestination(int x, int y, Node dest);						// If current tile is destination searched for

	float				CalculateH(int x, int y, Node dest);						// Calculate pathfind heuristicc

	std::vector<Node>	AStar(Node player, Node dest, std::vector<int>& WFCoutput);	// Main pathfinding algorithm
	std::vector<Node>	MakePath(std::vector<std::vector<Node>>& map, Node dest);	// Create path from 
private:
	int					ROW_SIZE, COLUMN_SIZE;				// Dimensions of grid

	int					X_TILE_SIZE, Y_TILE_SIZE;			// Size of a tile 
	bool				_pathCreation;						// Path creator flag

	std::vector<int>	_pathIndexes;						// Created path
		

	std::vector<int>	_dx = { 1, 0, -1, 0 };				// Change of direction in x plane
	std::vector<int>	_dy = { 0, 1, 0, -1 };				// Change of direction in y plane
public:
	// Getter and Setters
	std::vector<int>	GetPathIndexes() { return _pathIndexes; };

};
