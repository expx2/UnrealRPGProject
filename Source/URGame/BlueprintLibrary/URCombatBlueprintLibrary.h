// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Combat/URHitEffect.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "URCombatBlueprintLibrary.generated.h"

enum class EURFaction : uint8;
class UAbilitySystemComponent;
/**
 * 타격 판정과 대미지/부가효과 전달의 공통 진입점.
 */
UCLASS()
class URGAME_API UURCombatBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="UR|Combat")
	static EURFaction GetRelativeTargetFaction(EURFaction OriginFaction, const AActor* SourceActor);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="UR|Combat")
	static void ResolveTargets(
		TArray<AActor*>& TargetableActors, 
		int MaxTargetCount, 
		AActor* MainTargetActor,
		TArray<AActor*>& PriorTargets,
		TArray<AActor*>& OutHitActors);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="UR|Combat")
	static bool Hit(
		UAbilitySystemComponent* SourceASC, 
		AActor* TargetActor,
		const TArray<FURHitEffect>& HitEffects, 
		const FGameplayEffectContextHandle& EffectContext);
};
