// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardLogic.h"
#include "ChessTestProject/EditorRessourceBank.h"
#include "Pathfinding.h"


#include "EngineUtils.h"

BoardLogic::BoardLogic(AChessBoard* board, UWorld* world)
    : mBoardActor(board)
    , mWorld(world)
    , mTileInfos(mBoardActor->getNbTiles())
{
    CreateTiles();
    PlacePieces();
    enPassantMove = false;
}

void BoardLogic::CreateTiles()
{
    mTiles.Empty(0); // just to be sure
    mTiles.Reserve(mBoardActor->getNbTiles());

    auto root = mBoardActor->GetRootComponent();
    auto children = root->GetAttachChildren();
    bool isBlack = true;

    for (auto child : children)
    {
        if (auto tile = Cast<UStaticMeshComponent>(child))
        {
            if (auto selector = Cast<UStaticMeshComponent>(tile->GetChildComponent(0)))
            {

                // parse int, write colum and row to title
 				TArray<FString> Out;

                UStaticMeshComponent* selectorTile = Cast<UStaticMeshComponent>(tile->GetChildComponent(0));
                FString TileName = selectorTile->GetFName().ToString();
                TileName.ParseIntoArray(Out, TEXT("_"), true);

                int32 Row = FCString::Atoi(*Out[1]);
                int32 Colum = FCString::Atoi(*Out[2]);

                mTiles.Add({ tile, selector, Row, Colum, isBlack });

				isBlack = !isBlack;
				if (Colum % 8 == 0/*column == 8*/)
				{
					isBlack = !isBlack;
				}
            }
        }
    }

    for (auto & tile : mTiles)
    {
        tile.SetSelectorVisibility(false);
    }
}

void BoardLogic::PlacePieces()
{
    auto piecesPlacement = mBoardActor->getPiecesPlacement();
    FVector zDelta(0.f, 0.f, 20.f);

    // personal note : if there is no values associated with Roll/Pitch/Yah,
    //                  there will be garbage values
    FRotator blackRotation(0.f, 180.f, 0.f);
    const int MAX_INDEX_ROW = mTileInfos.GetMaxIndex();
    // set pieces to right spot
    for (int i = 0; i < piecesPlacement.Num(); ++i)
    {
        AChessPiece * chessPiece = nullptr;
        auto tileInfo = mTileInfos.GetTileInfo(i / MAX_INDEX_ROW, i % MAX_INDEX_ROW);
        int currentPlacement = piecesPlacement[i];
        int currentPiece = currentPlacement % 10;
        bool isBlack = currentPlacement > 10;

        auto & currentTile = mTiles[i];
        tileInfo->tile = &currentTile;

        auto spawnPosition = currentTile.GetGlobalPosition() + zDelta;
        auto spawnRotation = currentTile.GetGlobalRotation();

        if (isBlack)
        {
            spawnRotation += blackRotation;
        }

        if (currentPiece != 0)
        {
            using RessourceBlueprints = EditorRessourceBank::RessourceBlueprints;

            RessourceBlueprints type = RessourceBlueprints::PIECE_PAWN;

            switch (currentPiece)
            {
                case 1:
                    type = RessourceBlueprints::PIECE_PAWN;
                    break;
                case 2:
                    type = RessourceBlueprints::PIECE_ROOK;
                    break;
                case 3:
                    type = RessourceBlueprints::PIECE_KNIGHT;
                    break;
                case 4:
                    type = RessourceBlueprints::PIECE_BISHOP;
                    break;
                case 5:
                    type = RessourceBlueprints::PIECE_QUEEN;
                    break;
                case 6:
                    type = RessourceBlueprints::PIECE_KING;
                    break;
                default:
                    break;
            }

            if (auto pieceActor = EditorRessourceBank::GetBlueprintRessource(/*type*/currentPiece))
            {
                chessPiece = mWorld->SpawnActor<AChessPiece>(
                    pieceActor->GeneratedClass,
                    spawnPosition,
                    spawnRotation);

                if (chessPiece)
                {
                    chessPiece->setIsWhite(!isBlack);
                    mPieces.Add(chessPiece);
                    tileInfo->piece = chessPiece;
                    if (isBlack)
                    {
                        mBlackPieces.Add(chessPiece);
                    }
                    else
                    {
                        mWhitePieces.Add(chessPiece);
                    }
                }
            }
        }
    }
}

