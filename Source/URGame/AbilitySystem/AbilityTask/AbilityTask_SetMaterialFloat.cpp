// Written by EXPx2

#include "AbilitySystem/AbilityTask/AbilityTask_SetMaterialFloat.h"

#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"

UAbilityTask_SetMaterialFloat* UAbilityTask_SetMaterialFloat::SetMaterialFloat(
	UGameplayAbility* OwningAbility,
	const FName ParameterName,
	const float StartValue,
	const float EndValue,
	const float Duration)
{
	UAbilityTask_SetMaterialFloat* Task = NewAbilityTask<UAbilityTask_SetMaterialFloat>(OwningAbility);
	Task->bTickingTask = true;
	Task->ParameterName = ParameterName;
	Task->StartValue = StartValue;
	Task->EndValue = EndValue;
	Task->Duration = Duration;
	return Task;
}

void UAbilityTask_SetMaterialFloat::Activate()
{
	Super::Activate();

	const AActor* AvatarActor = GetAvatarActor();
	UMeshComponent* MeshComponent = IsValid(AvatarActor) ? AvatarActor->FindComponentByClass<UMeshComponent>() : nullptr;
	if (!IsValid(MeshComponent) || ParameterName.IsNone())
	{
		EndTask();
		return;
	}
	MeshComponentPtr = MeshComponent;

	if (Duration <= 0.f)
	{
		MeshComponent->SetScalarParameterValueOnMaterials(ParameterName, EndValue);
		FinishTask();
		return;
	}

	MeshComponent->SetScalarParameterValueOnMaterials(ParameterName, StartValue);
}

void UAbilityTask_SetMaterialFloat::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	UMeshComponent* MeshComponent = MeshComponentPtr.Get();
	if (!MeshComponent)
	{
		EndTask();
		return;
	}

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= Duration)
	{
		MeshComponent->SetScalarParameterValueOnMaterials(ParameterName, EndValue);
		FinishTask();
		return;
	}

	const float Value = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);
	MeshComponent->SetScalarParameterValueOnMaterials(ParameterName, FMath::Lerp(StartValue, EndValue, Value));
}

void UAbilityTask_SetMaterialFloat::FinishTask()
{
	bTickingTask = false;
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCompleted.Broadcast();
	}
	EndTask();
}
