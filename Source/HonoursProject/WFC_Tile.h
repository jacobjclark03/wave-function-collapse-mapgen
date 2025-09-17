// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HelperClass.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "WFC_Tile.generated.h"

UCLASS()
class HONOURSPROJECT_API AWFC_Tile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWFC_Tile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Debug visualistion functions
	void DrawSpawnBox();		// Draw green box around tile	
	void DrawBombBox();			// Draw red box around tile
	void DrawColourBox();		// Draw coloured box around tile
	void EnableCollision();		// Enable tile collisions
private:
	class UStaticMeshComponent* _tileMesh;		// Tiles mesh
	UBoxComponent*				_collisionBox;	// Tile Collision Box
 
public: 
	// Getters and Setters
	UStaticMeshComponent* GetTileMesh() { return _tileMesh; };
	

};