bool BoardLogic::MovePiece(AChessPiece* piece, ChessTile * tileDestination)
{
    bool isValidMove = false;
    auto oldTile = mTileInfos.GetTileInfoFromPiece(piece);
    auto newTile = mTileInfos.GetTileInfoFromTile(tileDestination);
    

    if (mCurrentSelectionPathfinding)
    {
        auto weightedDestination = mCurrentSelectionPathfinding->GetWeightedTileFromTile(tileDestination);
        const int MAX_MOVEMENT = piece->GetMovementValue();
//         if (weightedDestination->GetWeight() <= MAX_MOVEMENT)
//         {
            // set last move
		    ChessTile* PieceTile = GetTileInfos().GetTileInfoFromPiece(piece)->tile;
		    SetLastMove(FMove{ piece, PieceTile->GetRow(), PieceTile->GetColum(), tileDestination->GetRow(), tileDestination->GetColum() });
		    UE_LOG(LogTemp, Warning, TEXT("Piese - %s move from Row - %d Colum - %d to Row - %d Colum - %d"),
			    *piece->GetName(), PieceTile->GetRow(), PieceTile->GetColum(), tileDestination->GetRow(), tileDestination->GetColum());

            

            oldTile->piece = nullptr;
            newTile->piece = piece;

            piece->SetActorLocation(tileDestination->GetGlobalPosition());

            
			if (piece->getPieceType() == EPieces::PE_PAWN)
			{
				if (enPassantMove)
				{
				    for (AChessPiece* CPiece : CapturedPiece)
				    {
					    if (CPiece->getPieceType() == EPieces::PE_PAWN)
					    {
						    if (piece->IsWhite() && newTile->tile->GetRow() == mTileInfos.GetTileInfoFromPiece(CPiece)->tile->GetRow() + 1 ||
                                !piece->IsWhite() && newTile->tile->GetRow() == mTileInfos.GetTileInfoFromPiece(CPiece)->tile->GetRow() - 1)
						    {
							    // TODO Checks Arrays
							    CPiece->Destroy();
						    }
					    }
				    }
			    }
                else if(piece->IsWhite() && tileDestination->GetRow() == 7 || !piece->IsWhite() && tileDestination->GetRow() == 0)
                {
                    // TODO If a pawn reaches its eight rank, it must be promoted to another piece
                    UE_LOG(LogTemp, Warning, TEXT("Pawn must be promoted"));
                }
            }
            mCurrentSelectionPathfinding.reset(nullptr);
            HideAllSelectors();
            isValidMove = true;
//        }
    }

    return isValidMove;
}

bool BoardLogic::IsValidMove(AChessPiece* piece, ChessTile * tileDestination)
{
    bool isValidMove = false;

    if (mCurrentSelectionPathfinding)
    {
        auto weightedDestination = mCurrentSelectionPathfinding->GetWeightedTileFromTile(tileDestination);
        const int MAX_MOVEMENT = piece->GetMovementValue();
        isValidMove = weightedDestination->GetWeight() <= MAX_MOVEMENT;
    }

    return isValidMove;
}

ChessTile * BoardLogic::getChessTileFromComponent(UStaticMeshComponent * component)
{
    auto foundTile = mTiles.FindByPredicate([component](ChessTile & tile)
    {
        return tile.isSameTile(component);
    });

    return foundTile;
}

bool BoardLogic::isRightColor(AChessPiece * piece, bool isWhite)
{
    if (isWhite)
    {
        return mWhitePieces.Contains(piece);
    }
    return mBlackPieces.Contains(piece);
}

