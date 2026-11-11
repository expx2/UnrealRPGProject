// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_SimpleWaitTargetData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimpleWaitTargetDataDelegate, const FGameplayAbilityTargetDataHandle&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleWaitTargetDataSimpleDelegate);

/**
 * 타겟 액터를 생성하지 않고 설정한 태그의 타겟 데이터를 받을 때까지 대기
 */
UCLASS()
class URGAME_API UAbilityTask_SimpleWaitTargetData : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta=(HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms="Instigator"), Category="Ability|Tasks")
	static UAbilityTask_SimpleWaitTargetData* SimpleWaitTargetData(UGameplayAbility* OwningAbility,  FGameplayTag TargetDataTag, float TimeoutSeconds = 1.0f);
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	/** TargetData 수신 시 호출 */
	UPROPERTY(BlueprintAssignable)
	FSimpleWaitTargetDataDelegate ValidData;
	
	/** TargetData 수신 전 제한 시간이 지나면 호출 */
	UPROPERTY(BlueprintAssignable)
	FSimpleWaitTargetDataSimpleDelegate TimeOut;

protected:
	virtual void Activate() override;
	
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);
	
	void HandleTimeOut();
	
	void FinishTask(const FGameplayAbilityTargetDataHandle& Data ,const bool bIsSuccess);
	
	// 기다리는 타겟 데이터의 태그. 타겟 데이터를 수신해도 태그가 다르면 넘어감
	FGameplayTag TargetDataTag;
	
	float TimeoutSeconds = 1.0f;
	
	FDelegateHandle TargetDataSetDelegateHandle;
	FTimerHandle TimeoutTimerHandle;

	bool bIsFinished = false;
};
