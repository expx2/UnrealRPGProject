// Written by EXPx2


#include "Character/URCameraBoomComponent.h"

UURCameraBoomComponent::UURCameraBoomComponent()
{
	// SpringArm
	bUsePawnControlRotation = true;
	TargetArmLength = FMath::Clamp(500.f, MinZoomValue, MaxZoomValue);
}

void UURCameraBoomComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitialLocation = GetRelativeLocation();
}

void UURCameraBoomComponent::Zoom(const float Addition)
{
	const float NewValue = TargetArmLength + (Addition * ZoomSensitivity);
	TargetArmLength = FMath::Clamp(NewValue, MinZoomValue, MaxZoomValue);

	if (TargetArmLength > MaxCloseupValue)
	{
		SetRelativeLocation(InitialLocation);
	}
	else
	{
		const float CloseupValue = 1.f - (TargetArmLength - MinZoomValue) / (MaxCloseupValue - MinZoomValue);
		SetRelativeLocation(InitialLocation + CloseupLocation * CloseupValue);
	}
}
