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

	FVector targetLocation = FVector(0, currentColumn * 100, 2000 - ((currentRow + 1) * 100));

	if (currentRow < 0) 
		return;

	if (currentRow > 19)
		return;

	if (!isPlaced && !gameHandler->playField[currentRow + 1][currentColumn].isPlaced && currentRow < 19)
	{
		currentTime += DeltaTime;

		if (currentTime >= gameHandler->moveDownFrequency && gameHandler->playField[currentRow + 1][currentColumn].piece == nullptr)
		{
			if (!isPlaced)
			{
				DownMovement();
			}

			currentTime = 0.0f;
		}
		else
		{
			currentTime += DeltaTime;
		}

		if (CheckPieceIndex(targetLocation) == 0)
			SetActorLocation(targetLocation);
	}
}

void ABasePiece::DownMovement() 
{
	currentRow += 1;
}

//Standard downward movement for all pieces
void ABasePiece::StandardMovement() 
{
	FVector targetLocation = FVector(0, currentColumn * 100, GetActorLocation().Z);

	SetActorLocation(targetLocation);
}

int ABasePiece::CheckPieceIndex(FVector targetLocation)
{
	// Check if the piece is in the grid
	if (currentRow < 0 || currentColumn > 10) 
	{
		return 1;
	}
	
	//Check the gameHandler to see if the current row / column is occupied
	if (currentRow == 19 || gameHandler->playField[currentRow + 1][currentColumn].isPlaced)
	{
		for (size_t i = 0; i < 4; i++)
		{
			gameHandler->pieceController->block[i]->PlaceBlock();
		}
		
		return 2;
	}

	return 0;
}

void ABasePiece::PlaceBlock()
{
	UE_LOG(LogTemp, Log, TEXT("Placed"));
	gameHandler->SetPositionActive(this, currentRow, currentColumn);
	isPlaced = true;
}

