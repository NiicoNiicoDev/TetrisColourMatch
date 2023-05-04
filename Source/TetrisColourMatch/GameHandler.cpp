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

	//Initalize the playfield array to ensure all values are as expected
	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			playField[i][j].isPlaced = false;
			playField[i][j].piece = nullptr;
		}
	}
}

// Called when the game starts or when spawned
void AGameHandler::BeginPlay()
{
	Super::BeginPlay();
	
	//get reference to piece control and store as local variable
	pieceController = Cast<APieceController>(UGameplayStatics::GetActorOfClass(GetWorld(), APieceController::StaticClass()));

	//get reference to the audio component and store as local variable
	audioComponent = FindComponentByClass<UAudioComponent>();
	
	// Initalize the tetrominoes array with all tetromino spawn indexes relative to the playfield array.
	CreateTetrominoes();
	
	// Initalize the rotations array which stores all of the requires row and colum shifts for each cube of each teterimo for all 4 degrees of rotation.
	InitalizeRotationArrays();

	//Generate a new block to start the game.
	GenerateNewBlock();

	//Get reference to the score text component and store as local variable
	scoreTextComp = Cast<UTextRenderComponent>(scoreText->GetComponentByClass(UTextRenderComponent::StaticClass()));

	//Get reference to the level text component and store as local variable
	levelTextComp = Cast<UTextRenderComponent>(levelText->GetComponentByClass(UTextRenderComponent::StaticClass()));
	
	//Create a string variable to store the current score and convert it to a string
	FString scoreString = "Current Score: " + FString::FromInt(currentScore);

	//Set the score text component to the above created string variable
	scoreTextComp->SetText(FText::FromString(scoreString));
}

// Called every frame
void AGameHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bGameOver) 
	{
		//Check if the centre block at the top of the playfield array contains a placed block, if so end the game
		if (playField[0][5].isPlaced)
		{
			bGameOver = true;
			UE_LOG(LogTemp, Log, TEXT("Game Over"))
		}

		//Check if the centre block at the second from top of the playfield array contains a placed block, if so end the game
		if (playField[1][5].isPlaced) {
			bGameOver = true;
			UE_LOG(LogTemp, Log, TEXT("Game Over"))
		}

		//Both the above checks are required as the tetrominoes spawn at the top of the playfield array in one of those two positions.

		// if the bGameOver Bool is true, call the Game Over Function.
		if (bGameOver) 
		{
			GameOver();
		}
	}
}

void AGameHandler::GenerateNewBlock()
{
	// If the game is not over, procede with generating a new tetromino to spawn at the top of the play field.
	if (!bGameOver) 
	{
		//Set the current 'block' (collection of 4 cubes that make up a tetromino) to null.
		pieceController->block = nullptr;
		
		// Set the blocks array to the return value of the SpawnTetrominoe function.
		ABasePiece** blocks = SpawnTetrominoe();

		//set the block in the piece controller to the blocks array returned from the SpawnTetrominoe function.
		pieceController->block = blocks;
	}
}

ABasePiece** AGameHandler::SpawnTetrominoe() 
{
	//if the game is over exit this function.
	if (bGameOver) { return nullptr; }

	// initialize a new tetrominoe variable to store the tetrominoe to be spawned.
	tetromino tetrominoType = tetrominoes[0];
	
	//if the boolean bRandomTetrominoes is true, generate a random tetrominoe to spawn.
	if (bRandomTetrominoes) 
	{
		//Selects a random tetrominoe from the tetrominoes array and stores it in the tetrominoType variable.
		tetrominoType = tetrominoes[FMath::RandRange(0, 6)];
	}
	else 
	{
		//Used for debugging, allows for the selection of a specific tetrominoe to spawn.
		tetrominoType = tetrominoes[TetToSpawn];
	}

	//Set the colour variable of the newly spawned tetrominoe to a random colour enum.
	tetrominoType.Colour = StaticCast<colour>(FMath::RandRange(0, 6));
	
	//Set the current tetrominoe variable to the newly spawned tetrominoe.
	currentTetrominoe = tetrominoType;

	//Create a new array of 4 ABasePiece pointers to store the tetrominoe to be spawned.
	ABasePiece** tetrominoe = new ABasePiece*[4];

	//For each of the cubes that comprise thew newly spawned tetrominoe...
	for (int i = 0; i < 4; i++)
	{
		//Spawn an empty actor in the world
		AActor* actor = GWorld->SpawnActor<ABasePiece>(ABasePiece::StaticClass());
		//Set the piece variable in the ABasePiece class to said spawned actor
		ABasePiece* Piece = Cast<ABasePiece>(actor);
		//Give the newly spawned actor a mesh component
		UStaticMeshComponent* mesh = (UStaticMeshComponent*)actor->GetComponentByClass(UStaticMeshComponent::StaticClass());

		//Set the current row and column of the newly spawned actor to the row and column of the current cube of the tetrominoe to be spawned.
		Piece->currentRow = tetrominoType.RowIndex[i];
		Piece->currentColumn = tetrominoType.ColumnIndex[i];

		//Set the colour of the newly spawned actor to the colour of the tetrominoe to be spawned.
		if (mesh != nullptr)
			mesh->SetMaterial(0, ColoursArray[tetrominoType.Colour]);

		//Set the location of the newly spawned actor to the row and column of the current cube of the tetrominoe to be spawned.
		Piece->SetActorLocation(FVector(0, Piece->currentColumn * 100, 2000 - (Piece->currentRow) * 100));

		//Initializes the game handlers local reference to the newly spawned actor.
		tetrominoe[i] = Piece;
	}

	//returns the newly spawned tetrominoe.
	return tetrominoe;
}