bool BoardLogic::isSimpleMove(int32 FutureRow, int32 FutureColum, AChessPiece* piece)
{
    if (FutureRow < 8 && FutureRow >= 0 && FutureColum < 8 && FutureColum >= 0)
    {
        if (mPiecesPosition[FutureRow][FutureColum])
        {
            if (piece->getPieceType() != EPieces::PE_PAWN)
            {
                if (piece->IsWhite() != mPiecesPosition[FutureRow][FutureColum]->IsWhite())
                {
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
                    CapturedPiece.Add(mTileInfos.GetTileInfo(FutureRow, FutureColum)->piece);
                    AvailableTiles.Add(mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile);
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (!mPiecesPosition[FutureRow][FutureColum])
        {
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
            AvailableTiles.Add(mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile);
            return true;
        }
    }
    return false;
}

void BoardLogic::HighlingPossiblePlacement(AChessPiece * piece)
{
    Pathfinding pathfinding(mTileInfos);
    auto tileInfo = mTileInfos.GetTileInfoFromPiece(piece);

    auto weightedTiles = pathfinding.GetWeightedTiles(tileInfo);

    CapturedPiece.Empty();
//    AvailablePlaces.Empty();
    AvailableTiles.Empty();

    HideAllSelectors();

    // My HighlingPossiblePlacement

	int32 CurentRow = tileInfo->tile->GetRow();
	int32 CurentColum = tileInfo->tile->GetColum();

    int32 mTilesNum = mTiles.Num();

	// Clear Array TODO make better
	for (int32 r = 0; r < 8; r++)
	{
		for (int32 c = 0; c < 8; c++)
		{
			mPiecesPosition[r][c] = nullptr;
		}
	}
    for (int32 i = 0; i < mTilesNum; i++)
    {

        //write colum and row
 		int32 FutureRow = mTiles[i].GetRow();
 		int32 FutureColum = mTiles[i].GetColum();

        mPiecesPosition[FutureRow][FutureColum] = mTileInfos.GetTileInfo(FutureRow, FutureColum)->piece;
//        UE_LOG(LogTemp, Warning, TEXT("Row - %d  Colum - %d"), FutureRow, FutureColum);
        if (mPiecesPosition[FutureRow][FutureColum])
        {
            UE_LOG(LogTemp, Warning, TEXT("Chess  - %s   Row - %d  Colum - %d"), *mPiecesPosition[FutureRow][FutureColum]->GetName(), FutureRow, FutureColum);
        }
        
    }

	int32 FutureRow = CurentRow;
	int32 FutureColum = CurentColum;

            switch (piece->getPieceType())
            {
            case EPieces::PE_PAWN:
            {
                enPassantMove = false;
                // Simple move forward
                FutureRow = piece->IsWhite() ? CurentRow + 1 : CurentRow - 1;

                if (isSimpleMove(FutureRow, FutureColum, piece))
                {
                    // if the Row is equal to the spawn Row of "Pawn"
                    if (piece->IsWhite() && CurentRow == 1 || !piece->IsWhite() && CurentRow == 6)
                    {
						// Double move forward
						FutureRow = piece->IsWhite() ? CurentRow + 2 : CurentRow - 2;
						isSimpleMove(FutureRow, FutureColum, piece);
                    }
                }

                // The "en passant" move
                // right
                AChessPiece* PieceAtFuturePos;
                PieceAtFuturePos = mPiecesPosition[CurentRow][CurentColum + 1];
                if (PieceAtFuturePos)
                {
                    if (piece->IsWhite() != PieceAtFuturePos->IsWhite())
                    {
                        FMove LastMove = GetLastMove();
                        if (LastMove.Pieces->getPieceType() == EPieces::PE_PAWN)
                        {
                            if (PieceAtFuturePos == LastMove.Pieces && abs(LastMove.FromRow - LastMove.ToRow) == 2)
                            {
                                if (piece->IsWhite())
                                { 
                                    mTileInfos.GetTileInfo(CurentRow + 1, CurentColum + 1)->tile->SetSelectorVisibility(true);
                                    //AvailablePlaces.Add(TPairInitializer<int32,int32>(CurentRow + 1, CurentColum + 1));
                                    AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow + 1, CurentColum + 1)->tile);
                                }
                                else
                                { 
                                    mTileInfos.GetTileInfo(CurentRow - 1, CurentColum + 1)->tile->SetSelectorVisibility(true);
                                    AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow + 1, CurentColum + 1)->tile);
                                }
                                
                                CapturedPiece.Add(PieceAtFuturePos);
                                enPassantMove = true;
                            }
                        }
                    }
                }
                // left
				PieceAtFuturePos = mPiecesPosition[CurentRow][CurentColum - 1];
				if (PieceAtFuturePos)
				{
					if (piece->IsWhite() != PieceAtFuturePos->IsWhite())
					{
						FMove LastMove = GetLastMove();
						if (LastMove.Pieces->getPieceType() == EPieces::PE_PAWN)
						{
							if (PieceAtFuturePos == LastMove.Pieces && abs(LastMove.FromRow - LastMove.ToRow) == 2)
							{
                                if (piece->IsWhite())
                                { 
                                    mTileInfos.GetTileInfo(CurentRow + 1, CurentColum - 1)->tile->SetSelectorVisibility(true);
                                    AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow + 1, CurentColum - 1)->tile);
                                }
                                else
                                {
                                    mTileInfos.GetTileInfo(CurentRow - 1, CurentColum - 1)->tile->SetSelectorVisibility(true);
                                    AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow - 1, CurentColum - 1)->tile);
                                }
                                CapturedPiece.Add(PieceAtFuturePos);
                                enPassantMove = true;
							}
						}
					}
				}
          		// Wants to capture a piece
                if (piece->IsWhite())
                {
                    // right
                    PieceAtFuturePos = mPiecesPosition[CurentRow + 1][CurentColum + 1];
                    if (PieceAtFuturePos)
                    {
                        if (piece->IsWhite() != PieceAtFuturePos->IsWhite())
                        {
							mTileInfos.GetTileInfo(CurentRow + 1, CurentColum + 1)->tile->SetSelectorVisibility(true);
                            AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow + 1, CurentColum + 1)->tile);
							CapturedPiece.Add(PieceAtFuturePos);
                        }
                    }
                    // left
					PieceAtFuturePos = mPiecesPosition[CurentRow + 1][CurentColum - 1];
					if (PieceAtFuturePos)
					{
						if (piece->IsWhite() != PieceAtFuturePos->IsWhite())
						{
							mTileInfos.GetTileInfo(CurentRow + 1, CurentColum - 1)->tile->SetSelectorVisibility(true);
                            AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow + 1, CurentColum - 1)->tile);
							CapturedPiece.Add(PieceAtFuturePos);
						}
					}
                }
				if (!piece->IsWhite())
				{
					// right
                    if (CurentRow > 0 && CurentColum <= 6)
                    {
					    PieceAtFuturePos = mPiecesPosition[CurentRow - 1][CurentColum + 1];
					    if (PieceAtFuturePos)
					    {
						    if (piece->IsWhite() != PieceAtFuturePos->IsWhite())
						    {
							    mTileInfos.GetTileInfo(CurentRow - 1, CurentColum + 1)->tile->SetSelectorVisibility(true);
                                AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow - 1, CurentColum + 1)->tile);
							    CapturedPiece.Add(PieceAtFuturePos);
						    }
					    }
                    }
					// left
                    if (CurentRow > 0 && CurentColum < 0)
                    {
                        PieceAtFuturePos = mPiecesPosition[CurentRow - 1][CurentColum - 1];
                        if (PieceAtFuturePos)
                        {
                            if (piece->IsWhite() != PieceAtFuturePos->IsWhite())
                            {
                                mTileInfos.GetTileInfo(CurentRow - 1, CurentColum - 1)->tile->SetSelectorVisibility(true);
                                AvailableTiles.Add(mTileInfos.GetTileInfo(CurentRow - 1, CurentColum - 1)->tile);
                                CapturedPiece.Add(PieceAtFuturePos);
                            }
                        }
                    }
				}
            }
            break;
            case EPieces::PE_ROOK:
			    // Horizontal move
                isPathFree(CurentRow, CurentColum, piece->IsWhite(), Direction::HORIZONTAL);
			    // Vertical move
                isPathFree(CurentRow, CurentColum, piece->IsWhite(), Direction::VERTICAL);
                break;
            case EPieces::PE_KNIGHT:
		    {
                isSimpleMove(CurentRow + 2, CurentColum + 1, piece);
                isSimpleMove(CurentRow + 2, CurentColum - 1, piece);
                isSimpleMove(CurentRow + 1, CurentColum + 2, piece);
                isSimpleMove(CurentRow + 1, CurentColum - 2, piece);
                isSimpleMove(CurentRow - 1, CurentColum + 2, piece);
                isSimpleMove(CurentRow - 1, CurentColum - 2, piece);
                isSimpleMove(CurentRow - 2, CurentColum + 1, piece);
                isSimpleMove(CurentRow - 2, CurentColum - 1, piece);
		    }
                break;
             case EPieces::PE_BISHOP:
