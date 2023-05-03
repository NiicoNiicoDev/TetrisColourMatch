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
	for (size_t i = 0; i < 4; i++)
	{
		/*for (int j = 0; j < 4; j++)
		{
			if (block[i]->currentColumn +1 == block[j]->currentColumn)
			{
				return;
			}
		}*/

		if (block[i]->currentColumn >= 9) { 
			UE_LOG(LogTemp, Log, TEXT("can't move"));
			return;
		}

		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn + 1].isPlaced == true)
		{
			UE_LOG(LogTemp, Log, TEXT("can't move"));
			return;
		}
		
	}

	for (size_t i = 0; i < 4; i++) 
	{
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn + 1].piece == nullptr && block[i] != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("move right"));
			block[i]->currentColumn += 1;
		}
	}
}

void APieceController::MoveLeft()
{
	for (size_t i = 0; i < 4; i++)
	{
		/*for (int j = 0; j < 4; j++)
		{
			if (block[i]->currentColumn - 1 == block[j]->currentColumn) 	
			{
				return;
			}
		}*/

		if (block[i]->currentColumn <= 0) 
		{
			UE_LOG(LogTemp, Log, TEXT("can't move"));
			return;
		}

		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn - 1].isPlaced == true)
		{
			UE_LOG(LogTemp, Log, TEXT("can't move"));
			return;
		}
	}

	for (size_t i = 0; i < 4; i++) 
	{
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn - 1].piece == nullptr && block[i] != nullptr)
		{
			block[i]->currentColumn -= 1;
		}
	}
	
}

void APieceController::RotateRight() 
{
	gameHandler->CheckPieceDataMismatch();
	
	if (gameHandler->currentTetrominoe.currentRotation < 3)
	{
		gameHandler->currentTetrominoe.currentRotation += 1;
	}
	else
	{
		gameHandler->currentTetrominoe.currentRotation = 0;
	}

	UE_LOG(LogTemp, Log, TEXT("rotation index: %d"), gameHandler->currentTetrominoe.currentRotation);
	gameHandler->SetPieceRotation(gameHandler->currentTetrominoe.TetrominoIndex, gameHandler->currentTetrominoe.currentRotation);
}

void APieceController::RotateLeft()
{
	gameHandler->CheckPieceDataMismatch();
	
	if (gameHandler->currentTetrominoe.currentRotation > 0)
	{
		gameHandler->currentTetrominoe.currentRotation -= 1;
	}
	else
	{
		gameHandler->currentTetrominoe.currentRotation = 3;
	}
	
	gameHandler->SetPieceRotation(gameHandler->currentTetrominoe.TetrominoIndex, gameHandler->currentTetrominoe.currentRotation);

}

void APieceController::QuickDropPressed() 
{
	previousDropSpeed = gameHandler->moveDownFrequency;
	gameHandler->moveDownFrequency = 0.05f;
	gameHandler->bIsFastDrop = true;
}

void APieceController::QuickDropReleased()
{
	gameHandler->moveDownFrequency = previousDropSpeed;
	gameHandler->bIsFastDrop = false;
}

