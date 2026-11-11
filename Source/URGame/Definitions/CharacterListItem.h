// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/PlayerCharactersSet.h"
#include "UObject/Object.h"
#include "CharacterListItem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class URGAME_API UCharacterListItem : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	FPlayerCharacterInfo Info;
};
