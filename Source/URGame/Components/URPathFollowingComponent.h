// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "URPathFollowingComponent.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="UR|Navigation")
	void MoveToActor(AController* Controller, const AActor* Goal, FAIRequestID& OutRequestID);
};
