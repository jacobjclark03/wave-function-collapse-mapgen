// Fill out your copyright notice in the Description page of Project Settings.

// https://www.geeksforgeeks.org/kruskals-minimum-spanning-tree-algorithm-greedy-algo-2/
#include "Graph.h"
#include <algorithm>
#include "DSU.h"

Graph::Graph(int V, int rows, int columns, int chunkRows, int chunkColumns)
{
	this->_V = V;						// Set numnber of vertices in graph
	ROW_SIZE = rows;					// Set num of rows in map grid
	COLUMN_SIZE = columns;				// Set num of columns in map grid

	CHUNK_ROW_SIZE = chunkRows;			// Set num of rows for each grid in map grid cell
	CHUNK_COLUMN_SIZE = chunkColumns;	// Set num of columnns for each grid in map grid cell
}

Graph::~Graph()
{
}

/// 
/// Add edge to edge list
/// 
/// x - x co-ordinate of edge
/// y - y co-ordinate of edge
/// w - weight of the edge
void Graph::AddEdge(int x, int y, int w)
{
	_edgeList.push_back({ w, x, y });	// Add edge to list
}

/// 
/// Calculates shortest distance pair between two rooms
/// 
/// edges0 - indexes of tiles within first room
/// edges1 - indexes of tiles within second room
Edge Graph::ShortestDistancePair(const std::vector<int>& edges0, const std::vector<int>& edges1)
{
	int _minDist = INT_MAX;			// Set minimum distance to int max
	std::vector<Edge> _shortest;	// Create vector to store shortest edges
	
	for (int i = 0; i < edges0.size(); i++)					// Loop through first room's indexes
	{
		int _x0 = edges0[i] % (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate x co-ord of i-th tile in first room
		int _y0 = edges0[i] / (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate y co-ord of i-th tile in first room
	
		for (int j = 0; j < edges1.size(); j++)					// Loop through second room's indexes
		{
			int _x1 = edges1[j] % (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate x co-ord of j-th tile in second room
			int _y1 = edges1[j] / (ROW_SIZE * CHUNK_ROW_SIZE);	// Calculate y co-ord of j-th tile in second room

			int _dist = abs(_x0 - _x1) + abs(_y0 - _y1);		// Calculate the distance between i-th index and j-th index

			if (_dist < _minDist)								// If the dist between two indexes is less than the shortest distance
			{
				_minDist = _dist;								// Set shortest distance
				_shortest.clear();								// Clear shortest distance vector as new shortest distance has been found, nullifying others
				_shortest.push_back(Edge(i, j, _dist));			// Add shortest edge to vector
			}
			else if (_dist == _minDist)							// If the distance between two indexes is equal to the shortest distance
			{
				_shortest.push_back(Edge(i, j, _dist));			// Add to vector as there is more than one option for shortest distance pair
			}
		}
	}
	
	int index = _shortest.size() / 2;		// Get the middle element of shortest distance vector

	return _shortest[index];				// Return middle element of shortest pairs
}



/// 
/// Run Kruskal's algorithm to generate MST
/// 
void Graph::Kruskals()
{
	std::sort(_edgeList.begin(), _edgeList.end()); // Sprt all edges in graph based on weight

	DSU _s(_V);		// Initialise DSU to track connected componenets

	int _ans = 0;	// Track total of weights of MST
	int _count = 0; // Track number of edges added to MST

	for (auto _edge : _edgeList)	// Loop through sorted edges
	{
		int _w = _edge[0];			// Get the weight of the edge
		int _x = _edge[1];			// Get the first vertex of edge
		int _y = _edge[2];			// Get the second vertex of edge

		if (_s.Find(_x) != _s.Find(_y)) // If vertices belong to different sets
		{
			_s.Unite(_x, _y);			// Merge
			_ans += _w;					// Add the weight of edge to total
	
			_output.push_back(_edge);	// Add edge to the output
			_count++;					// Increment number of edges in MST
		}

		if (_count == _V - 1)			// If number of edges reaches the number of vertices - 1
			break;						// Break as MST has been completed
	}
}
