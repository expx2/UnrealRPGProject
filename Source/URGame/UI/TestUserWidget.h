// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/URActorReceiverInterface.h"
#include "TestUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UTestUserWidget : public UUserWidget,
                                   public IURActorReceiverInterface
{
	GENERATED_BODY()
	
public:
	// ~Begin ActorReceiverInterface
	virtual void ReceiveActor_Implementation(AActor* Actor) override;
	virtual void ClearActor_Implementation() override;
	// ~End ActorReceiverInterface
};
