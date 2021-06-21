// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardLogic.h"
#include "EditorRessourceBank.h"
#include "Pathfinding.h"


#include "EngineUtils.h"

BoardLogic::BoardLogic(AChessBoard* board, UWorld* world)
    : mBoardActor(board)
    , mWorld(world)
    , mTileInfos(mBoardActor->getNbTiles())
{
    CreateTiles();
    PlacePieces();
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
        if (weightedDestination->GetWeight() <= MAX_MOVEMENT)
        {
            oldTile->piece = nullptr;
            newTile->piece = piece;

            piece->SetActorLocation(tileDestination->GetGlobalPosition());
            mCurrentSelectionPathfinding.reset(nullptr);
            HideAllSelectors();
            isValidMove = true;
        }
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

void BoardLogic::HighlingPossiblePlacement(AChessPiece * piece)
{
    Pathfinding pathfinding(mTileInfos);
    auto tileInfo = mTileInfos.GetTileInfoFromPiece(piece);

    auto weightedTiles = pathfinding.GetWeightedTiles(tileInfo);

    HideAllSelectors();

    // My HighlingPossiblePlacement

    for (ChessTile tile : mTiles)
    {
		//write colum and row

        int32 CurentRow = tileInfo->tile->GetRow();
		int32 CurentColum = tileInfo->tile->GetColum();

        int32 FutureRow = tile.GetRow();
        int32 FutureColum = tile.GetColum();

        switch (piece->getPieceType())
        {
        case EPieces::PE_PAWN:
        {
            // Wants to move forward
            if (FutureColum == CurentColum)
            {
                // Simple move forward
                ChessTile* CurentTile = &tile;

                //                    bool isWhite = mTileInfos.GetTileInfoFromTile(CurentTile)->piece->IsWhite();
                if (piece->IsWhite() && FutureRow == CurentRow + 1 || !piece->IsWhite() && FutureRow == CurentRow - 1)
                {
                    if (!mTileInfos.GetTileInfo(FutureRow, FutureColum)->piece)
                    {
                        mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//                        tile.SetSelectorVisibility(true);
                        //        					bValid = true;
                    }
                }

                // Double move forward
                else if (piece->IsWhite() && FutureRow == CurentRow + 2 || !piece->IsWhite() && FutureRow == CurentRow - 2)
                {
                    // This is only allowed if the pawn is in its original place
                    if (piece->IsWhite())
                    {
                        if (!mTileInfos.GetTileInfo(FutureRow - 1, FutureColum)->piece &&
                            !mTileInfos.GetTileInfo(FutureRow, FutureColum)->piece &&
                            1 == CurentRow)
                        {
                            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//                            tile.SetSelectorVisibility(true);
                            //        						bValid = true;
                            //         					    }
                        }
                    }
                    else /*if (!piece->IsWhite())*/
                    {
                        if (!mTileInfos.GetTileInfo(FutureRow + 1, FutureColum)->piece &&
                            !mTileInfos.GetTileInfo(FutureRow, FutureColum)->piece &&
                            6 == CurentRow)
                        {
                            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//                                tile.SetSelectorVisibility(true);
                            //         						    bValid = true;
                        }
                    }
                    
//          			else
//          			{
//          				// This is invalid
//          				return false;
//          			}
//         		}

                // The "en passant" move
//          		else if ((Chess::isWhitePiece(chPiece) && 4 == CurentRow && 5 == FutureRow && 1 == abs(FutureColum - CurentColum)) ||
//          			(Chess::isBlackPiece(chPiece) && 3 == CurentRow && 2 == FutureRow && 1 == abs(FutureColum - CurentColum)))
//          		{
//          			// It is only valid if last move of the opponent was a double move forward by a pawn on a adjacent column
//          			string last_move = current_game->getLastMove();
//          
//          			// Parse the line
//          			Chess::Position LastMoveFrom;
//          			Chess::Position LastMoveTo;
//          			current_game->parseMove(last_move, &LastMoveFrom, &LastMoveTo);
//          
//          			// First of all, was it a pawn?
//          			char chLstMvPiece = current_game->getPieceAtPosition(LastMoveTo.iRow, LastMoveTo.iColumn);
//          
//          			if (toupper(chLstMvPiece) != 'P')
//          			{
//          				return false;
//          			}
//          
//          			// Did the pawn have a double move forward and was it an adjacent column?
//          			if (2 == abs(LastMoveTo.iRow - LastMoveFrom.iRow) && 1 == abs(LastMoveFrom.iColumn - CurentColum))
//          			{
//          				cout << "En passant move!\n";
//          				bValid = true;
//          
//          				S_enPassant->bApplied = true;
//          				S_enPassant->PawnCaptured.iRow = LastMoveTo.iRow;
//          				S_enPassant->PawnCaptured.iColumn = LastMoveTo.iColumn;
//          			}
//          		}
//          
//          		// Wants to capture a piece
//          		else if (1 == abs(FutureColum - CurentColum))
//          		{
//          			if ((Chess::isWhitePiece(chPiece) && FutureRow == CurentRow + 1) || (Chess::isBlackPiece(chPiece) && FutureRow == CurentRow - 1))
//          			{
//          				// Only allowed if there is something to be captured in the square
//          				if (EMPTY_SQUARE != current_game->getPieceAtPosition(FutureRow, FutureColum))
//          				{
//          					bValid = true;
//          					cout << "Pawn captured a piece!\n";
//          				}
//          			}
//          		}
//          		else
//          		{
//          			// This is invalid
//          			return false;
//          		}
//          
//          		// If a pawn reaches its eight rank, it must be promoted to another piece
//          		if ((Chess::isWhitePiece(chPiece) && 7 == FutureRow) ||
//          			(Chess::isBlackPiece(chPiece) && 0 == FutureRow))
//          		{
//          			cout << "Pawn must be promoted!\n";
//          			S_promotion->bApplied = true;
                }
            }
        }
        break;
        case EPieces::PE_ROOK:
			// Horizontal move
			if ((FutureRow == CurentRow) && (FutureColum != CurentColum))
			{
				// TODO Check if there are no pieces on the way
//   				if (isPathFree(CurentRow, CurentColum, FutureRow, FutureColum, Direction::HORIZONTAL))
//   				{
                 isPathFree(CurentRow, CurentColum, FutureRow, FutureColum);
//                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//				    }
			}
			// Vertical move
			else if ((FutureRow != CurentRow) && (FutureColum == CurentColum))
			{
				// TODO Check if there are no pieces on the way
// 				if (current_game->isPathFree(present, future, Chess::VERTICAL))
// 				{
                 isPathFree(CurentRow, CurentColum, FutureRow, FutureColum);
//                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//				}
			}
            break;
        case EPieces::PE_KNIGHT:
		{
			{
				if ((2 == abs(FutureRow - CurentRow)) && (1 == abs(FutureColum - CurentColum)))
				{
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
				}

				else if ((1 == abs(FutureRow - CurentRow)) && (2 == abs(FutureColum - CurentColum)))
				{
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
				}
			}
		}
            break;
        case EPieces::PE_BISHOP:
			// Diagonal move
			if (abs(FutureRow - CurentRow) == abs(FutureColum - CurentColum))
			{
				// TODO Check if there are no pieces on the way
// 				if (current_game->isPathFree(present, future, Chess::DIAGONAL))
// 				{
                isPathFree(CurentRow, CurentColum, FutureRow, FutureColum);
//                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//				}
			}
            break;
        case EPieces::PE_QUEEN:
			// Horizontal move
			if ((FutureRow == CurentRow) && (FutureColum != CurentColum))
			{
				// TODO Check if there are no pieces on the way
// 				if (current_game->isPathFree(present, future, Chess::HORIZONTAL))
// 				{
                isPathFree(CurentRow, CurentColum, FutureRow, FutureColum);
//                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//				}
			}
			// Vertical move
			else if ((FutureRow != CurentRow) && (FutureColum == CurentColum))
			{
				// TODO Check if there are no pieces on the way
// 				if (current_game->isPathFree(present, future, Chess::VERTICAL))
// 				{
                isPathFree(CurentRow, CurentColum, FutureRow, FutureColum);
//                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//				}
			}

			// Diagonal move
			else if (abs(FutureRow - CurentRow) == abs(FutureColum - CurentColum))
			{
				// TODO Check if there are no pieces on the way
// 				if (current_game->isPathFree(present, future, Chess::DIAGONAL))
// 				{
                isPathFree(CurentRow, CurentColum, FutureRow, FutureColum);
//                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
//				}
			}
            break;
        case EPieces::PE_KING:
			// Horizontal move by 1
			if ((FutureRow == CurentRow) && (1 == abs(FutureColum - CurentColum)))
			{
                mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
			}

			// Vertical move by 1
			else if ((FutureColum == CurentColum) && (1 == abs(FutureRow - CurentRow)))
			{
                mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
			}

			// Diagonal move by 1
			else if ((1 == abs(FutureRow - CurentRow)) && (1 == abs(FutureColum - CurentColum)))
			{
                mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);
			}

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
    }

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

//bool BoardLogic::isPathFree(Position startingPos, Position finishingPos, int iDirection)
bool BoardLogic::isPathFree(int32 CurentRow, int32 CurentColum, int32 FutureRow, int32 FutureColum/*, Direction / *int32* / iDirection*/)
{
	//bool bFree = false;
    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);

// 	switch (iDirection)
// 	{
// 	case Direction::HORIZONTAL:
// 	{
		// If it is a horizontal move, we can assume the CurentRow == FutureRow
		// If the piece wants to move from column 0 to column 7, we must check if columns 1-6 are free
		if (CurentColum == FutureColum)
		{
//			cout << "Error. Movement is horizontal but column is the same\n";
		}

		// Moving to the right
		else if (CurentColum < FutureColum)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = CurentColum + 1; i < FutureColum; i++)
			{
				if (mTileInfos.GetTileInfo(CurentRow, i)->piece)
				{
// 					bFree = false;
// 					cout << "Horizontal path to the right is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}

		// Moving to the left
		else //if (CurentColum > FutureColum)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = CurentColum - 1; i > FutureColum; i--)
			{
				if (mTileInfos.GetTileInfo(CurentRow, i)->piece)
				{
// 					bFree = false;
// 					cout << "Horizontal path to the left is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}
//	}
// 	break;
// 
// 	case Direction::VERTICAL:
// 	{
		// If it is a vertical move, we can assume the CurentColum == FutureColum
		// If the piece wants to move from column 0 to column 7, we must check if columns 1-6 are free
		if (CurentRow == FutureRow)
		{
// 			cout << "Error. Movement is vertical but row is the same\n";
// 			throw("Error. Movement is vertical but row is the same");
		}

		// Moving up
		else if (CurentRow < FutureRow)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = CurentRow + 1; i < FutureRow; i++)
			{
				if (mTileInfos.GetTileInfo(i, CurentColum)->piece)
				{
// 					bFree = false;
// 					cout << "Vertical path up is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}

		// Moving down
		else //if (CurentColum > FutureRow)
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = CurentRow - 1; i > FutureRow; i--)
			{
				if (mTileInfos.GetTileInfo(i, CurentColum)->piece)
				{
// 					bFree = false;
// 					cout << "Vertical path down is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}
//	}
// 	break;
// 
// 	case Direction::DIAGONAL:
// 	{
		// Moving up and right
		if ((FutureRow > CurentRow) && (FutureColum > CurentColum))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = 1; i < abs(FutureRow - CurentRow); i++)
			{
				if (mTileInfos.GetTileInfo(CurentRow + i, CurentColum + i)->piece)
				{
// 					bFree = false;
// 					cout << "Diagonal path up-right is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}

		// Moving up and left
		else if ((FutureRow > CurentRow) && (FutureColum < CurentColum))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = 1; i < abs(FutureRow - CurentRow); i++)
			{
				if (mTileInfos.GetTileInfo(CurentRow + i, CurentColum - i)->piece)
				{
// 					bFree = false;
// 					cout << "Diagonal path up-left is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}

		// Moving down and right
		else if ((FutureRow < CurentRow) && (FutureColum > CurentColum))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = 1; i < abs(FutureRow - CurentRow); i++)
			{
				if (mTileInfos.GetTileInfo(CurentRow - i, CurentColum + i)->piece)
				{
					//bFree = false;
					//cout << "Diagonal path down-right is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}

		// Moving down and left
		else if ((FutureRow < CurentRow) && (FutureColum < CurentColum))
		{
			// Settting bFree as initially true, only inside the cases, guarantees that the path is checked
			//bFree = true;
            mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(true);

			for (int i = 1; i < abs(FutureRow - CurentRow); i++)
			{
				if (mTileInfos.GetTileInfo(CurentRow - i, CurentColum - i)->piece)
				{
// 					bFree = false;
// 					cout << "Diagonal path down-left is not clear!\n";
                    mTileInfos.GetTileInfo(FutureRow, FutureColum)->tile->SetSelectorVisibility(false);
				}
			}
		}

// 		else
// 		{
// 			throw("Error. Diagonal move not allowed");
// 		}
// 	}
// 	break;
// 	}

//	return bFree;
return true;
}