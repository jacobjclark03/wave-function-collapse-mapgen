// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HelperClass.h"
#include <vector>

class HONOURSPROJECT_API Graph
{

public:
	Graph(int V, int rows, int columns, int chunkRows, int chunkColumns);						// Constructor
	~Graph();																					// Destructor									

	void Kruskals();																			// Run kruskal's algorithm and generates MST
	void AddEdge(int x, int y, int w);															// Add edges to graph
	Edge ShortestDistancePair(const std::vector<int>& edges0, const std::vector<int>& edges1);	// Calculates the shortest distance pair between two rooms
private:
	int								_V;									// Represents the number of vertices in graph
	int								ROW_SIZE, COLUMN_SIZE;				// Dimensions of grid of grids 
	int								CHUNK_ROW_SIZE, CHUNK_COLUMN_SIZE;	// Dimensions of each grid

	std::vector<std::vector<int>>	_output;							// Stores output of algorithm
	std::vector<std::vector<int>>	_edgeList;							// Stores the edges in the graph
public:
	// Getters and setters
	std::vector<std::vector<int>>& GetOutput()		{ return _output; };
	std::vector<std::vector<int>>& GetEdgeList()	{ return _edgeList; };
	
};
