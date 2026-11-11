// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCharactersSet.generated.h"

USTRUCT(BlueprintType)
struct FPlayerCharacterInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CharacterCode;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<class UTexture2D> Portrait;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class AURPlayerCharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<class USkeletalMesh> CharacterMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<class UAnimMontage> SelectionAnimMontage;
};

/**
 * 
 */
UCLASS(BlueprintType)
class URGAME_API UPlayerCharactersSet : public UDataAsset
{
	GENERATED_BODY()

public:
	FPlayerCharacterInfo* FindByCharacterCode(FName CharacterCode);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FPlayerCharacterInfo> Characters;
};
