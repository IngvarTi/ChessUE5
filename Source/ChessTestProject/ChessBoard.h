// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPiece.h"
#include "ChessTile.h"
#include "BoardLogic.h"
#include "GameFramework/SpringArmComponent.h"

#include "ChessBoard.generated.h"

UCLASS()
class CHESSTESTPROJECT_API AChessBoard : public AActor
{
	GENERATED_BODY()

        // The board will always be with the (queen) black rook piece at 0,0 (so top left)
        // and the (king) white rook at 7,7

        /*
        row :   0   1   2   3   4   5   6   7
        col 0   BR  Bk  BB  BQ  BK  BB  Bk  BR
        col 1   BP  BP  BP  BP  BP  BP  BP  BP
        col 2   E   E   E   E   E   E   E   E
        col 3   E   E   E   E   E   E   E   E
        col 4   E   E   E   E   E   E   E   E
        col 4   E   E   E   E   E   E   E   E
        col 6   WP  WP  WP  WP  WP  WP  WP  WP
        col 7   WR  Wk  WB  WQ  WK  WB  Wk  WR
        */
	
public:	
	// Sets default values for this actor's properties
	AChessBoard();

	void OnConstruction(const FTransform& Transform) override;

	void Tick(float DeltaSeconds) override;

	void switchCamera(bool lookingWhite);
	void setCameraMiddle();

	int getNbTiles() const;
	TArray<int> getPiecesPlacement() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void LoadCheckerMaterial();
	void CreateBoardLayout();
	void SetupComponents();
	void ComputeCameraSettings();
	void ComputeCameraLocation();
//	void PlacePieces();

	USpringArmComponent* GetSpringArm();

	UPROPERTY(EditAnywhere, Category = "Board")
	TArray<int> mPiecesPlacement;      // where are the pieces

	UPROPERTY(EditAnywhere, Category = "Board")
	int mNbTiles = 64;      // must be a square of a number

	FVector mCenterLocation;

	FVector mWhiteLookAtLocation;
	FVector mBlackLookAtLocation;
	FRotator mWhiteLookAtRotation;
	FRotator mBlackLookAtRotation;

	FVector mMiddleLookAtLocation;
	FRotator mMiddleLookAtRotation;

	FVector mCurrentLookAtLocation;
	FRotator mCurrentLookAtRotation;
	FVector mTargetLookAtLocation;
	FRotator mTargetLookAtRotation;

	float mTimeSwitching = 0.0f;
	bool mIsSwitchingCamera = false;

	// Copy from BoardLogic 
// 	TArray<ChessTile> mTiles;
// 	TileInformations mTileInfos;
};
