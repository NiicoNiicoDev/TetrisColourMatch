// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHandler.h"
#include "BasePiece.h"
#include "Math/UnrealMathUtility.h"
#include "PieceController.h"

// Sets default values
AGameHandler::AGameHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameHandler::BeginPlay()
{
	Super::BeginPlay();
	pieceController = Cast<APieceController>(UGameplayStatics::GetActorOfClass(GetWorld(), APieceController::StaticClass()));
	//GenerateNewBlock();
	CreateTetrominoes();

	GenerateNewBlock();
}

// Called every frame
void AGameHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

ABasePiece** AGameHandler::SpawnTetrominoe() 
{
	tetromino tetrominoType = tetrominoes[FMath::RandRange(0, 7)];

	ABasePiece** tetrominoe = new ABasePiece*[4];

	for (int i = 0; i < 4; i++)
	{
		AActor* actor = GWorld->SpawnActor<ABasePiece>(ABasePiece::StaticClass());
		ABasePiece* Piece = Cast<ABasePiece>(actor);

		Piece->currentRow = tetrominoType.RowIndex[i];
		Piece->currentColumn = tetrominoType.ColumnIndex[i];

		tetrominoe[i] = Piece;

	}

	return tetrominoe;
}

void AGameHandler::GenerateNewBlock()
{
	pieceController->block = nullptr;
	ABasePiece** blocks = SpawnTetrominoe();
	
	pieceController->block = blocks;
}

void AGameHandler::CheckFullRow()
{
	for (int i = 0; i < 20; i++)
	{
		int filledSpaces = 0;

		for (int j = 0; j < 10; j++)
		{
			
			if (playField[i][j].isPlaced)
			{
				filledSpaces++;
			}

		}

		if (filledSpaces == 10)
		{
			//Remove row
			for (int k = 0; k < 10; k++)
			{
				if (playField[i][k].piece != nullptr) 
				{
					playField[i][k].piece->Destroy();

					playField[i][k].isPlaced = false;
					playField[i][k].piece = nullptr;
				}
			}

			MovePlayfield();
		}

		UE_LOG(LogTemp, Log, TEXT("Row %d -> Current Filled Spaces: %d"), i, filledSpaces);
	}

}

void AGameHandler::MovePlayfield() 
{
	for (int i = 19; i > 0; i--)
	{
		for (int j = 0; j < 10; j++)
		{
			playField[i][j].isPlaced = false;

			if (playField[i][j].piece != nullptr)
			{
				FVector targetLocation = playField[i][j].piece->GetActorLocation();
				targetLocation.Z -= 100;
				playField[i][j].piece->SetActorLocation(targetLocation);

				playField[i][j].piece->currentRow += 1;
				
				if (i < 18) 
				{
					playField[i + 1][j].piece = playField[i][j].piece;
				}

				playField[i][j].piece = nullptr;

				playField[i + 1][j].isPlaced = true;
				playField[i][j].isPlaced = false;
			}
		}
	}

	//VerifyPlayfield();
}

void AGameHandler::VerifyPlayfield() 
{
	for (int i = 0; i < 20; i++)
	{
		for (int j = 1; j < 10; j++)
		{
			if (playField[i][j].piece != nullptr) 
			{
				if (!playField[i][j].piece->isPlaced)
					playField[i][j].isPlaced = true;
				
				if (playField[i][j].piece->GetName().IsEmpty())
				{
					UE_LOG(LogTemp, Log, TEXT("target piece pos: %s"), *playField[i][j].piece->GetActorLocation().ToString());
					playField[i][j].piece = nullptr;
				}
			}
		}
	}
}

void AGameHandler::SetPositionActive(ABasePiece* piece, int row, int column)
{
	playField[row][column].piece = piece;
	playField[row][column].isPlaced = true;

	placePiecesThisPiece += 1;

	if (placePiecesThisPiece == 4) 
	{
		UE_LOG(LogTemp, Log, TEXT("Pieces placed this piece: %d"), placePiecesThisPiece);
		pieceController->block = nullptr;
		GenerateNewBlock();
		placePiecesThisPiece = 0;
	}
}

