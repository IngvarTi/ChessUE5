// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChessTestProject/Actors/ChessBoard.h"
#include "ChessTestProject/Actors/ChessPiece.h"
#include "ChessTile.h"

#include "TileInformations.h"
#include "Pathfinding.h"

#include <memory>

class AChessBoard;
//class AChessPiece;



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

//USTRUCT()
struct FMove
{
//	GENERATED_USTRUCT_BODY()

	AChessPiece* Pieces;
	int32 FromRow;
	int32 FromColumn;
	int32 ToRow;
	int32 ToColumn;
};

class CHESSTESTPROJECT_API BoardLogic
{
    AChessBoard* mBoardActor;       // non-owning ptr
    UWorld* mWorld;                 // non-owning ptr

    TArray<AChessPiece*> mPieces;  // the owner of the all the pieces
    TArray<AChessPiece*> mWhitePieces;
    TArray<AChessPiece*> mBlackPieces;
    AChessPiece* mPiecesPosition [8][8]; // mPiecesPosition[Row][Colum]

    TArray<ChessTile> mTiles;
    TileInformations mTileInfos;
    std::unique_ptr<WeightedTiles> mCurrentSelectionPathfinding;
    TArray<AChessPiece*> piecesAvailableForCapture;

public:
    BoardLogic()/* = default*/;
    BoardLogic(AChessBoard* board, UWorld* world);

    //returns false if move is invalid
    bool MovePiece(AChessPiece* piece, ChessTile * tileDestination);
    bool IsValidMove(AChessPiece* piece, ChessTile * tileDestination);
    ChessTile * getChessTileFromComponent(UStaticMeshComponent * component);

    bool isRightColor(AChessPiece * piece, bool isWhite);
    TArray<AChessPiece*> HighlingPossiblePlacement(AChessPiece * piece);

    bool isSimpleMove(int32 FutureRow, int32 FutureColum, AChessPiece* piece);


    TileInformations GetTileInfos();

    TArray<AChessPiece*> GetChessPieces();

	void SetLastMove(FMove Move);
	FMove GetLastMove();


private:
    TArray<FMove> AllMoves;
    TArray<AChessPiece*> CapturedPiece;
//    TArray<TPair<int32, int32>> AvailablePlaces;
    TArray<ChessTile*> AvailableTiles;
    bool enPassantMove;
    bool isCheck;

    void CreateTiles();
    void PlacePieces();
    void HideAllSelectors();
    bool isPathFree(int32 CurentRow, int32 CurentColum, bool isWhitePiece, Direction MoveDirection);

    bool CheckPath(int32 Row, int32 Colum, bool isWhitePiece);


};
