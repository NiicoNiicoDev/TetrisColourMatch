// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine.h"
#include "GameHandler.generated.h"

class ABasePiece;
class APieceController;

UCLASS()
class TETRISCOLOURMATCH_API AGameHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameHandler();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	struct pieceContainer
	{
		ABasePiece* piece = nullptr;
		bool isPlaced = false;
	};
	
	struct tetromino
	{
		int32 ColumnIndex[4];
		int32 RowIndex[4];

		tetromino()
		{
			// Initialize the ColumnIndex and RowIndex arrays to zero
			for (int i = 0; i < 4; i++)
			{
				ColumnIndex[i] = 0;
				RowIndex[i] = 0;
			}
		}
	};

	UPROPERTY(EditAnywhere)
	float moveDownFrequency = 1.0f;

	APieceController* pieceController;

	pieceContainer playField[20][10];

	tetromino tetrominoes[7];

	UPROPERTY(EditAnywhere)
	int placePiecesThisPiece = 0;

	void GenerateNewBlock();

	void CheckFullRow();

	void MovePlayfield();

	void VerifyPlayfield();

	void SetPositionActive(ABasePiece* piece, int row, int column);

	ABasePiece** SpawnTetrominoe();

	void CreateTetrominoes();

};
