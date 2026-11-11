// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayAbilityTargetActor_URResolveTarget.generated.h"

enum class EURFaction : uint8;
/**
 * 
 */
UCLASS()
class URGAME_API AGameplayAbilityTargetActor_URResolveTarget : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
	AGameplayAbilityTargetActor_URResolveTarget(const FObjectInitializer& ObjectInitializer);
	
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	
	virtual void ConfirmTargetingAndContinue() override;
	
protected:
	virtual FGameplayAbilityTargetDataHandle MakeTargetData() const;
};
