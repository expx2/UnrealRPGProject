// Written by EXPx2


#include "URAbilityAction_PlayMontage.h"

#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_PlayMontageAndWait.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"

FURAbilityActionStartResult FURAbilityAction_PlayMontage::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	float Rate = 1.0f;
	float StartSeconds = 0.0f;
	
	if (CompensationPolicy == EPlayMontageCompensationPolicy::Skipped)
	{
		// 지난 시간만큼 시작 시간 설정
		const float CompensationTime = GetActionElapsedTime(TimelineElapsed);
		StartSeconds = CompensationTime;
	}
	else if (CompensationPolicy == EPlayMontageCompensationPolicy::SpedUp)
	{
		// 종료 시간이 같도록 재생 속도 조절
		const float CompensationTime = GetActionElapsedTime(TimelineElapsed);
		const float MontageLength = AnimMontage.Get()->GetPlayLength();
		if (MontageLength <= CompensationTime)
		{
			// 지연 시간이 몽타주 시간보다 길어 실행하지 않고 즉시 종료
			return FURAbilityActionStartResult::Completed();
		}
		Rate = MontageLength / (MontageLength - CompensationTime);
	}
	
	UAbilityActionTask_PlayMontageAndWait* PlayMontageAndWait =
		UAbilityActionTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			Ability,
			TEXT("PlayMontageAndWait"),
			AnimMontage,
			Rate,
			NAME_None,
			!bIsInstant,
			1.0f,
			StartSeconds,
			false);
	
	if (bIsInstant)
	{
		PlayMontageAndWait->ReadyForActivation();
		return FURAbilityActionStartResult::Completed();
	}

	return FURAbilityActionStartResult::Running(PlayMontageAndWait);
}
