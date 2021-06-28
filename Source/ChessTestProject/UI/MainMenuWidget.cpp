// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayBtn)
	{
		PlayBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
	}
	if (RestartBtn)
	{
		RestartBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnRestartClicked);
	}
	if (QuitBtn)
	{
		QuitBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
	if (QuitBtn)
	{
		QuitBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
	if (PauseBtn)
	{
		PauseBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPauseClicked);
	}

	// RulesBtn binding in bp
}

void UMainMenuWidget::OnPlayClicked()
{
	// start play
	//RemoveFromParent();
}

void UMainMenuWidget::OnRestartClicked()
{
	// restart
	//RemoveFromParent();
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Type::Quit, false);
}

void UMainMenuWidget::OnPauseClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Pause Presed"));
}
