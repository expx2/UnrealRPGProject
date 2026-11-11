// Written by EXPx2
// AI Agent가 작성함.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_CameraLensEffect.generated.h"

class AEmitterCameraLensEffectBase;


/** Plays a camera lens effect only for the locally controlled player pawn. */
UCLASS(const, meta = (DisplayName = "Camera Lens Effect"))
class URGAME_API UAnimNotify_CameraLensEffect : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_CameraLensEffect();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override;

	/** Lens effect blueprint to play. Its particle completion/lifespan controls cleanup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Lens Effect")
	TSubclassOf<AEmitterCameraLensEffectBase> EffectClass;
};
