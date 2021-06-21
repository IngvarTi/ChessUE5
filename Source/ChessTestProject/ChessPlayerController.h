// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Actors/ChessPiece.h"
#include "ChessPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CHESSTESTPROJECT_API AChessPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	void SetupInputComponent() override;

private:
	void rotateCamera();
	void ProcessMouseClick();

	AChessPiece* mSelectedPiece = nullptr;
};
