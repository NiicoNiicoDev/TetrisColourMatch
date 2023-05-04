// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine.h"
#include <map>
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

	UAudioComponent* audioComponent;

	UPROPERTY(EditAnywhere)
		bool bGameOver;

	UPROPERTY(EditAnywhere)
		bool bRandomTetrominoes = true;
	
	UPROPERTY(EditAnywhere)
	bool bIsFastDrop = false;
	
	UPROPERTY(EditAnywhere)
	UMaterialInstance* ColoursArray[7];

	enum colour 
	{
		red,
		blue,
		yellow,
		green,
		pink,
		black,
		white
	};

	struct pieceContainer
	{
		ABasePiece* piece = nullptr;
		bool isPlaced = false;
		colour Colour;
	};

	struct targetLocationInArray
	{
		int8 ColumnShift;
		int8 RowShift;
	};

	struct RotationData
	{
		targetLocationInArray data[4];
	};
	
	struct tetromino
	{
		FString TetrominoName;
		int TetrominoIndex = 0;
		int32 ColumnIndex[4];
		int32 RowIndex[4];
		colour Colour;
		int currentRotation = 0;
		
		RotationData rotationData[4];

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
		int TetToSpawn = 0;

	UPROPERTY(EditAnywhere)
	float moveDownFrequency = 1.0f;

	UPROPERTY(EditAnywhere)
	uint64 currentScore = 0;
	
	UPROPERTY(EditAnywhere)
	uint32 totalLinesCleared = 0;

	UPROPERTY(EditAnywhere)
	uint8 currentLevel = 1;

	UPROPERTY(EditAnywhere)
		uint16 LinesToClearPerLevel[28];

	APieceController* pieceController;

	pieceContainer playField[20][10];

	tetromino tetrominoes[7];

	tetromino currentTetrominoe;

	UPROPERTY(EditAnywhere)
	int placePiecesThisTetrominoe = 0;

	UPROPERTY(EditAnywhere)
		AActor* scoreText;

	UPROPERTY(EditAnywhere)
		AActor* levelText;

	UTextRenderComponent* scoreTextComp;
	
	UTextRenderComponent* levelTextComp;

	void GenerateNewBlock();

	void CheckFullRow();

	void MovePlayfield(int rowsToMove, int rowIndexToMoveFrom);

	void VerifyPlayfield();

	void PlayBlockMoveSound();

	void SetPositionActive(ABasePiece* piece, int row, int column);

	void BlockShift(int columnShift, int rowShift);

	ABasePiece** SpawnTetrominoe();

	void CreateTetrominoes();

	void SetPieceRotation(int tetrominoIndex, int rotationIndex);

	bool CheckRotationLocations(targetLocationInArray locations[]);
	
	void InitalizeRotationArrays();

	void UpdateScore(int linesCleared, int multiplier);

	void IncreaseDifficulty();

	void CheckPieceDataMismatch();

	void GameOver();
		
};
