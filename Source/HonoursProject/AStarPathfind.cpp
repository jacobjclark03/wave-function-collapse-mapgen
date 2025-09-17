// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarPathfind.h"
#include <stack>
#include <cfloat> 
#include <cmath>

// Code from link below 
// https://dev.to/jansonsa/a-star-a-path-finding-c-4a4h

AStarPathfind::AStarPathfind()
{
}

AStarPathfind::~AStarPathfind()
{
}

/// 
/// Initialise pathfinding 
/// 
/// rows	- number of rows in grid
/// columns - number of columns in grid
/// xtile	- size of tile in x direction
/// ytile	- size of tile in y direction
void AStarPathfind::Init(int rows, int columns, int xtile, int ytile, bool pathCreationFlag)
{
	ROW_SIZE = rows;						// Set grid dimensions
	COLUMN_SIZE = columns;					// Set grid dimensions

	X_TILE_SIZE = xtile;					// Set tile size in x direction
	Y_TILE_SIZE = ytile;					// Set tile size in y direction

	_pathCreation = pathCreationFlag;		// Set path creation flag
}


/// 
/// Determine whether the observed node is a valid tile for the algorithm to traverse
/// 
/// x - x coordinate of node
/// y - y coordinate of node
/// tileType - assigned type of tile at node
bool AStarPathfind::IsValid(int x, int y, int tileType)
{
	if (x < 0 || y < 0 || x >= ROW_SIZE || y >= COLUMN_SIZE)	// If node is outside grid return invalid node
		return false;
	

	if (_pathCreation)
	{
		if (tileType == -1)											// If tile is traversable node return valid node
			return true;
		else
			return false;											// Else tile is invalid and non-traversable
	}
	else
	{
		if (tileType == 0 || tileType == 19)						// If tile is traversable node return valid node
			return true;
		else
			return false;											// Else tile is invalid and non-traversable
	}

}

/// 
/// Check if currently observed node is the final destination algorithm is searching for
/// 
/// x - x coordinate of node
/// y - y coordinate of node
/// dest - Destination node of pathfinding algorithm
bool AStarPathfind::IsDestination(int x, int y, Node dest)
{
	if (x == dest._x && y == dest._y)		// Check if current node is destination node
		return true;						// Return true, path has been found

	return false;							// Return false, current node is not the destination node
}

/// 
/// Calculate heuristic
/// 
/// x - x coordinate of node
/// y - y coordinate of node
/// dest - Destination node of pathfinding algorithm
float AStarPathfind::CalculateH(int x, int y, Node dest)
{
	//float H = (sqrt((x - dest.x) * (x - dest.x) + (y - dest.y) * (y - dest.y)));	// Euclidean distance - general distance 
	float H = abs(x - dest._x) + abs(y - dest._y);									// Manhatten distance - better for grids
	return H;																		// Return heuristic
}

