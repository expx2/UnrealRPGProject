// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "URAbilitySystemComponentReceiverInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UURAbilitySystemComponentReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class URGAME_API IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|UI")
	void ReceiveAbilitySystemComponent(class UAbilitySystemComponent* AbilitySystemComponent);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|UI")
	void ClearAbilitySystemComponent();
};
