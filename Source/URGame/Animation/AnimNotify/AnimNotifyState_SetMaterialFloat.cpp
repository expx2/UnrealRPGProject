// Written by EXPx2

#include "AnimNotifyState_SetMaterialFloat.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifyLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_SetMaterialFloat::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SetMaterialValue(MeshComp, TotalDuration <= 0.f ? EndValue : StartValue);
}

void UAnimNotifyState_SetMaterialFloat::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	const float Alpha = UAnimNotifyLibrary::GetCurrentAnimationNotifyStateTimeRatio(EventReference);
	SetMaterialValue(MeshComp, FMath::Lerp(StartValue, EndValue, Alpha));
}

void UAnimNotifyState_SetMaterialFloat::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UAnimNotifyLibrary::NotifyStateReachedEnd(EventReference))
	{
		SetMaterialValue(MeshComp, EndValue);
	}
}

void UAnimNotifyState_SetMaterialFloat::BranchingPointNotifyTick(
	FBranchingPointNotifyPayload& BranchingPointPayload,
	float FrameDeltaTime)
{
	// Branching Point에는 EventReference의 재생 시간이 없으므로 해당 몽타주 인스턴스에서 읽는다.
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	UAnimInstance* AnimInstance = IsValid(MeshComp) ? MeshComp->GetAnimInstance() : nullptr;
	const FAnimMontageInstance* MontageInstance = AnimInstance ? AnimInstance->GetMontageInstanceForID(BranchingPointPayload.MontageInstanceID) : nullptr;
	const FAnimNotifyEvent* NotifyEvent = BranchingPointPayload.NotifyEvent;
	if (!MontageInstance || !NotifyEvent)
	{
		return;
	}

	const float Duration = NotifyEvent->GetDuration();
	const float Alpha = Duration > 0.f
		? FMath::Clamp((MontageInstance->GetPosition() - NotifyEvent->GetTriggerTime()) / Duration, 0.f, 1.f)
		: 1.f;
	SetMaterialValue(MeshComp, FMath::Lerp(StartValue, EndValue, Alpha));
}

void UAnimNotifyState_SetMaterialFloat::SetMaterialValue(USkeletalMeshComponent* MeshComp, float Value) const
{
	if (IsValid(MeshComp) && !ParameterName.IsNone())
	{
		MeshComp->SetScalarParameterValueOnMaterials(ParameterName, Value);
	}
}
