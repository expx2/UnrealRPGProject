// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "Interface/URActorReceiverInterface.h"
#include "URGameHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURGameHUDWidget : public UUserWidget,
                                    public IURActorReceiverInterface,
                                    public IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()

public:
	// ~Begin ReceiverInterface
	virtual void ReceiveActor_Implementation(AActor* Actor) override;
	virtual void ReceiveAbilitySystemComponent_Implementation(class UAbilitySystemComponent* AbilitySystemComponent) override;
	// ~End ReceiverInterface
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="UR|UI")
	void SetTargetActorIndicatorEnable(bool bValue);
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="UR|UI")
	void SetIsSpectating(bool bValue);
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="UR|UI")
	void SetViewTarget(AActor* Actor);
};
