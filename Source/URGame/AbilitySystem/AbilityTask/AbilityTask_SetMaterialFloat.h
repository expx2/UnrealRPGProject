// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_SetMaterialFloat.generated.h"

class UMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSetMaterialFloatDelegate);

/**
 *
 */
UCLASS()
class URGAME_API UAbilityTask_SetMaterialFloat : public UAbilityTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_SetMaterialFloat* SetMaterialFloat(UGameplayAbility* OwningAbility, FName ParameterName, float StartValue, float EndValue, float Duration);

	virtual void TickTask(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable)
	FSetMaterialFloatDelegate OnCompleted;

protected:
	virtual void Activate() override;

	void FinishTask();

	TWeakObjectPtr<UMeshComponent> MeshComponentPtr;
	FName ParameterName;
	float StartValue = 0.f;
	float EndValue = 0.f;
	float Duration = 0.f;
	float ElapsedTime = 0.f;
};
