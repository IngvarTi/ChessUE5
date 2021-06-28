// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHESSTESTPROJECT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void NativeConstruct() override;

protected:

	UPROPERTY(meta = (BindWidgetOptional))
	UButton * PlayBtn;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton * RestartBtn;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton * RulesBtn;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton * QuitBtn;
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* PauseBtn;

	UFUNCTION()
	void OnPlayClicked();
	UFUNCTION()
	void OnRestartClicked();
	UFUNCTION()
	void OnQuitClicked();
	UFUNCTION()
	void OnPauseClicked();
};
