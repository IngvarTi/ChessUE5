// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Components/StaticMeshComponent.h>

/**
 *
 */
class CHESSTESTPROJECT_API ChessTile
{
public:
    ChessTile(UStaticMeshComponent* tile, UStaticMeshComponent* selector, int32 row, int32 colum, bool isblack)
        : mTile(tile)
        , mSelector(selector)
        , mRow(row)
        , mColum(colum)
        , isBlack(isblack)
    {
    }

    void SetSelectorVisibility(bool isVisible);
    FVector GetGlobalPosition() const;
    FRotator GetGlobalRotation() const;
    int32 GetRow();
    int32 GetColum();
    bool IsBlack();

    bool isSameTile(UStaticMeshComponent * tile);
    void setShadowMesh(UStaticMesh * mesh);
    UPROPERTY(BlueprintReadWrite)
    int32 mColum;
    UPROPERTY(BlueprintReadWrite)
    int32 mRow;
    UPROPERTY(BlueprintReadWrite)
    bool isBlack;

private:
    UStaticMeshComponent * mTile;
    UStaticMeshComponent * mSelector;
    UStaticMeshComponent * mShadownMesh;
};
