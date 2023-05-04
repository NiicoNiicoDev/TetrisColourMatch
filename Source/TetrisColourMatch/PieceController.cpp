// Fill out your copyright notice in the Description page of Project Settings.

#include "PieceController.h"
#include "BasePiece.h"
#include "GameHandler.h"

// Sets default values
APieceController::APieceController()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void APieceController::BeginPlay()
{
	Super::BeginPlay();
	gameHandler = Cast<AGameHandler>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameHandler::StaticClass()));
}

void APieceController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Binds input actions from the UE4 Standard player controller
	InputComponent->BindAction("MoveRight", IE_Pressed, this, &APieceController::MoveRight);
	InputComponent->BindAction("MoveLeft", IE_Pressed, this, &APieceController::MoveLeft);
	InputComponent->BindAction("RotateRight", IE_Pressed, this, &APieceController::RotateRight);
	InputComponent->BindAction("RotateLeft", IE_Pressed, this, &APieceController::RotateLeft);
	
	InputComponent->BindAction("QuickDrop", IE_Pressed, this, &APieceController::QuickDropPressed);
	InputComponent->BindAction("QuickDrop", IE_Released, this, &APieceController::QuickDropReleased);
}

// Called every frame
void APieceController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APieceController::MoveRight()
{
	//For each block that comprises a Tetrominoe
	for (size_t i = 0; i < 4; i++)
	{
		//if any of the blocks are currently in column 9 (right edge of the playfield) then return
		if (block[i]->currentColumn >= 9) { 
			return;
		}

		//if the block to the right of the current block is already placed then return
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn + 1].isPlaced == true)
		{
			return;
		}
		
	}

	// if none of the above conditions are met then...
	for (size_t i = 0; i < 4; i++) 
	{
		//Check if the block to the right of the current block is empty and if the current block is not null
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn + 1].piece == nullptr && block[i] != nullptr)
		{
			//move the current block to the right
			block[i]->currentColumn += 1;
		}
	}
}

void APieceController::MoveLeft()
{
	//For each block that comprises a Tetrominoe
	for (size_t i = 0; i < 4; i++)
	{
		//if any of the blocks are currently in column 0 (left edge of the playfield) then return
		if (block[i]->currentColumn <= 0) 
		{
			return;
		}

		//if the block to the left of the current block is already placed then return
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn - 1].isPlaced == true)
		{
			return;
		}
	}

	// if none of the above conditions are met then...
	for (size_t i = 0; i < 4; i++) 
	{
		//Check if the block to the left of the current block is empty and if the current block is not null
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn - 1].piece == nullptr && block[i] != nullptr)
		{
			//move the current block to the left
			block[i]->currentColumn -= 1;
		}
	}
	
}

void APieceController::RotateRight() 
{
	//Check if the gameHandler data and pieceController data match
	gameHandler->CheckPieceDataMismatch();
	
	//if the current rotation is less than 3 (270 degrees) then increment the rotation index
	if (gameHandler->currentTetrominoe.currentRotation < 3)
	{
		gameHandler->currentTetrominoe.currentRotation += 1;
	}
	else //otherwise set the rotation index to 0 (0 degrees)
	{
		gameHandler->currentTetrominoe.currentRotation = 0;
	}
	
	//Call the set piece rotation function in the game handler passing in the current tetrominoe index and the current rotation index
	gameHandler->SetPieceRotation(gameHandler->currentTetrominoe.TetrominoIndex, gameHandler->currentTetrominoe.currentRotation);
}

void APieceController::RotateLeft()
{
	//Check if the gameHandler data and pieceController data match
	gameHandler->CheckPieceDataMismatch();
	
	//if the current rotation is greater than 0 (90 degrees) then decrement the rotation index
	if (gameHandler->currentTetrominoe.currentRotation > 0)
	{
		gameHandler->currentTetrominoe.currentRotation -= 1;
	}
	else //otherwise set the rotation index to 3 (270 degrees)
	{
		gameHandler->currentTetrominoe.currentRotation = 3;
	}
	
	//Call the set piece rotation function in the game handler passing in the current tetrominoe index and the current rotation index
	gameHandler->SetPieceRotation(gameHandler->currentTetrominoe.TetrominoIndex, gameHandler->currentTetrominoe.currentRotation);

}



void APieceController::QuickDropPressed() 
{
	//When the S or Down Arrows keys are pressed, increase the rate at which the pieces fall...
	gameHandler->moveDownFrequency = 0.05f;
	//and set the bIsFastDrop boolean to true.
	gameHandler->bIsFastDrop = true;
}

void APieceController::QuickDropReleased()
{
	//When the S or Down Arrows keys are released, reset the rate at which the pieces fall...
	gameHandler->moveDownFrequency = 0.9 - (gameHandler->currentLevel * 0.03);
	//and set the bIsFastDrop boolean to false.
	gameHandler->bIsFastDrop = false;
}

