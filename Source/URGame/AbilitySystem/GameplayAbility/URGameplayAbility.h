// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "URGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UURGameplayAbility();
	
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	/** Tag Relationship을 포함한 소유자의 활성화 필수/차단 태그로 출력 컨테이너를 설정한다. */
	virtual void GetActivationTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, FGameplayTagContainer& OutRequiredTags, FGameplayTagContainer& OutBlockedTags) const;
	
	/** Call from blueprints to forcibly end the ability with canceling it. This will replicate the end ability to the client or server which can interrupt tasks */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName="End Ability As Cancelled", meta=(ScriptName = "EndAbilityAsCancelled"))
	void K2_EndAbilityAsCancelled();
	
	/** 어빌리티가 활성화될 때의 우선순위. 낮은 값일 수록 우선권을 가진다. */
	int GetPriority() const { return Priority; }
	
protected:
	/** 어빌리티가 활성화될 때의 우선순위. 낮은 값일 수록 우선권을 가진다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(DisplayPriority=0))
	int Priority = 100;
};
