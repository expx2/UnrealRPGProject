// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"

#include "URAbilityAction_PlayMontage.generated.h"

UENUM(BlueprintType)
namespace EPlayMontageCompensationPolicy
{
	enum Type : int
	{
		/** 보정하지 않음 */
		None, 
		
		/** 앞 부분을 보정 시간만큼 생략 */
		Skipped,
		
		/** 종료 시간이 같도록 PlayRate 증가 */
		SpedUp,
	};
}

class UAnimMontage;

USTRUCT(BlueprintType, meta=(DisplayName="Play Montage"))
struct FURAbilityAction_PlayMontage : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override { return IsValid(AnimMontage); }
	
	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
	
	/** 지연 시간 보정 정책 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TEnumAsByte<EPlayMontageCompensationPolicy::Type> CompensationPolicy = EPlayMontageCompensationPolicy::SpedUp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<UAnimMontage> AnimMontage;
	
	/** 참일 경우 몽타주 실행과 동시에 해당 태스크는 완료되어 타임라인 종료에 영향을 주지 않음 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	bool bIsInstant = false;
};