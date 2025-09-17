// Fill out your copyright notice in the Description page of Project Settings.


#include "DSU.h"

DSU::DSU(int n)
{
 
    _parent = new int[n];          // Initialise parent array
    _rank   = new int[n];          // Initialise rank array

	for (int i = 0; i < n; i++)    // Loop through size of array
    {
		_parent[i] = -1;		  // Set parent to -1
		_rank[i] = 1;			  // Set rank to 1
    }
}

DSU::~DSU()
{
    delete[] _parent;
    delete[] _rank;
}

/// 
/// Find root of set 
///
/// i - index of set
int DSU::Find(int i)
{
    if (_parent[i] == -1)                   // If node owns parent
        return i;                           // Return root

    return _parent[i] = Find(_parent[i]);   // Link node to its root
}

/// 
/// Merge sets
/// 
/// x - co-ord
/// y - co-ord
void DSU::Unite(int x, int y)
{
    int _s1 = Find(x);                  // Find root of first set
    int _s2 = Find(y);                  // Find root of second set

    if (_s1 != _s2)                     // Merge if the roots are in different sets
    {
        if (_rank[_s1] < _rank[_s2])        // Attach smaller tree to larger tree
            _parent[_s1] = _s2;
        else if (_rank[_s1] > _rank[_s2]) 
            _parent[_s2] = _s1;
        else                                // Ranks are equal, attach to each other
        {
			_parent[_s2] = _s1;             // Attach s2 to s1
			_rank[_s1] += 1;                // Increase rank of s1
        }
    }
}
