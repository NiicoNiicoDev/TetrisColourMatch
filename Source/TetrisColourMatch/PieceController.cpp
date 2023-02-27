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
		if (block[i]->currentColumn > 9)
			return;
		
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn + 1].piece 
			== nullptr && block[i] != nullptr) 
		{
			block[i]->currentColumn += 1;
		}
	}
}

void APieceController::MoveLeft()
{
	for (size_t i = 4; i > 0; i--)
	{
		if (block[i]->currentColumn < 0)
			return;
		
		if (gameHandler->playField[block[i]->currentRow][block[i]->currentColumn - 1].piece
			== nullptr && block[i] != nullptr)
		{
			block[i]->currentColumn -= 1;
		}
	}
}

