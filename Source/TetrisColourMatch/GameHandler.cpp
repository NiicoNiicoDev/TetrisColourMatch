// Fill out your copyright notice in the Description page of Project Settings.

//NOTES
// BOTTOM LEFT CORNER OF PLAYFIELD IS [19][0]

#include "GameHandler.h"
#include "BasePiece.h"
#include "Math/UnrealMathUtility.h"
#include "PieceController.h"
#include <iterator>
#include <string>


// Sets default values
AGameHandler::AGameHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//playFieldVisual.SetNum(200);
	InitalizeRotationArrays();
}

// Called when the game starts or when spawned
void AGameHandler::BeginPlay()
{
	Super::BeginPlay();
	pieceController = Cast<APieceController>(UGameplayStatics::GetActorOfClass(GetWorld(), APieceController::StaticClass()));

	audioComponent = FindComponentByClass<UAudioComponent>();

	if (audioComponent == nullptr) 
	{
		UE_LOG(LogTemp, Log, TEXT("Audio Component Is Null"))
	}
	
	CreateTetrominoes();
	InitalizeRotationArrays();
	GenerateNewBlock();

	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			playField[i][j].isPlaced = false;
			playField[i][j].piece = nullptr;
		}
	}

	textComp = Cast<UTextRenderComponent>(scoreText->GetComponentByClass(UTextRenderComponent::StaticClass()));
	FString scoreString = "Current Score: " + FString::FromInt(currentScore);
	textComp->SetText(FText::FromString(scoreString));
}

// Called every frame
void AGameHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameHandler::GenerateNewBlock()
{
	if (bGameOver) { return; }

	pieceController->block = nullptr;
	ABasePiece** blocks = SpawnTetrominoe();

	if (playField[1][5].isPlaced || playField[0][5].isPlaced) {
		bGameOver = true;
	}

	pieceController->block = blocks;
}

ABasePiece** AGameHandler::SpawnTetrominoe() 
{
	if (bGameOver) { return nullptr; }

	tetromino tetrominoType = tetrominoes[0];
	
	if (bRandomTetrominoes) 
	{
		tetrominoType = tetrominoes[FMath::RandRange(0, 6)];
	}
	else 
	{
		tetrominoType = tetrominoes[TetToSpawn];
	}

	tetrominoType.Colour = StaticCast<colour>(FMath::RandRange(0, 6));
	currentTetrominoe = tetrominoType;
	UE_LOG(LogTemp, Log, TEXT("current tetromino: %s >> colour: %d"), *tetrominoType.TetrominoName, tetrominoType.Colour);

	ABasePiece** tetrominoe = new ABasePiece*[4];

	for (int i = 0; i < 4; i++)
	{
		AActor* actor = GWorld->SpawnActor<ABasePiece>(ABasePiece::StaticClass());
		ABasePiece* Piece = Cast<ABasePiece>(actor);
		UStaticMeshComponent* mesh = (UStaticMeshComponent*)actor->GetComponentByClass(UStaticMeshComponent::StaticClass());

		Piece->currentRow = tetrominoType.RowIndex[i];
		Piece->currentColumn = tetrominoType.ColumnIndex[i];

		if (mesh != nullptr)
			mesh->SetMaterial(0, ColoursArray[tetrominoType.Colour]);

		Piece->SetActorLocation(FVector(0, Piece->currentColumn * 100, 2000 - (Piece->currentRow) * 100));

		tetrominoe[i] = Piece;

		Piece->bDoTick = true;
	}

	return tetrominoe;
}

void AGameHandler::CheckPieceDataMismatch() 
{
	for (int i = 0; i < 4; i++) {
		if (currentTetrominoe.ColumnIndex[i] != pieceController->block[i]->currentColumn || currentTetrominoe.RowIndex[i] != pieceController->block[i]->currentRow)
		{
			currentTetrominoe.ColumnIndex[i] = pieceController->block[i]->currentColumn;
			currentTetrominoe.RowIndex[i] = pieceController->block[i]->currentRow;
		}
	}
}

void AGameHandler::SetPositionActive(ABasePiece* piece, int row, int column)
{
	if (bGameOver) { return; }

	playField[row][column].piece = piece;
	playField[row][column].isPlaced = true;


	pieceController->block = nullptr;

	if (pieceController->block == nullptr)
		GenerateNewBlock();

	CheckFullRow();

	placePiecesThisTetrominoe = 0;
	
}