/// 
/// A star pathfinding algorithm
/// 
/// start - beginning of path
/// dest - Destination node of pathfinding algorithm
/// WFCoutput - Grid that algorithm will traverse
std::vector<Node> AStarPathfind::AStar(Node start, Node dest, std::vector<int>& WFCoutput)
{
	std::vector<Node> _empty;

	if (!IsValid(dest._x, dest._y, dest._tileType))		// If destination is in valid return empty path
		return _empty;
	

	if (IsDestination(start._x, start._y, dest))		// If start is already at the destination node return empty path
		return _empty;
	
	
	std::vector<std::vector<bool>> _closedList(ROW_SIZE, std::vector<bool>(COLUMN_SIZE, false)); // Tracks tiles that have already been visited
	
	std::vector<std::vector<Node>> _allMap(ROW_SIZE, std::vector<Node>(COLUMN_SIZE));			// Node map
	
	for (int x = 0; x < ROW_SIZE; x++)									// Loop through rows					
	{					
		for (int y = 0; y < COLUMN_SIZE; y++)							// Loop through columns
		{
			_allMap[x][y]._fCost = FLT_MAX;								// Set fCost to max float value
			_allMap[x][y]._gCost = FLT_MAX;								// Set gCost to max float value
			_allMap[x][y]._hCost = FLT_MAX;								// Set hCost to max float value
			_allMap[x][y]._parentX = -1;								// Set parent node x co-ord 
			_allMap[x][y]._parentY = -1;								// Set parent node y co-ord 
			_allMap[x][y]._x = x;										// Set x position of node
			_allMap[x][y]._y = y;										// Set y position of node
			if (!(y * ROW_SIZE + x >= WFCoutput.size()))				// Calculate index and check if it is valid
				_allMap[x][y]._tileType = WFCoutput[y * ROW_SIZE + x];	// Set tile type for current node
		}
	}

	// Initialise Start node
	int _x = start._x;
	int _y = start._y;
	_allMap[_x][_y]._fCost = 0.0;
	_allMap[_x][_y]._gCost = 0.0;
	_allMap[_x][_y]._hCost = 0.0;
	_allMap[_x][_y]._parentX = _x;
	_allMap[_x][_y]._parentY = _y;
	_allMap[_x][_y]._tileType = start._tileType;


	std::vector<Node> _openList;				// Create vector of nodes yet to be explored
	_openList.emplace_back(_allMap[_x][_y]);	// Place start node at back of vector

	while (!_openList.empty())					// While there are still nodes to explore
	{
		Node _node;							
		auto _itNode = _openList.begin();		// Initilaise iterator to first element in open list
		float _maxF = FLT_MAX;					// Create max float value

		for (auto it = _openList.begin(); it != _openList.end(); ++it)	// Loop through openlist using iterator
		{
			if (it->_fCost < _maxF)	// If current cost of node is less than max cost value
			{
				_maxF = it->_fCost;	// Set new max cost value
				_itNode = it;		// Set iterator to smallest cost node
			}
		}

		_node = *_itNode;				// Set smallest fCost node
		_openList.erase(_itNode);		// Erase lowest cost node from open list

		_x = _node._x;					// Get x co-ord from lowest cost node
		_y = _node._y;					// Get y co-ord from lowest cost node
		_closedList[_x][_y] = true;		// Set node to true in closedList meaning it has been visited by algorithm

		// Pathfinding using euclidean distance - commented out as not in use but could still provide purpose
		/*for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -1; dy <= 1; dy++) 
			{
				if (dx == 0 && dy == 0) continue;
				int newX = x + dx, newY = y + dy;
				if (newX < 0 || newX >= ROW_SIZE || newY < 0 || newY >= COLUMN_SIZE)
				{
					UE_LOG(LogTemp, Log, TEXT("Invalid new coords"));
					continue;
				}
				if (IsValid(newX, newY, allMap[newX][newY].tileType)) 
				{
					if (IsDestination(newX, newY, dest)) 
					{
						allMap[newX][newY].parentX = x;
						allMap[newX][newY].parentY = y;
						return MakePath(allMap, dest);
					}
					if (!closedList[newX][newY]) 
					{
						double gNew = node.gCost + 1.0;
						double hNew = CalculateH(newX, newY, dest);
						double fNew = gNew + hNew;

						if (allMap[newX][newY].fCost == FLT_MAX || allMap[newX][newY].fCost > fNew) 
						{
							allMap[newX][newY].fCost = fNew;
							allMap[newX][newY].gCost = gNew;
							allMap[newX][newY].hCost = hNew;
							allMap[newX][newY].parentX = x;
							allMap[newX][newY].parentY = y;
							openList.push_back(allMap[newX][newY]);
						}
					}
				}
			}
		}*/

		// Pathfinding for grid system using manhatten distance
		for (int d = 0; d < 4; d++)			// Loop for each direction
		{
			int _newX = _x + _dx[d];			// Calculate new x co-ord
			int _newY = _y + _dy[d];			// Calculate new y co-ord

			if (_newX < 0 || _newX >= ROW_SIZE || _newY < 0 || _newY >= COLUMN_SIZE) // If new co-ordinates fall outside grid
				continue;															 // move onto next direction

			if (IsValid(_newX, _newY, _allMap[_newX][_newY]._tileType))	// If new node is traversable by A star
			{
				
				if (IsDestination(_newX, _newY, dest))					// Check if new node is destination
				{
					_allMap[_newX][_newY]._parentX = _x;				// Assign new node's parent x co-ord
					_allMap[_newX][_newY]._parentY = _y;				// Assign new node's parent y co-ord
					return MakePath(_allMap, dest);						// Return path from start to end
				}
				if (!_closedList[_newX][_newY])							// Check if new node has already been observed
				{
					double _gNew = _node._gCost + 1.0;					// Calculate new gCost value for new node
					double _hNew = CalculateH(_newX, _newY, dest);		// Calculate new hCost value for new node
					double _fNew = _gNew + _hNew;						// Calculate new fCost value for new node

					if (_allMap[_newX][_newY]._fCost == FLT_MAX || _allMap[_newX][_newY]._fCost > _fNew) // If new node is unobserved or new fCost is a better pathing option
					{
						// Update new node with costs & parent co-ords 
						_allMap[_newX][_newY]._fCost = _fNew;
						_allMap[_newX][_newY]._gCost = _gNew;
						_allMap[_newX][_newY]._hCost = _hNew;
						_allMap[_newX][_newY]._parentX = _x;
						_allMap[_newX][_newY]._parentY = _y;
						if (!(_newY * ROW_SIZE + _newX >= WFCoutput.size()))						// Calculate index and check if it is valid
							_allMap[_newX][_newY]._tileType = WFCoutput[_newY * ROW_SIZE + _newX];	// Set tile type for new node
						_openList.push_back(_allMap[_newX][_newY]);									// Push new node to open list
					}
				}
			}
		}
	}

    return _empty;	// Return empty vector of nodes as no path has been found
}

