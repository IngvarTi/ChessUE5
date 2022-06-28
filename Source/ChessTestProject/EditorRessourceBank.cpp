// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorRessourceBank.h"

#include <algorithm>

EditorRessourceBank * EditorRessourceBank::mSingleton = nullptr;

namespace
{
    template <class C, class I>
    void AddElement(typename std::vector<typename std::pair<I, C>> & toAddInto, I key, FString path)
    {
        ConstructorHelpers::FObjectFinder<typename std::remove_pointer<C>::type> element(*path);

        if (element.Succeeded())
        {
            toAddInto.push_back({ key, element.Object });
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid : %s"), *path);
        }
    }
}

EditorRessourceBank::EditorRessourceBank()
{
    // must be done in constructor... to test later on
    AddElement(mBlueprints, RessourceBlueprints::PIECE_PAWN, FString("Blueprint'/Game/Blueprints/Pieces/BP_Pawn.BP_Pawn'"));
    AddElement(mBlueprints, RessourceBlueprints::PIECE_ROOK, FString("Blueprint'/Game/Blueprints/Pieces/BP_Rook.BP_Rook'"));
    AddElement(mBlueprints, RessourceBlueprints::PIECE_KNIGHT, FString("Blueprint'/Game/Blueprints/Pieces/BP_Knight.BP_Knight'"));
    AddElement(mBlueprints, RessourceBlueprints::PIECE_BISHOP, FString("Blueprint'/Game/Blueprints/Pieces/BP_Bishop.BP_Bishop'"));
    AddElement(mBlueprints, RessourceBlueprints::PIECE_QUEEN, FString("Blueprint'/Game/Blueprints/Pieces/BP_Queen.BP_Queen'"));
    AddElement(mBlueprints, RessourceBlueprints::PIECE_KING, FString("Blueprint'/Game/Blueprints/Pieces/BP_King.BP_King'"));

//     BlueprintPath.Add(FString("Blueprint'/Game/Blueprints/Pieces/BP_Pawn.BP_Pawn'"));
//     BlueprintPath.Add(FString("Blueprint'/Game/Blueprints/Pieces/BP_Rook.BP_Rook'"));
//     BlueprintPath.Add(FString("Blueprint'/Game/Blueprints/Pieces/BP_Knight.BP_Knight'"));
//     BlueprintPath.Add(FString("Blueprint'/Game/Blueprints/Pieces/BP_Bishop.BP_Bishop'"));
//     BlueprintPath.Add(FString("Blueprint'/Game/Blueprints/Pieces/BP_Queen.BP_Queen'"));
//     BlueprintPath.Add(FString("Blueprint'/Game/Blueprints/Pieces/BP_King.BP_King'"));

}

//EditorRessourceBank::~EditorRessourceBank()
//{
//}

UBlueprint*/*UClass**/ EditorRessourceBank::GetBlueprintRessource(/*RessourceBlueprints type*/int32 currentPiece)
{
//    auto & blueprints = mSingleton->mBlueprints;

//    UObject* BPObject = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/Blueprints/Pieces/BP_Rook.BP_Rook'"));
    UObject* BPObject = nullptr;
	switch (currentPiece)
	{
	case 1:
        BPObject = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/Blueprints/Pieces/BP_Pawn.BP_Pawn'"));
		break;
	case 2:
        BPObject = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/Blueprints/Pieces/BP_Rook.BP_Rook'"));
		break;
	case 3:
        BPObject = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/Blueprints/Pieces/BP_Knight.BP_Knight'"));
		break;
	case 4:
        BPObject = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/Blueprints/Pieces/BP_Bishop.BP_Bishop'"));
		break;
	case 5:
        BPObject = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/Blueprints/Pieces/BP_Queen.BP_Queen'"));
		break;
	case 6:
        BPObject = StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("Blueprint'/Game/Blueprints/Pieces/BP_King.BP_King'"));
		break;
	default:
		break;
	}

//     auto findByType = [type](std::pair<RessourceBlueprints, UBlueprint*> elem)
//     {
//         return elem.first == type;
//     };
// 
//     auto result = std::find_if(blueprints.cbegin(), blueprints.cend(), findByType);
// 
//     if (result != blueprints.cend())
//     {
//         return (*result).second;
//     }


      UBlueprint* PieceBlueprint = Cast<UBlueprint>(BPObject);
      if (PieceBlueprint)
      {
          return PieceBlueprint;
      }

// 	  if (castedBlueprint && castedBlueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
// 	  {
// 		  // Don't forget the asterisk
// 		  spawnedClass = *castedBlueprint->GeneratedClass;
// 	  }

    return nullptr;
}
