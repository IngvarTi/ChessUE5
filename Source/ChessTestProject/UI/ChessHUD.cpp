// Fill out your copyright notice in the Description page of Project Settings.


#include "ChessHUD.h"

void AChessHUD::BeginPlay()
{
	Super::BeginPlay();

	MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), _mainMenuWidget);
	MainMenuWidget->AddToViewport();
}
