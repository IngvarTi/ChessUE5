// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChessTestProject/Actors/ChessBoard.h"
#include "ChessTestProject/Actors/ChessPiece.h"
#include "ChessTile.h"

#include "TileInformations.h"
#include "Pathfinding.h"

#include <memory>

class AChessBoard;

enum Direction
{
	HORIZONTAL = 0,
	VERTICAL,
	DIAGONAL,
	L_SHAPE
};

class CHESSTESTPROJECT_API ChessMove
{
    AChessPiece * mPiece;
    TArray<ChessTile*> mPath;

public:

};

/**
 * BoardLogic

    class containing the logic of the board
        - movement
        - tiles
        - pieces
 */

class CHESSTESTPROJECT_API BoardLogic
{
    AChessBoard* mBoardActor;       // non-owning ptr
    UWorld* mWorld;                 // non-owning ptr

    TArray<AChessPiece*> mPieces;  // the owner of the all the pieces
    TArray<AChessPiece*> mWhitePieces;
    TArray<AChessPiece*> mBlackPieces;

    TArray<ChessTile> mTiles;
    TileInformations mTileInfos;
    std::unique_ptr<WeightedTiles> mCurrentSelectionPathfinding;

public:
    BoardLogic() = default;
    BoardLogic(AChessBoard* board, UWorld* world);

    //returns false if move is invalid
    bool MovePiece(AChessPiece* piece, ChessTile * tileDestination);
    bool IsValidMove(AChessPiece* piece, ChessTile * tileDestination);
    ChessTile * getChessTileFromComponent(UStaticMeshComponent * component);

    bool isRightColor(AChessPiece * piece, bool isWhite);
    void HighlingPossiblePlacement(AChessPiece * piece);

    //test
    TArray<AChessPiece*> GetChessPieces();

private:
    void CreateTiles();
    void PlacePieces();
    void HideAllSelectors();
    bool isPathFree(int32 CurentRow, int32 CurentColum, int32 FutureRow, int32 FutureColum/*, Direction / *int32* / iDirection*/);
};