void AGameHandler::CreateTetrominoes() 
{

#pragma region L_Piece
	tetrominoes[0].ColumnIndex[0] = 4;
	tetrominoes[0].RowIndex[0] = 1;

	tetrominoes[0].ColumnIndex[1] = 5;
	tetrominoes[0].RowIndex[1] = 1;

	tetrominoes[0].ColumnIndex[2] = 6;
	tetrominoes[0].RowIndex[2] = 1;

	tetrominoes[0].ColumnIndex[3] = 6;
	tetrominoes[0].RowIndex[3] = 0;
#pragma endregion L_Piece

#pragma region Z_Piece
	tetrominoes[1].ColumnIndex[0] = 4;
	tetrominoes[1].RowIndex[0] = 0;

	tetrominoes[1].ColumnIndex[1] = 5;
	tetrominoes[1].RowIndex[1] = 0;

	tetrominoes[1].ColumnIndex[2] = 5;
	tetrominoes[1].RowIndex[2] = 1;

	tetrominoes[1].ColumnIndex[3] = 6;
	tetrominoes[1].RowIndex[3] = 1;
#pragma endregion Z_Piece

#pragma region S_Piece
	tetrominoes[2].ColumnIndex[0] = 4;
	tetrominoes[2].RowIndex[0] = 1;

	tetrominoes[2].ColumnIndex[1] = 5;
	tetrominoes[2].RowIndex[1] = 1;

	tetrominoes[2].ColumnIndex[2] = 5;
	tetrominoes[2].RowIndex[2] = 0;

	tetrominoes[2].ColumnIndex[3] = 6;
	tetrominoes[2].RowIndex[3] = 0;
#pragma endregion S_Piece

#pragma region J_Piece
	tetrominoes[3].ColumnIndex[0] = 4;
	tetrominoes[3].RowIndex[0] = 0;

	tetrominoes[3].ColumnIndex[1] = 4;
	tetrominoes[3].RowIndex[1] = 1;

	tetrominoes[3].ColumnIndex[2] = 5;
	tetrominoes[3].RowIndex[2] = 1;

	tetrominoes[3].ColumnIndex[3] = 6;
	tetrominoes[3].RowIndex[3] = 1;
#pragma endregion J_Piece

#pragma region T_Piece
	tetrominoes[4].ColumnIndex[0] = 4;
	tetrominoes[4].RowIndex[0] = 1;

	tetrominoes[4].ColumnIndex[1] = 5;
	tetrominoes[4].RowIndex[1] = 1;

	tetrominoes[4].ColumnIndex[2] = 5;
	tetrominoes[4].RowIndex[2] = 0;

	tetrominoes[4].ColumnIndex[3] = 6;
	tetrominoes[4].RowIndex[3] = 1;
#pragma endregion T_Piece

#pragma region Square_Piece
	tetrominoes[5].ColumnIndex[0] = 5;
	tetrominoes[5].RowIndex[0] = 0;

	tetrominoes[5].ColumnIndex[1] = 6;
	tetrominoes[5].RowIndex[1] = 0;

	tetrominoes[5].ColumnIndex[2] = 5;
	tetrominoes[5].RowIndex[2] = 1;

	tetrominoes[5].ColumnIndex[3] = 6;
	tetrominoes[5].RowIndex[3] = 1;
#pragma endregion Square_Piece

#pragma region Line_Piece
	tetrominoes[6].ColumnIndex[0] = 4;
	tetrominoes[6].RowIndex[0] = 0;

	tetrominoes[6].ColumnIndex[1] = 5;
	tetrominoes[6].RowIndex[1] = 0;

	tetrominoes[6].ColumnIndex[2] = 6;
	tetrominoes[6].RowIndex[2] = 0;

	tetrominoes[6].ColumnIndex[3] = 7;
	tetrominoes[6].RowIndex[3] = 0;
#pragma endregion Line_Piece
}

