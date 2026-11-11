// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_LookTarget.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UAbilityTask_LookTarget : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	// UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_LookTarget* LookTarget(UGameplayAbility* OwningAbility, AActor* TargetActor);
	
	// UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_LookTarget* LookTarget(UGameplayAbility* OwningAbility, const FVector& InTargetLocation);
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
protected:
	virtual void Activate() override;
	
	TWeakObjectPtr<AActor> TargetActorPtr;
	FVector TargetLocation;
	TWeakObjectPtr<class UURCharacterMovementComponent> MovementComponent;
};
