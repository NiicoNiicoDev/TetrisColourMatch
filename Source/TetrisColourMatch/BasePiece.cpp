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

	if (!gameHandler->bGameOver && bDoTick)
	{
		FVector targetLocation = FVector(0, currentColumn * 100, 2000 - ((currentRow) * 100));

		SetActorLocation(targetLocation);

		if (currentRow < 0 || currentRow > 19)
			return;

		if (!isPlaced)
		{
			currentTime += DeltaTime;
			if (CheckPieceIndex() == 2)
			{
				PlaceBlock();

				return;
			} 
			else if (currentTime >= gameHandler->moveDownFrequency && gameHandler->playField[currentRow + 1][currentColumn].piece == nullptr)
			{
				currentRow += 1;
				gameHandler->PlayBlockMoveSound();

				currentTime = 0.0f;
			}
		}
	}
}

void ABasePiece::DownMovement() 
{
	if (!isPlaced) 
	{
		
	}
}

int ABasePiece::CheckPieceIndex()
{
	//return 0 = default return statement
	//return 1 = Out of Bounds of playfield << This should never happen
	//return 2 = This block or another in the tetromino has hit bottom of playfield or other block
	//return 3 = Hit top of playfield thus ending the game
	
	for (int i = 0; i < 4; i++)
	{
		if (gameHandler->pieceController->block[i]->isPlaced)
		{
			UE_LOG(LogTemp, Log, TEXT("Other Blocks are placed"));
			return 2;
		}
		
		if (currentRow < 0 || currentColumn > 10)
		{
			UE_LOG(LogTemp, Log, TEXT("OOB"));
			return 1;
		}
		else if (currentRow > 19 || currentColumn < 0)
		{
			UE_LOG(LogTemp, Log, TEXT("OOB"));
			return 1;
		} 
		else if (currentRow == 19) 
		{
			UE_LOG(LogTemp, Log, TEXT("Row = 19"));
			return 2;
		}
		else if (currentRow < 19 && gameHandler->playField[currentRow + 1][currentColumn].isPlaced) 
		{
			UE_LOG(LogTemp, Log, TEXT("Target Row is Occupied"));
			return 2;
		}
		else { return 0; }
	}

	return 0;
}

void ABasePiece::PlaceBlock()
{
	
}

