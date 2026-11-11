// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "URCameraBoomComponent.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURCameraBoomComponent : public USpringArmComponent
{
	GENERATED_BODY()
	
public:
	UURCameraBoomComponent();
	
	virtual void BeginPlay() override;

	void Zoom(const float Addition);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(UIMin=0, ClampMin=0))
	float MinZoomValue = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(UIMin=1, ClampMin=1))
	float MaxZoomValue = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(UIMin=0, ClampMin=0))
	float ZoomSensitivity = 30.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(UIMin=1, ClampMin=1))
	float MaxCloseupValue = 300.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	FVector CloseupLocation = FVector(0.f, 0.f, 40.f);
	
	FVector InitialLocation = FVector::ZeroVector;
};
