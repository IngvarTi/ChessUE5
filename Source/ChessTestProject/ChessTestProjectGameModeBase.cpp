// Copyright Epic Games, Inc. All Rights Reserved.


#include "ChessTestProjectGameModeBase.h"
#include "ChessBoard.h"
#include "Kismet/GameplayStatics.h"



void AChessTestProjectGameModeBase::StartPlay()
{
	if (auto board = getChessBoard())
	{
		mBoardLogic = std::make_unique<BoardLogic>(board, GetWorld());
	}

	ChessPieces = mBoardLogic->GetChessPieces();

	Super::StartPlay();
}

AChessBoard* AChessTestProjectGameModeBase::getChessBoard() const
{
	AChessBoard* board = nullptr;
	if (auto world = GetWorld())
	{
		TArray<AActor*> foundActors;
		UGameplayStatics::GetAllActorsOfClass(world, AChessBoard::StaticClass(), foundActors);

		if (foundActors.Num() > 0)
		{
			board = Cast<AChessBoard>(foundActors[0]);
		}
	}

	if (!board)
	{
		UE_LOG(LogTemp, Warning, TEXT("AChessProjectUE4GameMode::getChessBoard() board not found"));
	}
	return board;
}

void AChessTestProjectGameModeBase::EndCurrentPlayerTurn()
{
	mIsWhiteTurn = !mIsWhiteTurn;

	if (mIsWhiteTurn)
	{
		EndOfTurn();
	}
	LookAtSide();
}

void AChessTestProjectGameModeBase::EndOfTurn()
{
	// TODO
	// this is where we will move the pieces
}

void AChessTestProjectGameModeBase::LookAtSide()
{
	if (auto board = getChessBoard())
	{
		board->switchCamera(mIsWhiteTurn);
	}
}

void AChessTestProjectGameModeBase::LookAtCenter()
{
	// TODO
	// set camera from the middle to see the moves
}

TArray<AChessPiece*> AChessTestProjectGameModeBase::GetChessPieces()
{
	return ChessPieces;
}

bool AChessTestProjectGameModeBase::playTurn(AChessPiece* piece, UStaticMeshComponent* destination)
{
	bool isMoveValid = false;
	if (piece->IsWhite() == mIsWhiteTurn)
	{
		if (auto tile = mBoardLogic->getChessTileFromComponent(destination))
		{
			isMoveValid = mBoardLogic->MovePiece(piece, tile);
			if (isMoveValid)
			{
				mIsWhiteTurn = !mIsWhiteTurn;
				FTimerDelegate timedDelegate = FTimerDelegate::CreateLambda([=]()
					{
						this->LookAtSide();
					});

				FTimerHandle switchCameraTimer;
				GetWorldTimerManager().SetTimer(switchCameraTimer, timedDelegate, .3f, false);
			}
		}
	}
	return isMoveValid;
}

void AChessTestProjectGameModeBase::ShowPiecePossibleMovement(AChessPiece* piece)
{
	mBoardLogic->HighlingPossiblePlacement(piece);
}

bool AChessTestProjectGameModeBase::IsWhiteTurn() const
{
	return mIsWhiteTurn;
}