// 			    // Diagonal move
                 isPathFree(CurentRow, CurentColum, piece->IsWhite(), Direction::DIAGONAL);
                 break;
             case EPieces::PE_QUEEN:
				 // Horizontal move
				 isPathFree(CurentRow, CurentColum, piece->IsWhite(), Direction::HORIZONTAL);
				 // Vertical move
				 isPathFree(CurentRow, CurentColum, piece->IsWhite(), Direction::VERTICAL);
// 			     // Diagonal move
 			     isPathFree(CurentRow, CurentColum, piece->IsWhite(), Direction::DIAGONAL);
                 break;
             case EPieces::PE_KING:
 			     // Horizontal move by 1
                 isSimpleMove(CurentRow, CurentColum + 1, piece);
                 isSimpleMove(CurentRow, CurentColum - 1, piece);
 			     // Vertical move by 1
                 isSimpleMove(CurentRow + 1, CurentColum, piece);
                 isSimpleMove(CurentRow - 1, CurentColum, piece);
 
 			     // Diagonal move by 1
				 isSimpleMove(CurentRow + 1, CurentColum + 1, piece);
				 isSimpleMove(CurentRow - 1, CurentColum - 1, piece);
				 isSimpleMove(CurentRow - 1, CurentColum + 1, piece);
				 isSimpleMove(CurentRow + 1, CurentColum - 1, piece);
 
 			    // Castling
     // 			else if ((FutureRow == CurentRow) && (2 == abs(FutureColum - CurentColum)))
     // 			{
     // 				// Castling is only allowed in these circunstances:
     // 
     // 				// 1. King is not in check
     // 				if (true == current_game->playerKingInCheck())
     // 				{
     // 					return false;
     // 				}
     // 
     // 				// 2. No pieces in between the king and the rook
     // 				if (false == current_game->isPathFree(present, future, Chess::HORIZONTAL))
     // 				{
     // 					return false;
     // 				}
     // 
     // 				// 3. King and rook must not have moved yet;
     // 				// 4. King must not pass through a square that is attacked by an enemy piece
     // 				if (FutureColum > CurentColum)
     // 				{
     // 					// if FutureColum is greather, it means king side
     // 					if (false == current_game->castlingAllowed(Chess::Side::KING_SIDE, Chess::getPieceColor(chPiece)))
     // 					{
     // 						createNextMessage("Castling to the king side is not allowed.\n");
     // 						return false;
     // 					}
     // 					else
     // 					{
     // 						// Check if the square that the king skips is not under attack
     // 						Chess::UnderAttack square_skipped = current_game->isUnderAttack(CurentRow, CurentColum + 1, current_game->getCurrentTurn());
     // 						if (false == square_skipped.bUnderAttack)
     // 						{
     // 							// Fill the S_castling structure
     // 							S_castling->bApplied = true;
     // 
     // 							// Present position of the rook
     // 							S_castling->rook_before.iRow = CurentRow;
     // 							S_castling->rook_before.iColumn = CurentColum + 3;
     // 
     // 							// Future position of the rook
     // 							S_castling->rook_after.iRow = FutureRow;
     // 							S_castling->rook_after.iColumn = CurentColum + 1; // FutureColum -1
     // 
     //                             mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
     // 						}
     // 					}
     // 				}
     // 				else //if (FutureColum < CurentColum)
     // 				{
     // 					// if CurentColum is greather, it means queen side
     // 					if (false == current_game->castlingAllowed(Chess::Side::QUEEN_SIDE, Chess::getPieceColor(chPiece)))
     // 					{
     // 						createNextMessage("Castling to the queen side is not allowed.\n");
     // 						return false;
     // 					}
     // 					else
     // 					{
     // 						// Check if the square that the king skips is not attacked
     // 						Chess::UnderAttack square_skipped = current_game->isUnderAttack(CurentRow, CurentColum - 1, current_game->getCurrentTurn());
     // 						if (false == square_skipped.bUnderAttack)
     // 						{
     // 							// Fill the S_castling structure
     // 							S_castling->bApplied = true;
     // 
     // 							// Present position of the rook
     // 							S_castling->rook_before.iRow = CurentRow;
     // 							S_castling->rook_before.iColumn = CurentColum - 4;
     // 
     // 							// Future position of the rook
     // 							S_castling->rook_after.iRow = FutureRow;
     // 							S_castling->rook_after.iColumn = CurentColum - 1; // FutureColum +1
     // 
     //                             mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
     // 						}
     // 					}
     // 				}
     // 			}
                 break;
            default:
                break;
            }
