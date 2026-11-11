// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility.h"
#include "Combat/AbilityAction/URAbilityActionBase.h"
#include "Definitions/URFaction.h"
#include "StructUtils/InstancedStruct.h"
#include "URGameplayAbility_ActiveAbility.generated.h"

struct FURAbilityActionBase;
struct FInstancedStruct;
enum class EURFaction : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FActiveAbilityEndedDelegate);

UENUM(BlueprintType)
namespace EAbilityTargetingPolicy
{
	enum Type : int
	{
		/** 타겟 있어야 사용 가능 */
		Targeting,
		
		/** 타겟 없어도 사용 가능하지만 타겟을 지정하고 사용할 경우, 타겟이 유효하지 않으면 사용 불가 */
		NonTargetingWithTarget,
		
		/** 타겟 없어도 사용 가능 */
		NonTargeting,
		
		/** 범위 지정 */
		Range,
	};
}

/**
 * 액티브 스킬 어빌리티로 FGameplayEventData를 받아 Trigger될 경우에만 활성화 가능하다. 
 * 이외의 경우엔 활성화를 막거나 활성화 후 즉시 종료됨. 
 */
UCLASS()
class URGAME_API UURGameplayAbility_ActiveAbility : public UURGameplayAbility
{
	GENERATED_BODY()
	
public:
	UURGameplayAbility_ActiveAbility();

	// --------------------------------------
	//	UGameplayAbility overrides
	// --------------------------------------
	
	/** 해당 어빌리티는 FGameplayEventData없이 활성화가 불가능하므로, 활성화가 아닌 활성화 가능성을 얻기 위해 호출할 경우 SourceTags에 빈 TagContainer라도 포함시켜 보낼 것. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual bool ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* Payload) const override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	
	virtual bool CommitCheck(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	
	bool CanActivateAbilityToTarget(const UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle Handle, AActor* Target, FGameplayTagContainer* OptionalRelevantTags = nullptr) const;
	
	UFUNCTION(BlueprintCallable, Category=Ability, DisplayName="Can Activate Ability To Target", meta=(ScriptName="CanActivateAbilityToTarget"))
	bool K2_CanActivateAbilityToTarget(const UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle Handle, AActor* Target) const;
	
	// --------------------------------------
	//	UURGameplayAbility overrides
	// --------------------------------------	
	
	virtual void GetActivationTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, FGameplayTagContainer& OutRequiredTags, FGameplayTagContainer& OutBlockedTags) const override;
	
	
	// --------------------------------------
	// --------------------------------------
	
	/** Activate 가능과 별개로 ASC의 ResolveCurrentAbilityForInput에 선택될 수 있는지 여부. Relationship과 Cost를 제외한 태그를 검사한다. */
	bool DoesAbilitySatisfyTagRequirementsToSelect(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const;

	/** 스킬 슬롯에 표시될 때 활성으로 표시할지 여부. Relationship과 Cooldown을 제외한 태그를 검사한다. */
	bool CanDisplayAsEnabled(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const;
	
	UFUNCTION(BlueprintCallable)
	bool IsValidTarget(const AActor* SourceActor, const AActor* Actor) const;
	
	/** FURDamageGameplayEffectContext 생성 */
	FGameplayEffectContextHandle MakeHitEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo) const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "UR|Ability")
	void ActivateCustomAbilityAction(class UAbilityActionTask_Custom* CustomTask, FURAbilityActionContext InContext, const float ElapsedTime );
	
	// --------------------------------------
	//	Accessors
	// --------------------------------------
	
	FGameplayTagContainer& GetActivationRequiredTags() { return ActivationRequiredTags; }
	
	EAbilityTargetingPolicy::Type GetTargetingPolicy() const { return TargetingPolicy; }

	EURFaction GetTargetFactions() const { return static_cast<EURFaction>(TargetFactions); }
	
	/** 연계 (후속) 스킬인지 여부 */
	bool IsChainAbility() const { return !ConsumeTags.IsEmpty(); }
	
	/**
	 * @param bIsForRemoteClient 서버일 경우 보정된 값을 반환한다 
	 */
	UFUNCTION(BlueprintPure, Category="UR|Ability")
	float GetUseRange(const bool bIsForRemoteClient) const;
	
	UTexture2D* GetAbilityIcon() { return AbilityIcon; }
	
	UPROPERTY(BlueprintAssignable, Category="UR|Ability")
	FActiveAbilityEndedDelegate OnAbilityEnded;

protected:
	// --------------------------------------
	//	UGameplayAbility overrides
	// --------------------------------------
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	// --------------------------------------
	//	EndAbility
	// --------------------------------------
	
	void EndAbilityAsFailed(const FGameplayTagContainer& FailureReason);
	
	void EndAbilityAsFailed(const FGameplayTag FailureReason)
	{
		const FGameplayTagContainer Tags(FailureReason);
		EndAbilityAsFailed(Tags);
	}
	
	void EndAbilityAsFailed()
	{
		EndAbilityAsFailed(FGameplayTagContainer());
	}
	
	void Cleanup();
	
	// --------------------------------------
	//	Validation & Range
	// --------------------------------------
	
	UFUNCTION()
	void ValidateRangeTargetData(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	void HandleTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data);
	
	bool CanActivateWithTarget(const AActor* TargetActor, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* FailureReason = nullptr) const;
	
	// --------------------------------------
	//	Commit
	// --------------------------------------
	
