// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AIAbilityOrderSet.generated.h"

/**
 * 어빌리티 사용 순서
 */
UCLASS(BlueprintType)
class URGAME_API UAIAbilityOrderSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Ability.Active"))
	TArray<struct FGameplayTag> Order;
};
