// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_Custom.h"

#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"

UAbilityActionTask_Custom* UAbilityActionTask_Custom::CustomAction(
	UURGameplayAbility_ActiveAbility* OwningAbility,
	FURAbilityActionContext& Context, 
	const float ElapsedTime)
{
	UAbilityActionTask_Custom* Task = NewAbilityTask<UAbilityActionTask_Custom>(OwningAbility);
	Task->Context = Context;
	Task->ElapsedTime = ElapsedTime;
	return Task;
}

bool UAbilityActionTask_Custom::FinishTask(const bool bIsSuccess)
{
	return Super::FinishTask(bIsSuccess);
}

void UAbilityActionTask_Custom::Activate()
{
	Super::Activate();
	
	if (UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability))
	{
		ActiveAbility->ActivateCustomAbilityAction(this, Context, ElapsedTime);
	}
	else
	{
		FinishTask(false);
	}
	
}


