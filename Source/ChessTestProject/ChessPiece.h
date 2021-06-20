// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ChessPiece.generated.h"

UENUM(BlueprintType)
enum class EPieces : uint8
{
	PE_PAWN         UMETA(DisplayName = "Pawn"),
	PE_ROOK         UMETA(DisplayName = "Rook"),
	PE_KNIGHT       UMETA(DisplayName = "Knight"),
	PE_BISHOP       UMETA(DisplayName = "Bishop"),
	PE_QUEEN        UMETA(DisplayName = "Queen"),
	PE_KING         UMETA(DisplayName = "King"),
};

UCLASS()
class CHESSTESTPROJECT_API AChessPiece : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AChessPiece();

	void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
//	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// TODO Something special for battle...
 //      like when into battle
 //      or when charging
 //      or when diying

	void setMaterial(bool isBlack);
	void setSelected(bool isSelected);

	int GetMovementValue() const;
	bool IsWhite()const;
	void setIsWhite(bool isWhite);
private:
	void loadMaterials();

	UStaticMeshComponent* getMeshRoot() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Logic")
	UStaticMesh* mPieceMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Logic")
	int mMovementRange = 0;

	// Moral will represent the HP of the piece
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Logic")
	int mMorale = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Logic")
	EPieces mPieceType;

	bool mIsWhite = true;

};