//    }

//     for (auto & tile : weightedTiles.GetFlatArray())
//     {
//         int weight = tile->GetWeight();
//         if (weight <= piece->GetMovementValue() && weight > 0)
//         {
//             tile->GetTileInfo()->tile->SetSelectorVisibility(true);
//        }
//    }

    mCurrentSelectionPathfinding = std::make_unique<WeightedTiles>(std::move(weightedTiles));
}

TileInformations BoardLogic::GetTileInfos()
{
    return mTileInfos;
}

TArray<AChessPiece*> BoardLogic::GetChessPieces()
{
    return mPieces;
}

void BoardLogic::HideAllSelectors()
{
    for (auto & tile : mTiles)
    {
        tile.SetSelectorVisibility(false);
    }
}

bool BoardLogic::isPathFree(int32 CurentRow, int32 CurentColum, bool isWhitePiece, Direction MoveDirection)
{
	int32 FutureRow = CurentRow;
	int32 FutureColum = CurentColum;

	switch (MoveDirection)
	{
	    case Direction::HORIZONTAL:
	    {
		    // Moving to the right
			for (int i = CurentColum + 1; i <= 7; i++)
			{
                if (!CheckPath(CurentRow, i, isWhitePiece))
                {
                    break;
                }
			}
		// Moving to the left
			for (int i = CurentColum - 1; i >= 0; i--)
			{
				if (!CheckPath(CurentRow, i, isWhitePiece))
				{
					break;
				}
			}
	    }
	    break;
	    case Direction::VERTICAL:
	    {
		    // Moving up
			for (int i = CurentRow + 1; i <= 7; i++)
			{
				if (!CheckPath(i, CurentColum, isWhitePiece))
				{
					break;
				}
			}
		// Moving down
 			for (int i = CurentRow - 1; i >= 0; i--)
			{
				if (!CheckPath(i, CurentColum, isWhitePiece))
				{
					break;
				}
			}
	    }
	    break;
	    case Direction::DIAGONAL:
	    {
		    // Moving up and right
			for (int32 r = CurentRow + 1, c = CurentColum + 1; r <= 7 && c <= 7; r++, c++)
			{
				if (!CheckPath(r, c, isWhitePiece))
				{
					break;
				}
			}
		    // Moving up and left
			for (int32 r = CurentRow + 1, c = CurentColum - 1; r <= 7 && c >= 0; r++, c--)
			{
				if (!CheckPath(r, c, isWhitePiece))
				{
					break;
				}
			}
		    // Moving down and right
			for (int32 r = CurentRow - 1, c = CurentColum + 1; r >= 0 && c <= 7; r--, c++)
			{
				if (!CheckPath(r, c, isWhitePiece))
				{
					break;
				}
			}
		    // Moving down and left
			for (int32 r = CurentRow - 1, c = CurentColum - 1; r >= 0 && c >= 0; r--, c--)
			{
				if (!CheckPath(r, c, isWhitePiece))
				{
					break;
				}
			}
	    }
	    break;
	}
    return false;
}