/// 
/// Make path of nodes from A star algorithm
/// 
/// map - map of nodes within the grid
/// dest - the final destination node
std::vector<Node> AStarPathfind::MakePath(std::vector<std::vector<Node>>& map, Node dest)
{
	int _x = dest._x;	// Destination x co-ord
	int _y = dest._y;	// Destination y co-ord

	std::stack<Node>	_path;			// Stack stores node as path is created
	std::vector<Node>	_usablePath;	// Vector will contain the final path

	// Loop until destination node is reached

	if (_pathCreation)
	{
		while (!(map[_x][_y]._parentX == _x && map[_x][_y]._parentY == _y) && map[_x][_y]._x > -1 && map[_x][_y]._y > -1)
		{
		
			_path.push(map[_x][_y]);			// push node to path
			_x = map[_x][_y]._parentX;			// Set x co-ord to parent of current node
			_y = map[_x][_y]._parentY;			// Set y co-ord to parent of current node
		}

	}
	else
	{
		while (!(map[_x][_y]._parentX == _x && map[_x][_y]._parentY == _y) && map[_x][_y]._x != -1 && map[_x][_y]._y != -1)
		{
			_path.push(map[_x][_y]);			// push node to path
			_x = map[_x][_y]._parentX;			// Set x co-ord to parent of current node
			_y = map[_x][_y]._parentY;			// Set y co-ord to parent of current node
		}

	}

	_path.push(map[_x][_y]);	// Push destination node to path after rest of path has been found

	// Reconstruct path from start to destination
	while (!_path.empty())		
	{
		Node _top = _path.top();		// Retrieve top node
		_path.pop();					// Remove top node from path
		_usablePath.emplace_back(_top);	// Emplace top node into the final path vector
		_pathIndexes.push_back(_top._y * ROW_SIZE + _top._x);	// Push index of node to path indexes vector
	}

	return _usablePath; // Return final path 
}








