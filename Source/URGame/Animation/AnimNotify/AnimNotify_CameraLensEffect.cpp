// Written by EXPx2
// AI Agent가 작성함.

#include "AnimNotify_CameraLensEffect.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Particles/EmitterCameraLensEffectBase.h"

UAnimNotify_CameraLensEffect::UAnimNotify_CameraLensEffect()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif
}

void UAnimNotify_CameraLensEffect::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp) || !EffectClass)
	{
		return;
	}

	const UWorld* World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const APawn* Pawn = Cast<APawn>(MeshComp->GetOwner());
	if (!IsValid(Pawn) || !Pawn->IsPlayerControlled() || !Pawn->IsLocallyControlled())
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PlayerController) || !PlayerController->IsLocalPlayerController())
	{
		return;
	}

	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager.Get();
	if (IsValid(CameraManager))
	{
		CameraManager->AddGenericCameraLensEffect(EffectClass);
	}
}

FString UAnimNotify_CameraLensEffect::GetNotifyName_Implementation() const
{
	return TEXT("Camera Lens Effect");
}
