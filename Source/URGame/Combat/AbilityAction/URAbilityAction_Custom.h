// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"
#include "StructUtils/InstancedStruct.h"

#include "URAbilityAction_Custom.generated.h"

USTRUCT(BlueprintType, meta=(DisplayName="Custom"))
struct FURAbilityAction_Custom : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override;
	
	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
};