// Fill out your copyright notice in the Description page of Project Settings.


#include "HelperClass.h"

HelperClass::HelperClass()
{
}

HelperClass::~HelperClass()
{}


/// 
/// Debug print wave
/// 
void HelperClass::DebugOutputWave(std::vector<int>& vecToPrint)
{
	int _gridSize = static_cast<int>(std::sqrt(vecToPrint.size()));
	for (int y = 0; y < _gridSize; y++)
	{
		FString _row;
		for (int x = 0; x < _gridSize; x++)
		{
			int _index = y * _gridSize + x;
			if (vecToPrint[_index] >= 0)
			{
				_row += FString::Printf(TEXT("%4d "), vecToPrint[_index]); 
			}
			else
			{
				int _temp = -1;

				_row += FString::Printf(TEXT("%4d "), _temp);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("%s"), *_row);
	}
}