void AGameHandler::CheckPieceDataMismatch() 
{
	//Compares the current row and column of each cube in the tetrominoe to the current row and column of the corresponding cube in the piece controller.#
	//If this data differs the Piece controller takes presidence and the game handler sets it's copy of the data to match that of the piece controller.
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
	//If the game is over exit this fuction.
	if (bGameOver) { return; }

	//Sets the row and column of the playfield to the row and column of the current cube of the tetrominoe.
	playField[row][column].piece = piece;
	//Sets the is placed variable of the playfield at the row and column of the current cube to true.
	playField[row][column].isPlaced = true;

	//increment as each cube of the current tetrominoe calls this fuction.
	placePiecesThisTetrominoe += 1;

	//If all 4 cubes of the current tetrominoe have been placed and the piece controller currently has a block stored...
	if (placePiecesThisTetrominoe == 4 && pieceController->block != nullptr)
	{
		//remove the piece controllers current block reference.
		pieceController->block = nullptr;
		//Generate a new block
		GenerateNewBlock();

		//Check if the playfield has had a row filled.
		CheckFullRow();
	}
}

void AGameHandler::CheckFullRow()
{
	//Variable to store the numbers of rows that have been cleared this piece placement.
	int rowsToMove = 0;
	//Variable to store which row the game should start checking for filled rows.
	int rowIndexToMoveFrom = 0;

	//Varaible to check if all pieces in the current line clear are of the same colour.
	bool bIsColourMatch = true;
	
	//For every row in the playfield...
	for (int i = 19; i >= 0; i--)
	{
		//variable to track how many cubes are placed in the current row.
		int filledSpaces = 0;

		//For every column in the playfield
		for (int j = 0; j < 10; j++)
		{ 
			//if the current row and column of the playfield is placed...
			if (playField[i][j].isPlaced)
			{
				//increment the filled spaces varaible
				filledSpaces++;
			}
		}

		//If the filled spaces variable is equal to 10, and thus the row and index 'i' is full...
		if (filledSpaces == 10)
		{
			//iterate through each block in the row.
			for (int k = 0; k < 10; k++)
			{
				//further check to ensure that the current row and column of the playfield is placed and contains a block.
				if (playField[i][k].piece != nullptr) 
				{
					//iterator to ensure that an out of bounds error is not thrown.
					if (k < 9)
					{
						//Check if the current block has the same colour as the next block over in the column
						if (playField[i][k].Colour != playField[i][k + 1].Colour)
						{
							//If the colours do not match set the colour match variable to false.
							bIsColourMatch = false;
						}
					}
					
					//Destroy the current block in the row
					playField[i][k].piece->Destroy();

					//Set the current row and column of the playfield to not placed and to have no block.
					playField[i][k].isPlaced = false;
					playField[i][k].piece = nullptr;
				}
			}

			//check to ensure the 'lowest' row in the playfield array is going to be the row to clear from (The playfield is technically inverted, so the top row is the lowest row in the array)
			if (i > rowIndexToMoveFrom)
				rowIndexToMoveFrom = i;

			//increment the rows to move variable.
			rowsToMove += 1;
		}
	}

	//if a row has been cleared...
	if (rowsToMove > 0) {
		//Move the rest of the playfield by the number of rows that have been cleared starting from the row that has been cleared.
		MovePlayfield(rowsToMove, rowIndexToMoveFrom);
		
		//if the colour match variable is true...
		if (bIsColourMatch) 
		{
			//Update the score with a score multiplier of 4.
			UpdateScore(rowsToMove, 4);
		}
		else //otherwise
		{
			//Update the score with a score multiplier of 1.
			UpdateScore(rowsToMove, 1);
		}
		
	}
}