	bool CheckConsumeTag(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;
	
	/** ConsumeTags를 부여한 GE를 제거한다 */
	void ApplyConsumeTags(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo);
	
	void GrantInCombatGameplayEffect(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

	void GrantFollowupTag(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;
	
	// --------------------------------------
	//	Action (실제 실행)
	// --------------------------------------
	
	virtual void ActionAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);
	
	UFUNCTION()
	void ValidateActionTimeTargetData(const FGameplayAbilityTargetDataHandle& Data);
	
	UFUNCTION()
	void HandleActionTimeOut();

	void PlayActionTimeline(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);
	
	UFUNCTION()
	void HandleActiveDurationOver();
	
	UFUNCTION()
	void HandleActionTimelineCompleted();
	
	UFUNCTION()
	void HandleActionTimelineFailed();
	
	/** 전투 상태 GE */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TSubclassOf<UGameplayEffect> InCombatGE;
	
	/** 공중에서 사용할 수 있는지 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	bool bCanActivateWhileFalling = true;
	
	/** 마나 사용량. Cost GE가 GameplayEffect_ManaCost일 때만 사용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Cost", meta=(ClampMin=0, UIMin=0))
	float ManaCost = 0.f;

	/** 쿨다운 시간. Cooldown GE가 GameplayEffect_Cooldown일 때만 사용. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Cooldown", meta=(ClampMin=0, UIMin=0, Units="s"))
	float Cooldown = 0.f;

	/** 쿨다운 태그. 같은 태그를 사용하는 어빌리티끼리 쿨다운 공유 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Cooldown", meta=(EditCondition="Cooldown > 0", EditConditionHides, Categories="TAG.Ability.Condition.Cooldown"))
	FGameplayTag CooldownTag;

	/** 어빌리티 종료 후 부여할 태그. 연계 스킬에 사용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Chain")
	FGameplayTagContainer FollowupTags;
	
	/** FollowupTags 부여 유지 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Chain", meta=(ClampMin=0, UIMin=0, Units="s"))
	float FollowupTagGrantDuration = 3.f;
	
	/** 해당 태그를 부여한 GE를 제거한다. 연계 스킬에 사용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Chain")
	FGameplayTagContainer ConsumeTags;
	
	/** 최대 네트워크 보정 시간. 중요한 Action의 시작 시간 이전 추천. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Networking", meta=(ClampMin=0, UIMin=0, Units="s"))
	double MaxLatencyCompensation = .2;

	/** 스킬 타겟팅 타입 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TEnumAsByte<EAbilityTargetingPolicy::Type> TargetingPolicy = EAbilityTargetingPolicy::NonTargeting;
	
	/** GroundTrace 타겟 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Range", meta=(EditCondition="TargetingPolicy == EAbilityTargetingPolicy::Range", EditConditionHides))
	TSubclassOf<class AGameplayAbilityTargetActor_GroundTrace> GroundTraceClass;
	
	/** GroundTrace 타겟 액터의 레티클 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Range", meta=(EditCondition="TargetingPolicy == EAbilityTargetingPolicy::Range", EditConditionHides))
	TSubclassOf<AActor> ReticleClass;
	
	/** GroundTrace 타겟 액터의 레티클 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Range", meta=(ClampMin=0, UIMin=0, EditCondition="TargetingPolicy == EAbilityTargetingPolicy::Range", EditConditionHides))
	float ReticleScale = 1.f;
	
	/** 대상 진영 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Bitmask, BitmaskEnum="/Script/URGame.EURFaction"))
	uint8 TargetFactions = static_cast<uint8>(EURFaction::Enemy);
	
	/** 사용 가능 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ClampMin=0, UIMin=0, Units="cm"))
	float UseRange = 2000.f;
	
	/** 실행할 Action 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FURAbilityActionBase>> Actions;
	
	/** 어빌리티 활성화 시간. Actions 실행이 모두 완료되어도 이 시간 동안은 어빌리티가 활성화 상태로 남는다. 0 이하일 경우 Actions 모두 종료 시 종료한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Units="s"))
	float ActiveDuration = 0.f;
	
	/** Actions 실행 타임라인과 상관없이 ActiveDuration이 지나면 즉시 어빌리티를 종료할지 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ClampMin=0, UIMin=0, EditCondition="ActiveDuration > 0", EditConditionHides))
	bool bEndAbilityWhenActiveDurationOver = false;
	
	/** 이동 제한 시간 범위. UpperBound값이 Open일 경우 모든 Action 종료 후 제한 해제 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	FFloatRange BlockMovementRange;
	
	/** 회전 제한 시간 범위. UpperBound값이 Open일 경우 모든 Action 종료 후 제한 해제 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	FFloatRange BlockRotationRange;
	
	/** 어빌리티 아이콘 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<UTexture2D> AbilityIcon;
	
	// GetCooldownTags()에 TagContainer가 필요해서 저장
	mutable FGameplayTagContainer CachedCooldownTags;
	
	// 타겟 액터
	TWeakObjectPtr<AActor> AbilityTargetPtr;
	
	// Range의 경우 지정한 위치, 타겟이 있을 경우 타겟 위치, 없을 경우 아바타 위치
	FVector TargetLocation;
	
	// Action 시작 시간
	double ClientDoActionTime = -1.f;
	
	FTimerHandle ActiveDurationTimerHandle;
	
	bool bIsActiveDurationOver;
	bool bIsTimelineCompleted;
};
