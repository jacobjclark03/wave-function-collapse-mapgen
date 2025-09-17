// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "HelperClass.h"
#include "Room.generated.h"

UCLASS()
class HONOURSPROJECT_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void CalculateArea();							// Calculate area of room
	void CalculateCentre(int numRows, int numColumns);
private:
	int				 _roomID;						// Unique identifier for each room identified
	int				 _rowCount;						// Count of rows in room
	int				 _columnCount;					// Count of columns in room
	int				 _corridorCount;				// Count of corridors in room
	int				 _roomCentreIndex;				// Center index of room

	float			 _roomDensity;					// Room Density
	float			 _roomDimensionRatio;			// Ratio of columns to rows or vice versa
	float			 _roomArea;						// Area of room
	float			 _bSiteScore;					// Score value of room's suitability to be a bomb site
	float			 _spawnScore;					// Score value of room's suitability to be a spawn site
	float			 _connectorScore;				// Score value of room's suitability to be a connector site

	bool			 _combinable;					// Flag to mark if room can be combined

	RoomType		 _roomType;						// Type of room
	
	const float		 _tileArea = 210.f * 210.f;		// Area of each tile
	std::vector<int> _tileIndexes;					// Vector contains each index of every tile assigned to the room
	std::vector<int> _spawnToSiteDist;				// Vector contains distance from spawn site to bomb site
	std::vector<int> _connectedRooms;				// Vector contains connected rooms			
public:
	// Setter and getter functions
	void SetRoomID(int ID)								{ _roomID = ID; };
	void SetRowCount(int rows)							{ _rowCount = rows; };
	void PushTileIndex(int index)						{ _tileIndexes.push_back(index); };
	void SetRoomType(RoomType type)						{ _roomType = type; };
	void SetColumnCount(int columns)					{ _columnCount = columns; };
	void SetBombSiteScore(float score)					{ _bSiteScore = score; };
	void SetSpawnSiteScore(float score)					{ _spawnScore = score; };
	void SetConnectorScore(float score)					{ _connectorScore = score; };
	void SetCorridorCount(int corridors)				{ _corridorCount = corridors; };
	void SetTileIndexes(const std::vector<int>& tiles)	{ _tileIndexes = tiles; };
	void PushSiteDistance(int dist)						{ _spawnToSiteDist.push_back(dist); };
	void SetCentreIndex(int centre)						{ _roomCentreIndex = centre; };
	void SetRoomDensity(float density)					{ _roomDensity = density; };
	void SetRoomDimensionRatio(float ratio)				{ _roomDimensionRatio = ratio; };
	void SetCombinable(bool flag)						{ _combinable = flag; };
	void PushConnectedRoom(int room)					{ _connectedRooms.push_back(room); };

	int GetRoomID()										{ return _roomID; };
	int GetRowCount()									{ return _rowCount; };
	int GetColumnCount()								{ return _columnCount; };
	int GetCorridorCount()								{ return _corridorCount; };
	int GetCentreIndex()								{ return _roomCentreIndex; };

	float GetRoomArea()									{ return _roomArea; }
	float GetBombSiteScore()							{ return _bSiteScore; };
	float GetSpawnSiteScore()							{ return _spawnScore; };
	float GetConnectorScore()							{ return _connectorScore; };
	float GetRoomDensity()								{ return _roomDensity; };
	float GetRoomDimensionRatio()						{ return _roomDimensionRatio; };

	bool GetCombinable()								{ return _combinable; };

	RoomType GetRoomType()				 				{ return _roomType; };

	std::vector<int>& GetTileIndexes()					{ return _tileIndexes; };
	std::vector<int>& GetSpawnToSiteDist()				{ return _spawnToSiteDist; };
	std::vector<int>& GetConnectedRooms()				{ return _connectedRooms; };

};
