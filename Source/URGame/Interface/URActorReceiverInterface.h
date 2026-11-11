// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "URActorReceiverInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UURActorReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class URGAME_API IURActorReceiverInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|UI")
	void ReceiveActor(AActor* Actor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|UI")
	void ClearActor();
};
