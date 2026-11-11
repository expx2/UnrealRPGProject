// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityTask_SimpleWaitTargetData.h"

#include "AbilitySystemComponent.h"

UAbilityTask_SimpleWaitTargetData* UAbilityTask_SimpleWaitTargetData::SimpleWaitTargetData(
	UGameplayAbility* OwningAbility,
	FGameplayTag TargetDataTag, 
	float TimeoutSeconds)
{
	UAbilityTask_SimpleWaitTargetData* MyObj = NewAbilityTask<UAbilityTask_SimpleWaitTargetData>(OwningAbility);
	MyObj->TargetDataTag = TargetDataTag;
	MyObj->TimeoutSeconds = TimeoutSeconds;
	return MyObj;
}

void UAbilityTask_SimpleWaitTargetData::OnDestroy(bool bInOwnerFinished)
{
	if (TargetDataSetDelegateHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
		{
			const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
			const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
			ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).Remove(TargetDataSetDelegateHandle);
		}
	}
	TargetDataSetDelegateHandle.Reset();
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}
	TimeoutTimerHandle.Invalidate();
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_SimpleWaitTargetData::Activate()
{
	Super::Activate();
	
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	
	check(ASC);
	
	// TargetData RPC 수신 시 델리게이트 설정
	const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
	const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
	TargetDataSetDelegateHandle = ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &ThisClass::OnTargetDataReplicatedCallback);
	// 타임아웃 타이머 등록
	if (TimeoutSeconds > SMALL_NUMBER)
	{
		GetWorld()->GetTimerManager().SetTimer(TimeoutTimerHandle, this, &ThisClass::HandleTimeOut, TimeoutSeconds, false);
	}
	// 이미 도착한 경우 처리
	ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
	
	SetWaitingOnRemotePlayerData();
}

/** Valid TargetData was replicated to use (we are server, was sent from client) */
void UAbilityTask_SimpleWaitTargetData::OnTargetDataReplicatedCallback(
	const FGameplayAbilityTargetDataHandle& Data, 
	FGameplayTag ActivationTag)
{
	// see also: UAbilityTask_WaitTargetData::OnTargetDataReplicatedCallback
	if (!TargetDataTag.MatchesTagExact(ActivationTag))
	{
		return;
	}
	
	FGameplayAbilityTargetDataHandle MutableData = Data;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	}
	
	FinishTask(MutableData, true);
}

void UAbilityTask_SimpleWaitTargetData::HandleTimeOut()
{
	FinishTask(FGameplayAbilityTargetDataHandle(), false);
}

void UAbilityTask_SimpleWaitTargetData::FinishTask(
	const FGameplayAbilityTargetDataHandle& Data, 
	const bool bIsSuccess)
{
	if (bIsFinished)
	{
		return;
	}
	
	bIsFinished = true;
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bIsSuccess)
		{
			ValidData.Broadcast(Data);
		}
		else
		{
			TimeOut.Broadcast();
		}
	}
	EndTask();
}