void AGameHandler::MovePlayfield(int rowsToMove, int rowIndexToMoveFrom)
{
	//starting from the lowest row that has been cleared and moving up the playfield...
	for (int i = rowIndexToMoveFrom; i >= 0; i--)
	{
		//iterate through each block in the each column of that row
		for (int j = 9; j >= 0; j--)
		{
			//If that row and column contains a block
			if (playField[i][j].piece != nullptr) {

				//move the row by the number of lines that were cleared
				playField[i][j].piece->currentRow += rowsToMove;
				//set the data of the playfield array in the new location equal to that of the previous location.
				playField[i + rowsToMove][j] = playField[i][j];

				//set the previous location of the playfield array to not placed and to have no block.
				playField[i][j].piece = nullptr;
				playField[i][j].isPlaced = false;
			}
		}
	}
	//Iterate through the entire playfield
	for (int i = 0; i < 20 ; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			//if the playfield array contains a block
			if (playField[i][j].piece != nullptr) {

				//Set the visual representation of the block to the correct position in the world.
				playField[i][j].piece->SetActorLocation(FVector(0, playField[i][j].piece->currentColumn * 100, 2000 - ((playField[i][j].piece->currentRow) * 100)));
				//Set the current index in the array to true to ensure all future collisions are detected correctly.
				playField[i][j].piece->isPlaced = true;
			}
		}
	}

	//Verify the playfield.
	VerifyPlayfield();
}

void AGameHandler::VerifyPlayfield() 
{
	//Iterate through the entire playfield
	for (int i = 0; i < 20; i++)
	{
		for (int j = 1; j < 10; j++)
		{
			if (playField[i][j].piece != nullptr) 
			{
				//Ensure that indexes that contain blocks have their isPlaced boolean set to true.
				if (!playField[i][j].piece->isPlaced)
					playField[i][j].isPlaced = true;
				
				//If the playfield contains a block at the given index but the block has no name...
				if (playField[i][j].piece->GetName().IsEmpty())
				{
					//Presume their is no block at that position and set that index to null.
					playField[i][j].piece = nullptr;
				}
			}
		}
	}
}

void AGameHandler::UpdateScore(int linesCleared, int multiplier) 
{
	//Switch on the number of lines cleared.
	switch (linesCleared)
	{
		//If one line has been cleared
	case 1:
		//Add 100 * the current level * the multiplier to the current score.
		currentScore += (100 * currentLevel) * multiplier;
		//Increment the total lines cleared by 1.
		totalLinesCleared += 1;
		break;
		//If two lines have been cleared
	case 2:
		//Add 300 * the current level * the multiplier to the current score.
		currentScore += (300 * currentLevel) * multiplier;
		//Increment the total lines cleared by 2.
		totalLinesCleared += 2;
		break;
		//If three lines have been cleared
	case 3:
		//Add 500 * the current level * the multiplier to the current score.
		currentScore += (500 * currentLevel) * multiplier;
		//Increment the total lines cleared by 3.
		totalLinesCleared += 3;
		break;
		//If four lines have been cleared
	case 4:
		//Add 1200 * the current level * the multiplier to the current score.
		currentScore += (1200 * currentLevel) * multiplier;
		//Increment the total lines cleared by 4.
		totalLinesCleared += 4;
		break;
	}

	//Iterate through the array of lines to clear per level
	for (int var : LinesToClearPerLevel)
	{
		//If the total lines cleared is equal to the current number of lines to clear per level...
		if (totalLinesCleared == var) 
		{
			//Increment the current level by 1.
			currentLevel += 1;
			//Call the IncreaseDifficulty function.
			IncreaseDifficulty();
		}
	}

	//Update the score text to display the current score.
	FString scoreString = "Current Score: " + FString::FromInt(currentScore);
	scoreTextComp->SetText(FText::FromString(scoreString));
}

void AGameHandler::IncreaseDifficulty() 
{

	//Update the level text to display the current level.
	FString levelString = FString::FromInt(currentLevel);
	levelTextComp->SetText(FText::FromString(levelString));
	
	//Set the move down frequency (the rate at which the blocks move down the playfield) equal to 0.9 - the current difficulty level multiplied by 0.03.
	moveDownFrequency = 0.9 - (currentLevel * 0.03);
}

void AGameHandler::BlockShift(int columnShift, int rowShift)
{
	// Iterating through each block in the piece controller block array
	for (int i = 0; i < 4; i++)
	{
		//if a column shift is required
		if (columnShift != 0) 
		{
			//modify the column index for each cube in the tetrominoe by the column shift parameter passed into the function for both the piece controller and the game handler.
			pieceController->block[i]->currentColumn += columnShift;
			currentTetrominoe.ColumnIndex[i] += columnShift;
		}
		
		//if a column shift is required
		if (rowShift != 0) 
		{
			//modify the row index for each cube in the tetrominoe by the row shift parameter passed into the function for both the piece controller and the game handler.
			pieceController->block[i]->currentRow += rowShift;
			currentTetrominoe.RowIndex[i] += rowShift;
		}

		//set the location of each cube in the tetrominoe to the new column and row position after the shift.
		pieceController->block[i]->SetActorLocation(FVector(0, pieceController->block[i]->currentColumn * 100, 2000 - pieceController->block[i]->currentRow * 100));
	}

	//Check if the any of the data between the pieceController and gameHandler is different/incorrect
	CheckPieceDataMismatch();
}