bool BoardLogic::CheckPath(int32 Row, int32 Colum, bool isWhitePiece)
{
	AChessPiece* OtherPiece;
	OtherPiece = mTileInfos.GetTileInfo(Row, Colum)->piece;
	if (!OtherPiece)
	{
		mTileInfos.GetTileInfo(Row, Colum)->tile->SetSelectorVisibility(true);
        AvailableTiles.Add(mTileInfos.GetTileInfo(Row, Colum)->tile);
		return true;
	}
	else if (OtherPiece->IsWhite() != isWhitePiece)
	{
		mTileInfos.GetTileInfo(Row, Colum)->tile->SetSelectorVisibility(true);
        AvailableTiles.Add(mTileInfos.GetTileInfo(Row, Colum)->tile);
		CapturedPiece.Add(OtherPiece);
		return false;
	}
	else if (OtherPiece->IsWhite() == isWhitePiece)
	{
		mTileInfos.GetTileInfo(Row, Colum)->tile->SetSelectorVisibility(false);
        AvailableTiles.Add(mTileInfos.GetTileInfo(Row, Colum)->tile);
		return false;
	}
	return false;
}

void BoardLogic::SetLastMove(FMove Move)
{
	AllMoves.Add(Move);
}

FMove BoardLogic::GetLastMove()
{
	return AllMoves.Last();
}