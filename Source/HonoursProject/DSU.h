// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class HONOURSPROJECT_API DSU
{
public:
	DSU(int n);						// Constructor
	~DSU();					
	
	int		Find(int i);			// Find the root of a set
	void	Unite(int x, int y);	// Merge Sets


	int* _parent;					// Stores parent of each element in set
	int* _rank;						// Stores rank of element
};