bool AGameHandler::CheckRotationLocations(targetLocationInArray locations[])
{
	//Checking to see if the target indexes in the playfield array are currently occupied by placed blocks or if they are out of bounds of the playfield.
	for (int i = 0; i < 4; i++)
	{
		//if the target location of the rotation contains a block exit the function.
		if (playField[locations[i].RowShift][locations[i].ColumnShift].isPlaced)
			return false;

		//Checking to see if the target rotation location is out of bounds and if so adjust the block so it remains within the bounds of the playfield (Like what actually happens in Tetris)
		if (!playField[locations[i].RowShift][locations[i].ColumnShift].isPlaced) 
		{
			//if the location target location of the rotation exceed the bounds of the left wall of the playfield...
			if (locations[i].ColumnShift < 0) 
			{
				//shift the column of all blocks of the tetrominoe one place to the right and exit the function
				BlockShift(1,0);
				return true;
			}
			
			//if the location target location of the rotation exceed the bounds of the right wall of the playfield...
			if (locations[i].ColumnShift > 9)
			{
				//shift the column of all blocks of the tetrominoe one place to the left and exit the function
				BlockShift(-1,0);
				return true;
			}


			//if the location target location of the rotation exceed the bounds of the top of the playfield...
			if (locations[i].RowShift < 0) 
			{
				//Shift row of the blocks of the tetrominoe one place down and exit the function.
				BlockShift(0, 1);
			}
		}

	}

	//Check if the any of the data between the pieceController and gameHandler is different/incorrect
	CheckPieceDataMismatch();

	return true;
}

void AGameHandler::SetPieceRotation(int tetrominoIndex, int rotationIndex) {

	//Custom Struct array to hold the target rotation data for each cube in the tetrominoe
	targetLocationInArray targetLocations[4];

	//Iterate over each cube in the tetrominoe
	for (int i = 0; i < 4; i++)
	{
		//Set the target location of the cube to the 2nd (Pivot) block of the current tetrominoe + the pre-determined offset required to achieve the rotation
		//Thius is then passed into the CheckRotationLocations function to be checked for any potential collisions.
		targetLocations[i].ColumnShift = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[i].ColumnShift;
		targetLocations[i].RowShift = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[i].RowShift;
	}

	//If the target location can not be achieved, decrement the tetrominoes current rotation variable and exit the function.
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

	//If the rotation was successful update the the current column and row of each cube in the tetrominoe and set it's visual representations world position.
	for (int i = 0; i < 4; i++)
	{
		pieceController->block[i]->currentColumn = pieceController->block[1]->currentColumn + currentTetrominoe.rotationData[rotationIndex].data[i].ColumnShift;
		pieceController->block[i]->currentRow = pieceController->block[1]->currentRow + currentTetrominoe.rotationData[rotationIndex].data[i].RowShift;

		pieceController->block[i]->SetActorLocation(FVector(0, pieceController->block[i]->currentColumn * 100, 2000 - pieceController->block[i]->currentRow * 100));
	}
	
	//Check if the any of the data between the pieceController and gameHandler is different/incorrect
	CheckPieceDataMismatch();
}


void AGameHandler::PlayBlockMoveSound() 
{
	//Plays the audio file attached to the scene audio component
	audioComponent->Play();
	
	//Sets the number of pieces placed this block to 0. This is used in the SetPositionActive function. This is called here as the PlayBlock move sound only occurs when the block moves down.
	//This was done as there was an issue with multiple blocks spawning when this was executed in the SetPositionActive function. Calling it here delayed the variable being reset until all code execution has completed and the newly spawned block had begun moving.
	placePiecesThisTetrominoe = 0;
}


void AGameHandler::CreateTetrominoes() 
{
	//Unique Indexes, Names and Spawn positions for each of the 4 cubes that comprise each tetrominoe shape.
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
	//This is initailizing the offset that each of the 4 block comprising a tetrominoe require to achieve the required rotation.
	// >>> KEY <<<
	// rotationData 0 = 0 degrees
	// rotationData 1 = 90 degrees
	// rotationData 2 = 180 degrees
	// rotationData 3 = 270 degrees

	// data 0 = block 0
	// data 1 = block 1
	// data 2 = block 2
	// data 3 = block 3
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
	//Calls a blueprint function attached to a child blueprint class of the piece controller. This is only used for User Interface functionality
	pieceController->BPGameOver(currentScore, bGameOver);
	return;
}

