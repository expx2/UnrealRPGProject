// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/URGameplayTags.h"
#include "URAbilitySystemComponent.generated.h"

class UURAbilityTagRelationshipMapping;

namespace EGameplayAbilityInputTriggerPolicy
{
	enum Type : int;
}

namespace EAbilityInputBinds
{
	enum Type : int;
}

// Note: OnRep_ActivateAbilities에선 추가/제거된 어빌리티의 Spec 정보가 없어 파라미터 삭제하고 이름 변경함
// DECLARE_MULTICAST_DELEGATE_OneParam(FActiveAbilityChangedDelegate, FGameplayAbilitySpec&, Spec);
DECLARE_MULTICAST_DELEGATE(FAbilityListChangedDelegate);

/**
 * 
 */
UCLASS()
class URGAME_API UURAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UURAbilitySystemComponent();
	
	virtual FActiveGameplayEffectHandle ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& GameplayEffect, FPredictionKey PredictionKey = FPredictionKey()) override;
	
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	
	void AbilityInputTagTriggered(const EAbilityInputBinds::Type& InputBinding);
	
	void AbilityInputTagReleased(const EAbilityInputBinds::Type& InputBinding);
	
	void ClearAbilityInput();

	// ~Begin Tag Relationship Mapping
	
	/** Sets the current tag relationship mapping, if null it will clear it out */
	void SetTagRelationshipMapping(UURAbilityTagRelationshipMapping* NewMapping);
	
	/** Looks at ability tags and gathers additional required and blocking tags */
	void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;
	
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	
	// ~End Tag Relationship Mapping
	
	/** 입력에 바인딩된 어빌리티 중 사용 조건과 우선 순위를 검사해 현재 사용할 어빌리티를 리턴한다. */
	FGameplayAbilitySpecHandle ResolveCurrentAbilityForInput(const EAbilityInputBinds::Type& InputBinding) const;
	
	void Respawn();
	
	/** 기존 NotifyAbilityFailed 대신 사용되도록 하고 PredictionKey를 사용해 서버와 클라이언트에서 중복으로 실패를 전송하는 것을 방지한다 */
	void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, const int16 PredictionKey, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
	
	UFUNCTION(BlueprintCallable, Category = GameplayEffects)
	virtual FGameplayEffectContextHandle MakeDamageEffectContext() const;
	
	bool IsDead() const { return HasMatchingGameplayTag(URGameplayTags::Character_State_Dead); }
	
	bool HasAvatarActor() const { return IsValid(GetAvatarActor()); }
	
	/** 어빌리티 추가/제거 시 호출. 한 번에 여러 개가 변경되어도 한 번만 호출한다. */
	FAbilityListChangedDelegate OnAbilityListChanged;
	
protected:
	virtual void InternalServerTryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData) override;
	
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	virtual void OnRep_ActivateAbilities() override;
	
	void QueueAbilityListChanged();
	
	void BroadcastAbilityListChanged();
	
	// Confirm/Cancel에 바인딩되어 소모된 InputID를 Released 전까지 Trigger 되지 않도록 캐싱함
	TSet<int32> ConsumedInputIDs;
	
	// If set, this table is used to look up tag relationships for activate and cancel
	UPROPERTY()
	TObjectPtr<UURAbilityTagRelationshipMapping> TagRelationshipMapping;
	
	// 서버에서 TryActivateAbility할 때 잠시 저장할 PredictionKey 값
	int16 ActivatingPredictionKeyOnServer = 0;
	
	// 어빌리티 변화 델리게이트 호출 타이머 핸들
	FTimerHandle AbilityListChangedTimer;
};
