// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "Engine.h"
#include "PieceController.generated.h"


class ABasePiece;
class AGameHandler;

UCLASS()
class TETRISCOLOURMATCH_API APieceController : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APieceController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere)
		float quickDropSpeed = 0.05f;

	float previousDropSpeed = 0;
	
	void MoveRight();
	void MoveLeft();
	void RotateRight();
	void RotateLeft();

	void QuickDropPressed();
	void QuickDropReleased();
	
	ABasePiece** block = new ABasePiece*[4];

	AGameHandler* gameHandler;
};
