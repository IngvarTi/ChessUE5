// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainMenuWidget.h"
#include "ChessHUD.generated.h"

/**
 * 
 */
UCLASS()
class CHESSTESTPROJECT_API AChessHUD : public AHUD
{
	GENERATED_BODY()

	void BeginPlay();

	UMainMenuWidget* MainMenuWidget;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuWidget> _mainMenuWidget;
};
