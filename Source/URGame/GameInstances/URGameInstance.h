// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URGame.h"
#include "Definitions/URDefaults.h"
#include "Engine/GameInstance.h"
#include "URGameInstance.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class URGAME_API UURGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UClass* GetCharacterClass(const FName& CharacterCode) const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UR|Game")
	TObjectPtr<class UPlayerCharactersSet> PlayerCharactersSet;
	
	UPROPERTY(BlueprintReadWrite, Category="UR|Game")
	FName LocalSelectedCharacterCode = FURDefaults::DefaultCharacterCode;
};
