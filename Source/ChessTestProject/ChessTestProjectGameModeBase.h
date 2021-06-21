// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Logic/BoardLogic.h"
#include "ChessTestProjectGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class CHESSTESTPROJECT_API AChessTestProjectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	void StartPlay() override;
	void EndCurrentPlayerTurn();

	UPROPERTY(BlueprintReadWrite)
	TArray<AChessPiece*> ChessPieces;

	UFUNCTION(BlueprintCallable)
	TArray<AChessPiece*> GetChessPieces();

	// returns false if the move was invalid
	bool playTurn(AChessPiece* piece, UStaticMeshComponent* destination);
	UFUNCTION(BlueprintCallable)
	void ShowPiecePossibleMovement(AChessPiece* piece);
	bool IsWhiteTurn() const;

private:
	std::unique_ptr<BoardLogic> mBoardLogic;
	bool mIsWhiteTurn = true;

	AChessBoard* getChessBoard() const;

	void EndOfTurn();
	// set the pipeline to look at the right place
	void LookAtSide();
	void LookAtCenter();
};