void AGameHandler::CheckFullRow()
{
	int rowsToMove = 0;
	int rowIndexToMoveFrom = 0;

	bool bIsColourMatch = true;
	
	for (int i = 19; i >= 0; i--)
	{
		int filledSpaces = 0;

		for (int j = 0; j < 10; j++)
		{ 
			
			if (playField[i][j].isPlaced)
			{
				if (j != 9) {

					if (playField[i][j].Colour == playField[i][j + 1].Colour) 
					{
						bIsColourMatch = false;
					}
				}
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

			if (i > rowIndexToMoveFrom)
				rowIndexToMoveFrom = i;

			rowsToMove += 1;
		}

		//UE_LOG(LogTemp, Log, TEXT("Rows to move = %d "), rowsToMove);

		//UE_LOG(LogTemp, Log, TEXT("Row %d -> Current Filled Spaces: %d"), i, filledSpaces);
	}

	UE_LOG(LogTemp, Log, TEXT("Rows To Move: % d"), rowsToMove);

	UE_LOG(LogTemp, Log, TEXT("rowIndexToMoveFrom: % d"), rowIndexToMoveFrom);

	if (rowsToMove > 0) {
		MovePlayfield(rowsToMove, rowIndexToMoveFrom);

		if (bIsColourMatch) 
		{
			UpdateScore(rowsToMove, 4);
		}
		else 
		{
			UpdateScore(rowsToMove, 1);
		}
	}
		

}

void AGameHandler::MovePlayfield(int rowsToMove, int rowIndexToMoveFrom)
{
	// Notes incase you don't do it now,
	//Check full row needs to return which row number is full then this function needs to move that row down by one.
	//This is causing a crash
	for (int i = rowIndexToMoveFrom; i >= 0; i--)
	{
		for (int j = 9; j >= 0; j--)
		{
			if (playField[i][j].piece != nullptr) {

				playField[i][j].piece->currentRow += rowsToMove;
				playField[i + rowsToMove][j] = playField[i][j];

				playField[i][j].piece = nullptr;
				playField[i][j].isPlaced = false;
			}
		}
	}

	for (int i = 0; i < 20 ; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (playField[i][j].piece != nullptr) {

				playField[i][j].piece->SetActorLocation(FVector(0, playField[i][j].piece->currentColumn * 100, 2000 - ((playField[i][j].piece->currentRow) * 100)));

				playField[i][j].piece->isPlaced = true;
			}
		}
	}

	VerifyPlayfield();
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

void AGameHandler::UpdateScore(int linesCleared, int multiplier) 
{
	switch (linesCleared)
	{
	case 1:
		currentScore += (100 * currentLevel) * multiplier;
		totalLinesCleared += 1;
		break;
	case 2:
		totalLinesCleared += 2;
		currentScore += (300 * currentLevel) * multiplier;
		break;
	case 3:
		totalLinesCleared += 3;
		currentScore += (500 * currentLevel) * multiplier;
		break;
	case 4:
		totalLinesCleared += 4;
		currentScore += (1200 * currentLevel) * multiplier;
		break;
	}

	FString scoreString = "Current Score: " + FString::FromInt(currentScore);
	textComp->SetText(FText::FromString(scoreString));
}

void AGameHandler::BlockShift(int columnShift, int rowShift)
{
	for (int i = 0; i < 4; i++)
	{
		if (columnShift != 0) 
		{
			pieceController->block[i]->currentColumn += columnShift;
			currentTetrominoe.ColumnIndex[i] += columnShift;
		}
		
		if (rowShift != 0) 
		{
			pieceController->block[i]->currentRow += rowShift;
			currentTetrominoe.RowIndex[i] += rowShift;
		}

		pieceController->block[i]->SetActorLocation(FVector(0, pieceController->block[i]->currentColumn * 100, 2000 - pieceController->block[i]->currentRow * 100));
	}

	CheckPieceDataMismatch();
}

bool AGameHandler::CheckRotationLocations(targetLocationInArray locations[])
{
	//Checking to see if the target indexes in the playfield array are currently occupied by placed blocks or if they are out of bounds of the playfield.
	for (int i = 0; i < 4; i++)
	{
		if (playField[locations[i].RowShift][locations[i].ColumnShift].isPlaced)
			return false;

		//Checking to see if the target rotation location is out of bounds and if so adjust the block so it remains within the bounds of the playfield (Like what actually happens in Tetris)
		if (!playField[locations[i].RowShift][locations[i].ColumnShift].isPlaced) 
		{
			if (locations[i].ColumnShift < 0) 
			{
				BlockShift(1,0);
				return true;
			}
			
			if (locations[i].ColumnShift > 9)
			{
				BlockShift(-1,0);
				return true;
			}

			if (locations[i].RowShift < 0) 
			{
				BlockShift(0, 1);
			}
		}

	}

	CheckPieceDataMismatch();

	return true;
}

void AGameHandler::SetPieceRotation(int tetrominoIndex, int rotationIndex) {

	targetLocationInArray targetLocations[4];

	for (int i = 0; i < 4; i++)
	{
		targetLocations[i].ColumnShift = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[i].ColumnShift;
		targetLocations[i].RowShift = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[i].RowShift;
	}

	if (CheckRotationLocations(targetLocations) == false)
	{
		currentTetrominoe.currentRotation -= 1;
		return;
	}

	//index 5 is the square piece, due to this not sufferring from any shift throughout it's rotations you may aswell not rotate it at all. So I didn't.
	if (tetrominoIndex == 5)
	{
		return;
	}

	//Again another edge-case to do with the I / Line piece in Tetris, it actually rotates off axis and column shifts. This means that due to how rotation is programmed, the 2nd block of the 4 blocks that make up the I Piece needs to be rotated first as all other block positions in the tetromino are based off the second blocks positions.
	if (tetrominoIndex == 6)
	{
		pieceController->block[1]->currentColumn = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[1].ColumnShift;
		pieceController->block[1]->currentRow = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[1].RowShift;

		pieceController->block[0]->currentColumn = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[0].ColumnShift;
		pieceController->block[0]->currentRow = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[0].RowShift;

		pieceController->block[2]->currentColumn = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[2].ColumnShift;
		pieceController->block[2]->currentRow = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[2].RowShift;

		pieceController->block[3]->currentColumn = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[3].ColumnShift;
		pieceController->block[3]->currentRow = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[3].RowShift;

		return;
	}

	for (int i = 0; i < 4; i++)
	{
		pieceController->block[i]->currentColumn = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[i].ColumnShift;
		pieceController->block[i]->currentRow = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[i].RowShift;

		pieceController->block[i]->SetActorLocation(FVector(0, pieceController->block[i]->currentColumn * 100, 2000 - pieceController->block[i]->currentRow * 100));
	}

	CheckPieceDataMismatch();
}


void AGameHandler::PlayBlockMoveSound() 
{
	audioComponent->Play();
}


void AGameHandler::CreateTetrominoes() 
{

#pragma region L_Piece
	tetrominoes[0].TetrominoName = "L";
	tetrominoes[0].TetrominoIndex = 0;

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
	tetrominoes[1].TetrominoName = "Z";
	tetrominoes[1].TetrominoIndex = 1;

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
	tetrominoes[2].TetrominoName = "S";
	tetrominoes[2].TetrominoIndex = 2;

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
	tetrominoes[3].TetrominoName = "J";
	tetrominoes[3].TetrominoIndex = 3;

	tetrominoes[3].ColumnIndex[0] = 6;
	tetrominoes[3].RowIndex[0] = 1;

	tetrominoes[3].ColumnIndex[1] = 5;
	tetrominoes[3].RowIndex[1] = 1;

	tetrominoes[3].ColumnIndex[2] = 4;
	tetrominoes[3].RowIndex[2] = 1;

	tetrominoes[3].ColumnIndex[3] = 4;
	tetrominoes[3].RowIndex[3] = 0;
#pragma endregion J_Piece

#pragma region T_Piece
	tetrominoes[4].TetrominoName = "T";
	tetrominoes[4].TetrominoIndex = 4;

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
	tetrominoes[5].TetrominoName = "O";
	tetrominoes[5].TetrominoIndex = 5;

	tetrominoes[5].ColumnIndex[0] = 5;
	tetrominoes[5].RowIndex[0] = 0;

	tetrominoes[5].ColumnIndex[1] = 6;
	tetrominoes[5].RowIndex[1] = 0;

	tetrominoes[5].ColumnIndex[2] = 5;
	tetrominoes[5].RowIndex[2] = 1;

	tetrominoes[5].ColumnIndex[3] = 6;
	tetrominoes[5].RowIndex[3] = 1;
#pragma endregion Square_Piece

#pragma region I_Piece
	tetrominoes[6].TetrominoName = "I";
	tetrominoes[6].TetrominoIndex = 6;

	tetrominoes[6].ColumnIndex[0] = 3;
	tetrominoes[6].RowIndex[0] = 0;

	tetrominoes[6].ColumnIndex[1] = 4;
	tetrominoes[6].RowIndex[1] = 0;

	tetrominoes[6].ColumnIndex[2] = 5;
	tetrominoes[6].RowIndex[2] = 0;

	tetrominoes[6].ColumnIndex[3] = 6;
	tetrominoes[6].RowIndex[3] = 0;
#pragma endregion Line_Piece

}

void AGameHandler::InitalizeRotationArrays()
{
#pragma region L Piece Rotations
	//0 Degrees
	tetrominoes[0].rotationData[0].data[0].ColumnShift = -1;
	tetrominoes[0].rotationData[0].data[0].RowShift = 0;
	
	tetrominoes[0].rotationData[0].data[1].ColumnShift = 0;
	tetrominoes[0].rotationData[0].data[1].RowShift = 0;

	tetrominoes[0].rotationData[0].data[2].ColumnShift = 1;
	tetrominoes[0].rotationData[0].data[2].RowShift = 0;

	tetrominoes[0].rotationData[0].data[3].ColumnShift = 1;
	tetrominoes[0].rotationData[0].data[3].RowShift = -1;

	//90 Degrees
	tetrominoes[0].rotationData[1].data[0].ColumnShift = 0;
	tetrominoes[0].rotationData[1].data[0].RowShift = 1;

	tetrominoes[0].rotationData[1].data[1].ColumnShift = 0;
	tetrominoes[0].rotationData[1].data[1].RowShift = 0;

	tetrominoes[0].rotationData[1].data[2].ColumnShift = 0;
	tetrominoes[0].rotationData[1].data[2].RowShift = -1;

	tetrominoes[0].rotationData[1].data[3].ColumnShift = -1;
	tetrominoes[0].rotationData[1].data[3].RowShift = -1;

	//180 Degrees
	tetrominoes[0].rotationData[2].data[0].ColumnShift = 1;
	tetrominoes[0].rotationData[2].data[0].RowShift = 0;
								
	tetrominoes[0].rotationData[2].data[1].ColumnShift = 0;
	tetrominoes[0].rotationData[2].data[1].RowShift = 0;
								
	tetrominoes[0].rotationData[2].data[2].ColumnShift = -1;
	tetrominoes[0].rotationData[2].data[2].RowShift = 0;
								
	tetrominoes[0].rotationData[2].data[3].ColumnShift = -1;
	tetrominoes[0].rotationData[2].data[3].RowShift = 1;

	//270 Degrees
	tetrominoes[0].rotationData[3].data[0].ColumnShift = 0;
	tetrominoes[0].rotationData[3].data[0].RowShift = -1;

	tetrominoes[0].rotationData[3].data[1].ColumnShift = 0;
	tetrominoes[0].rotationData[3].data[1].RowShift = 0;

	tetrominoes[0].rotationData[3].data[2].ColumnShift = 0;
	tetrominoes[0].rotationData[3].data[2].RowShift = 1;

	tetrominoes[0].rotationData[3].data[3].ColumnShift = 1;
	tetrominoes[0].rotationData[3].data[3].RowShift = 1;
#pragma endregion

#pragma region Z Piece Rotations
	//0 Degrees
	tetrominoes[1].rotationData[0].data[0].ColumnShift = -1;
	tetrominoes[1].rotationData[0].data[0].RowShift = 0;
				
	tetrominoes[1].rotationData[0].data[1].ColumnShift = 0;
	tetrominoes[1].rotationData[0].data[1].RowShift = 0;
				
	tetrominoes[1].rotationData[0].data[2].ColumnShift = 0;
	tetrominoes[1].rotationData[0].data[2].RowShift = 1;
				
	tetrominoes[1].rotationData[0].data[3].ColumnShift = 1;
	tetrominoes[1].rotationData[0].data[3].RowShift = 1;
				
	//90 Degrees1
	tetrominoes[1].rotationData[1].data[0].ColumnShift = 0;
	tetrominoes[1].rotationData[1].data[0].RowShift = -1;
				
	tetrominoes[1].rotationData[1].data[1].ColumnShift = 0;
	tetrominoes[1].rotationData[1].data[1].RowShift = 0;
				
	tetrominoes[1].rotationData[1].data[2].ColumnShift = -1;
	tetrominoes[1].rotationData[1].data[2].RowShift = 0;
				
	tetrominoes[1].rotationData[1].data[3].ColumnShift = -1;
	tetrominoes[1].rotationData[1].data[3].RowShift = 1;

	//180
	tetrominoes[1].rotationData[2].data[0].ColumnShift = 1;
	tetrominoes[1].rotationData[2].data[0].RowShift = 0;

	tetrominoes[1].rotationData[2].data[1].ColumnShift = 0;
	tetrominoes[1].rotationData[2].data[1].RowShift = 0;
				
	tetrominoes[1].rotationData[2].data[2].ColumnShift = 0;
	tetrominoes[1].rotationData[2].data[2].RowShift = -1;
				
	tetrominoes[1].rotationData[2].data[3].ColumnShift = -1;
	tetrominoes[1].rotationData[2].data[3].RowShift = -1;
				
	//270		
	tetrominoes[1].rotationData[3].data[0].ColumnShift = 0;
	tetrominoes[1].rotationData[3].data[0].RowShift = 1;
				
	tetrominoes[1].rotationData[3].data[1].ColumnShift = 0;
	tetrominoes[1].rotationData[3].data[1].RowShift = 0;
				
	tetrominoes[1].rotationData[3].data[2].ColumnShift = -1;
	tetrominoes[1].rotationData[3].data[2].RowShift = 0;
				
	tetrominoes[1].rotationData[3].data[3].ColumnShift = -1;
	tetrominoes[1].rotationData[3].data[3].RowShift = -1;
#pragma endregion

#pragma region S Piece Rotations
	//0 Degrees
	tetrominoes[2].rotationData[0].data[0].ColumnShift = -1;
	tetrominoes[2].rotationData[0].data[0].RowShift = 0;
				
	tetrominoes[2].rotationData[0].data[1].ColumnShift = 0;
	tetrominoes[2].rotationData[0].data[1].RowShift = 0;
				
	tetrominoes[2].rotationData[0].data[2].ColumnShift = 0;
	tetrominoes[2].rotationData[0].data[2].RowShift = -1;
				
	tetrominoes[2].rotationData[0].data[3].ColumnShift = 1;
	tetrominoes[2].rotationData[0].data[3].RowShift = -1;
				
	//90 Degrees2
	tetrominoes[2].rotationData[1].data[0].ColumnShift = 0;
	tetrominoes[2].rotationData[1].data[0].RowShift = -1;
				
	tetrominoes[2].rotationData[1].data[1].ColumnShift = 0;
	tetrominoes[2].rotationData[1].data[1].RowShift = 0;
				
	tetrominoes[2].rotationData[1].data[2].ColumnShift = 1;
	tetrominoes[2].rotationData[1].data[2].RowShift = 0;
				
	tetrominoes[2].rotationData[1].data[3].ColumnShift = 1;
	tetrominoes[2].rotationData[1].data[3].RowShift = 1;
				
	//180		
	tetrominoes[2].rotationData[2].data[0].ColumnShift = 1;
	tetrominoes[2].rotationData[2].data[0].RowShift = 0;
				
	tetrominoes[2].rotationData[2].data[1].ColumnShift = 0;
	tetrominoes[2].rotationData[2].data[1].RowShift = 0;
				
	tetrominoes[2].rotationData[2].data[2].ColumnShift = 0;
	tetrominoes[2].rotationData[2].data[2].RowShift = 1;
				
	tetrominoes[2].rotationData[2].data[3].ColumnShift = -1;
	tetrominoes[2].rotationData[2].data[3].RowShift = 1;
				
	//270		
	tetrominoes[2].rotationData[3].data[0].ColumnShift = 0;
	tetrominoes[2].rotationData[3].data[0].RowShift = 1;
				
	tetrominoes[2].rotationData[3].data[1].ColumnShift = 0;
	tetrominoes[2].rotationData[3].data[1].RowShift = 0;
				
	tetrominoes[2].rotationData[3].data[2].ColumnShift = -1;
	tetrominoes[2].rotationData[3].data[2].RowShift = 0;
				
	tetrominoes[2].rotationData[3].data[3].ColumnShift = -1;
	tetrominoes[2].rotationData[3].data[3].RowShift = -1;
#pragma endregion
	
#pragma region J Piece Rotations
	//0 Degrees
	tetrominoes[3].rotationData[0].data[0].ColumnShift = 1;
	tetrominoes[3].rotationData[0].data[0].RowShift = 0;
				
	tetrominoes[3].rotationData[0].data[1].ColumnShift = 0;
	tetrominoes[3].rotationData[0].data[1].RowShift = 0;
				
	tetrominoes[3].rotationData[0].data[2].ColumnShift = -1;
	tetrominoes[3].rotationData[0].data[2].RowShift = 0;
				
	tetrominoes[3].rotationData[0].data[3].ColumnShift = -1;
	tetrominoes[3].rotationData[0].data[3].RowShift = -1;
				
	//90 Degrees
	tetrominoes[3].rotationData[1].data[0].ColumnShift = 0;
	tetrominoes[3].rotationData[1].data[0].RowShift = 1;
								
	tetrominoes[3].rotationData[1].data[1].ColumnShift = 0;
	tetrominoes[3].rotationData[1].data[1].RowShift = 0;
								
	tetrominoes[3].rotationData[1].data[2].ColumnShift = 0;
	tetrominoes[3].rotationData[1].data[2].RowShift = -1;
								
	tetrominoes[3].rotationData[1].data[3].ColumnShift = 1;
	tetrominoes[3].rotationData[1].data[3].RowShift = -1;
				
	//180		
	tetrominoes[3].rotationData[2].data[0].ColumnShift = -1;
	tetrominoes[3].rotationData[2].data[0].RowShift = 0;
				
	tetrominoes[3].rotationData[2].data[1].ColumnShift = 0;
	tetrominoes[3].rotationData[2].data[1].RowShift = 0;
				
	tetrominoes[3].rotationData[2].data[2].ColumnShift = 1;
	tetrominoes[3].rotationData[2].data[2].RowShift = 0;
				
	tetrominoes[3].rotationData[2].data[3].ColumnShift = 1;
	tetrominoes[3].rotationData[2].data[3].RowShift = 1;
				
	//270
	tetrominoes[3].rotationData[3].data[0].ColumnShift = 0;
	tetrominoes[3].rotationData[3].data[0].RowShift = -1;
								
	tetrominoes[3].rotationData[3].data[1].ColumnShift = 0;
	tetrominoes[3].rotationData[3].data[1].RowShift = 0;
								
	tetrominoes[3].rotationData[3].data[2].ColumnShift = 0;
	tetrominoes[3].rotationData[3].data[2].RowShift = 1;
								
	tetrominoes[3].rotationData[3].data[3].ColumnShift = -1;
	tetrominoes[3].rotationData[3].data[3].RowShift = 1;

#pragma endregion

#pragma region T Piece Rotations
	//0 Degrees
	tetrominoes[4].rotationData[0].data[0].ColumnShift = -1;
	tetrominoes[4].rotationData[0].data[0].RowShift = 0;
				
	tetrominoes[4].rotationData[0].data[1].ColumnShift = 0;
	tetrominoes[4].rotationData[0].data[1].RowShift = 0;
				
	tetrominoes[4].rotationData[0].data[2].ColumnShift = 0;
	tetrominoes[4].rotationData[0].data[2].RowShift = -1;
				
	tetrominoes[4].rotationData[0].data[3].ColumnShift = 1;
	tetrominoes[4].rotationData[0].data[3].RowShift = 0;
				
	//90 Degrees4
	tetrominoes[4].rotationData[1].data[0].ColumnShift = 0;
	tetrominoes[4].rotationData[1].data[0].RowShift = 1;
				
	tetrominoes[4].rotationData[1].data[1].ColumnShift = 0;
	tetrominoes[4].rotationData[1].data[1].RowShift = 0;
				
	tetrominoes[4].rotationData[1].data[2].ColumnShift = 0;
	tetrominoes[4].rotationData[1].data[2].RowShift = -1;
				
	tetrominoes[4].rotationData[1].data[3].ColumnShift = 1;
	tetrominoes[4].rotationData[1].data[3].RowShift = 0;
				
	//180		
	tetrominoes[4].rotationData[2].data[0].ColumnShift = 0;
	tetrominoes[4].rotationData[2].data[0].RowShift = 1;
				
	tetrominoes[4].rotationData[2].data[1].ColumnShift = 0;
	tetrominoes[4].rotationData[2].data[1].RowShift = 0;
				
	tetrominoes[4].rotationData[2].data[2].ColumnShift = -1;
	tetrominoes[4].rotationData[2].data[2].RowShift = 0;
				
	tetrominoes[4].rotationData[2].data[3].ColumnShift = 1;
	tetrominoes[4].rotationData[2].data[3].RowShift = 0;
				
	//270		
	tetrominoes[4].rotationData[3].data[0].ColumnShift = -1;
	tetrominoes[4].rotationData[3].data[0].RowShift = 0;
				
	tetrominoes[4].rotationData[3].data[1].ColumnShift = 0;
	tetrominoes[4].rotationData[3].data[1].RowShift = 0;
				
	tetrominoes[4].rotationData[3].data[2].ColumnShift = 0;
	tetrominoes[4].rotationData[3].data[2].RowShift = -1;
				
	tetrominoes[4].rotationData[3].data[3].ColumnShift = 0;
	tetrominoes[4].rotationData[3].data[3].RowShift = 1;
#pragma endregion
	
#pragma region O Piece Rotations

#pragma endregion
	
#pragma region I Piece Rotations
	//0 Degrees
	tetrominoes[6].rotationData[0].data[0].ColumnShift = -1;
	tetrominoes[6].rotationData[0].data[0].RowShift = 0;
				
	tetrominoes[6].rotationData[0].data[1].ColumnShift = 0;
	tetrominoes[6].rotationData[0].data[1].RowShift = 0;
				
	tetrominoes[6].rotationData[0].data[2].ColumnShift = 1;
	tetrominoes[6].rotationData[0].data[2].RowShift = 0;
				
	tetrominoes[6].rotationData[0].data[3].ColumnShift = 2;
	tetrominoes[6].rotationData[0].data[3].RowShift = 0;
				
	//90 Degrees
	tetrominoes[6].rotationData[1].data[0].ColumnShift = 0;
	tetrominoes[6].rotationData[1].data[0].RowShift = 1;
								
	tetrominoes[6].rotationData[1].data[1].ColumnShift = 1;
	tetrominoes[6].rotationData[1].data[1].RowShift = 0;
								
	tetrominoes[6].rotationData[1].data[2].ColumnShift = 0;
	tetrominoes[6].rotationData[1].data[2].RowShift = -1;
								
	tetrominoes[6].rotationData[1].data[3].ColumnShift = 0;
	tetrominoes[6].rotationData[1].data[3].RowShift = -2;
				
	//180		
	tetrominoes[6].rotationData[2].data[0].ColumnShift = -1;
	tetrominoes[6].rotationData[2].data[0].RowShift = 0;
				
	tetrominoes[6].rotationData[2].data[1].ColumnShift = 0;
	tetrominoes[6].rotationData[2].data[1].RowShift = 0;
				
	tetrominoes[6].rotationData[2].data[2].ColumnShift = 1;
	tetrominoes[6].rotationData[2].data[2].RowShift = 0;
				
	tetrominoes[6].rotationData[2].data[3].ColumnShift = 2;
	tetrominoes[6].rotationData[2].data[3].RowShift = 0;
				
	//270
	tetrominoes[6].rotationData[3].data[0].ColumnShift = 0;
	tetrominoes[6].rotationData[3].data[0].RowShift = -1;
								
	tetrominoes[6].rotationData[3].data[1].ColumnShift = -1;
	tetrominoes[6].rotationData[3].data[1].RowShift = 0;
								
	tetrominoes[6].rotationData[3].data[2].ColumnShift = 0;
	tetrominoes[6].rotationData[3].data[2].RowShift = 1;
								
	tetrominoes[6].rotationData[3].data[3].ColumnShift = 0;
	tetrominoes[6].rotationData[3].data[3].RowShift = 2;

#pragma endregion

#pragma endregion
}


void AGameHandler::GameOver() 
{
	return;
}

