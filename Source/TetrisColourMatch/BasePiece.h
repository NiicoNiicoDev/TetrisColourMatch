// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Engine.h"
#include "BasePiece.generated.h"

class USceneComponent;
class APieceController;
class AGameHandler;

UCLASS()
class TETRISCOLOURMATCH_API ABasePiece : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePiece();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	int spawnRow = 0;
	int spawnColumn = 4;
	
	UPROPERTY(EditAnywhere)
	int currentRow = 0;
	UPROPERTY(EditAnywhere)
	int currentColumn = 0;
	UPROPERTY(EditAnywhere)
	bool isPlaced = false;

	bool bDoTick = false;

	float currentTime;

	bool standardMovement = true;

	AGameHandler* gameHandler;

	int CheckPieceIndex();

	void PlaceBlock();

	void DownMovement();

};
