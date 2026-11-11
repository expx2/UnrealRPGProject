// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "URHitEffect.generated.h"

class UGameplayEffect;

/**
 * 히트 시 부여할 GE 정보
 */
USTRUCT(BlueprintType)
struct URGAME_API FURHitEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UR")
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UR", meta=(Categories="SetByCaller"))
	TMap<FGameplayTag, float> SetByCallerValues;
};
