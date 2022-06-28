// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessPlayerController.h"
#include "ChessTestProjectGameModeBase.h"
#include "ChessTestProject/Actors/ChessBoard.h"
#include "Kismet/GameplayStatics.h"

void AChessPlayerController::BeginPlay()
{
	if (auto world = GetWorld())
	{
		TArray<AActor*> foundActors;
		UGameplayStatics::GetAllActorsOfClass(world, AChessBoard::StaticClass(), foundActors);

		if (foundActors.Num() > 0)
		{
			SetViewTarget(foundActors[0]);
		}
	}

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AChessPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::M, IE_Released, this, &AChessPlayerController::rotateCamera);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AChessPlayerController::ProcessMouseClick);
	//InputComponent->BindTouch(IE_Released, this, &AChessPlayerController::ProcessTouchClick);
	InputComponent->BindTouch(IE_Pressed, this, &AChessPlayerController::ProcessTouchClick);
}

void AChessPlayerController::rotateCamera()
{
	if (auto gameMode = Cast<AChessTestProjectGameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		gameMode->EndCurrentPlayerTurn();
	}
}

void AChessPlayerController::ProcessTouchClick(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, *FString::Printf(TEXT("Toch Click")));
	}
	GetHitResultUnderFinger(FingerIndex, ECollisionChannel::ECC_Camera, true, ClickAndTouchHit);
	auto actorHit = ClickAndTouchHit.GetActor();
	if (GEngine)
	{
		FString HitActorName = actorHit->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, *FString::Printf(TEXT("Toch Click Hit Actor %s"), *HitActorName));
	}
	ProcessClick();
}

void AChessPlayerController::ProcessMouseClick()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, *FString::Printf(TEXT("Mouse Click")));
	}
	// Trace to see what is under the mouse cursor
	GetHitResultUnderCursor(ECC_Visibility, true, ClickAndTouchHit);
	auto actorHit = ClickAndTouchHit.GetActor();
	if (GEngine)
	{
		FString HitActorName = actorHit->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, *FString::Printf(TEXT("Mouse Click Hit Actor %s"),*HitActorName));
	}
	ProcessClick();
}

void AChessPlayerController::ProcessClick()
{
	// Trace to see what is under the mouse cursor
	//GetHitResultUnderCursor(ECC_Visibility, true, ClickAndTouchHit);

	if (ClickAndTouchHit.bBlockingHit)
	{
		auto actorHit = ClickAndTouchHit.GetActor();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, *FString::Printf(TEXT("Hit Actor %s"), *actorHit->GetName()));
		}
		if (auto chessPiece = Cast<AChessPiece>(actorHit))
		{
			if (mSelectedPiece && mSelectedPiece != chessPiece)
			{
				mSelectedPiece->setSelected(false);
			}
			mSelectedPiece = chessPiece;
			mSelectedPiece->setSelected(true);

			if (auto chessMode = Cast<AChessTestProjectGameModeBase>(GetWorld()->GetAuthGameMode()))
			{
				if (ActorCatched)
				{
					if (ActorCatched->IsWhite() != mSelectedPiece->IsWhite())
					{
						chessMode->playTurnGetPiese(ActorCatched, mSelectedPiece);
						ActorCatched = nullptr;
					}
					else
					{
						chessMode->ShowPiecePossibleMovement(mSelectedPiece);
						ActorCatched = mSelectedPiece;
					}
				}
				else
				{
					chessMode->ShowPiecePossibleMovement(mSelectedPiece);
					ActorCatched = mSelectedPiece;
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Selected : %s"), *chessPiece->GetName());
		}
		else if (auto chessBoard = Cast<AChessBoard>(actorHit))
		{
			ActorCatched = nullptr;
			auto component = ClickAndTouchHit.Component;
			if (component.IsValid()) // to be on safe side
			{
				// if it is the selector
				if (auto parent = Cast<UStaticMeshComponent>(component->GetAttachParent()))
				{
					component = parent;
				}

				if (component.IsValid() && mSelectedPiece)
				{
					if (auto chessMode = Cast<AChessTestProjectGameModeBase>(GetWorld()->GetAuthGameMode()))
					{
						if (chessMode->playTurn(mSelectedPiece, Cast<UStaticMeshComponent>(component.Get())))
						{
							mSelectedPiece->setSelected(false);
							mSelectedPiece = nullptr;
						}
					}
				}
			}
		}
	}
}