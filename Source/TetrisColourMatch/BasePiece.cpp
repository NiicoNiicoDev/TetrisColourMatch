// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePiece.h"
#include "GameHandler.h"
#include "PieceController.h"

// Sets default values
ABasePiece::ABasePiece()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	UStaticMeshComponent* CubeVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRep"));
	CubeVisual->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>CubeVisualAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeVisualAsset.Succeeded()) 
	{
		CubeVisual->SetStaticMesh(CubeVisualAsset.Object);
		CubeVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		CubeVisual->SetWorldScale3D(FVector(1.0f));
	}

	blockPlaceTimer = 0;
	bCanMoveDown = true;

	currentRow = spawnRow;
	currentColumn = spawnColumn;
}

// Called when the game starts or when spawned
void ABasePiece::BeginPlay()
{
	Super::BeginPlay();

	//Gets reference to the game handler in the current scene.
	gameHandler = Cast<AGameHandler>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameHandler::StaticClass()));
}

// Called every frame
void ABasePiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//If the game is not over
	if (!gameHandler->bGameOver)
	{
		//Variable to store the target location that the block is intending to move to based on it's current row and column variables.
		FVector targetLocation = FVector(0, currentColumn * 100, 2000 - ((currentRow) * 100));

		//Set the block to its target position.
		SetActorLocation(targetLocation);

		//If the block extends the playfield on either the left or right side. Exit the function
		if (currentRow < 0 || currentRow > 19)
			return;

		//If the block is not placed
		if (!isPlaced)
		{
			//Increase the currentTime variable
			currentTime += DeltaTime;
			//compartive to the check piece index function which checks to see the current state of the block. These states are defined by integer return numbers and are noted in the CheckPieceIndex Function.
			if (CheckPieceIndex() == 2)
			{
				//Set the bCanMoveDown boolean to false. //This stops the piece from moving vertically
				bCanMoveDown = false;
				//reset the current time Variable
				currentTime = 0;

				//If the player is fast dropping the current block. Do not allow them to slide the block, instead place it immediately.
				if (gameHandler->bIsFastDrop) 
				{
					PlaceBlock();
				}
				else //If the player is not fast dropping the block. Allow the player 1 / 10th of a second the move the current block left or right.
				{
					//Set the bCanMoveDown boolean to false. //This stops the piece from moving vertically
					bCanMoveDown = false;
					//increment the blockPlaceTimer variable.
					blockPlaceTimer += DeltaTime;

					//If the block places timer is equal to or greater than 0.1...
					if (blockPlaceTimer >= 0.1) {
						//Place the block and reset the timer.
						PlaceBlock();
						blockPlaceTimer = 0;
					}
				}	
			} // If the bCanMoveDown variable is true and currentTime variable is greater than the move down frequency and block in the row below is not occupied...
			else if (bCanMoveDown && currentTime >= gameHandler->moveDownFrequency && gameHandler->playField[currentRow + 1][currentColumn].piece == nullptr)
			{
				//Call the down movement function and reset the timer.
				DownMovement();
				currentTime = 0.0f;
			}
		}
	}
}

void ABasePiece::DownMovement() 
{
	//If the block is currently not placed. Increment the current row of the block and play a sound.
	if (!isPlaced) 
	{
		currentRow += 1;
		gameHandler->PlayBlockMoveSound();
	}
}

int ABasePiece::CheckPieceIndex()
{
	//return 0 = default return statement
	//return 1 = Out of Bounds of playfield << This should never happen
	//return 2 = This block or another in the tetromino has hit bottom of playfield or other block
	//return 3 = Hit top of playfield thus ending the game
	
	//For each block comprising the tetrominoe...
	for (int i = 0; i < 4; i++)
	{
		//Check if any other of the blocks comprising the tetrominoe have been placed, if so place this block. (check returns types key above)
		if (gameHandler->pieceController->block[i]->isPlaced)
		{
			return 2;
		}
	}
	
	//If the block has exceeded the left or right bounds of the playfield, return 1 (Out of Bounds)
	if (currentRow < 0 || currentColumn > 10)
	{
		return 1;
	}
	//If the block has exceeded the top or bottom bounds of the playfield, return 1 (Out of Bounds)
	else if (currentRow > 19 || currentColumn < 0)
	{
		return 1;
	} 
	//If the block is on the bottom row of the playfield, place the block
	else if (currentRow == 19) 
	{
		return 2;
	}
	//If the index in the playfield below this blocks current index contains a block, place this block.
	else if (currentRow < 19 && gameHandler->playField[currentRow + 1][currentColumn].isPlaced) 
	{
		return 2;
	}
	else { return 0; }
	

	return 0;
}

void ABasePiece::PlaceBlock()
{
	//Iterate through all 4 blocks comprising the tetrominoe and set their isPlaced boolean to true and call SetPosition active for all 4 blocks. (This is done so if any one of the 4 blocks collides with another block in the playfield. All 4 of the blocks are placed immediately.
	for (int i = 0; i < 4; i++)
	{
		gameHandler->pieceController->block[i]->isPlaced = true;
		gameHandler->SetPositionActive(gameHandler->pieceController->block[i], gameHandler->pieceController->block[i]->currentRow, gameHandler->pieceController->block[i]->currentColumn);
	}
}

