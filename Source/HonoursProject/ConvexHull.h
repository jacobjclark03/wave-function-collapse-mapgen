// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include "CoreMinimal.h"

/**
 * 
 */
class HONOURSPROJECT_API ConvexHull
{
public:
	ConvexHull();
	~ConvexHull();

	int Quad(std::pair<int, int> p);

	int Orientation(std::pair<int, int> a, std::pair<int, int> b, std::pair<int, int> c);

	bool Compare(std::pair<int, int> p1, std::pair<int, int> q1);

	std::vector<std::pair<int, int>> Merger(std::vector<std::pair<int, int>> a, std::vector<std::pair<int, int>> b);

	std::vector<std::pair<int, int>> BruteHull(std::vector<std::pair<int, int>> a);

	std::vector<std::pair<int, int>> Divide(std::vector<std::pair<int, int>> a);

private:
	std::pair<int, int> _mid;
};


