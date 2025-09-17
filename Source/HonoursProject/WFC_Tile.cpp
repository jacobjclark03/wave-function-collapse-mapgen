// Fill out your copyright notice in the Description page of Project Settings.


#include "WFC_Tile.h"

// Sets default values
AWFC_Tile::AWFC_Tile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Create a default scene component and set it as the root
    USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
    RootComponent = SceneComponent;

    // Create tile mesh and set up attachment
    _tileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Mesh"));
    _tileMesh->SetupAttachment(RootComponent);  
    _tileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    

    // Creat collision box and set collision types
    _collisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
    _collisionBox->SetupAttachment(RootComponent);
    _collisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    _collisionBox->SetBoxExtent(FVector(105.f, 105.f, 60.f));
    _collisionBox->AddRelativeLocation(FVector(105.f, 105.f, 0.0f));
 
}

// Called when the game starts or when spawned
void AWFC_Tile::BeginPlay()
{
	Super::BeginPlay();
    
}

/// 
/// Debug Visualer
/// 
void AWFC_Tile::DrawBombBox()
{
    DrawDebugBox(GetWorld(), _collisionBox->GetComponentLocation(), _collisionBox->GetScaledBoxExtent(), FColor::Red, false, 10.0f, 10.0f);
}

/// 
/// Debug Visualer
/// 
void AWFC_Tile::DrawColourBox()
{
    DrawDebugBox(GetWorld(), _collisionBox->GetComponentLocation(), _collisionBox->GetScaledBoxExtent(), FColor::Blue, true, 2.0f,10.f);
}

/// 
/// Debug Visualer
/// 
void AWFC_Tile::DrawSpawnBox()
{
    DrawDebugBox(GetWorld(), _collisionBox->GetComponentLocation(), _collisionBox->GetScaledBoxExtent(), FColor::Green, false, 10.0f);
}

/// 
/// Enable collision for tile to allow line trace
/// 
void AWFC_Tile::EnableCollision()
{
    _collisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    _collisionBox->SetCollisionObjectType(ECC_WorldDynamic);
    _collisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); 
    _collisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    _collisionBox->SetGenerateOverlapEvents(true);
}


// Called every frame
void AWFC_Tile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

            

