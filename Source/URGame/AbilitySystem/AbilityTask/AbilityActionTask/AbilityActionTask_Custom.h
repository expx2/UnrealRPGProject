// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask.h"
#include "Combat/AbilityAction/URAbilityActionBase.h"
#include "AbilityActionTask_Custom.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UAbilityActionTask_Custom : public UAbilityActionTask
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityActionTask_Custom* CustomAction(UURGameplayAbility_ActiveAbility* OwningAbility, FURAbilityActionContext& Context, const float ElapsedTime);
	
	virtual bool FinishTask(const bool bIsSuccess) override;
	
protected:
	virtual void Activate() override;
	
	
	FURAbilityActionContext Context;
	float ElapsedTime;
};
