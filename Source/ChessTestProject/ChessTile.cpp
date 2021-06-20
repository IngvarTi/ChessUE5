// Fill out your copyright notice in the Description page of Project Settings.

#include "ChessTile.h"

void ChessTile::SetSelectorVisibility(bool isVisible)
{
    if (mSelector)
    {
        mSelector->SetVisibility(isVisible);
    }
}

FVector ChessTile::GetGlobalPosition() const
{
    return mTile->GetComponentLocation();
}

FRotator ChessTile::GetGlobalRotation() const
{
    return mTile->GetComponentRotation();
}


bool ChessTile::isSameTile(UStaticMeshComponent * tile)
{
    return mTile == tile;
}
void ChessTile::setShadowMesh(UStaticMesh * mesh)
{
    mShadownMesh->SetStaticMesh(mesh);
}

