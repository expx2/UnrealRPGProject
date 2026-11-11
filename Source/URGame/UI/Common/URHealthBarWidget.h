// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Interface/URActorReceiverInterface.h"
#include "Blueprint/UserWidget.h"
#include "URHealthBarWidget.generated.h"

class UURHealthComponent;
/**
 * 
 */
UCLASS(BlueprintType)
class URGAME_API UURHealthBarWidget : public UUserWidget,
                                      public IURActorReceiverInterface
{
	GENERATED_BODY()
	
public:
	virtual void ReceiveActor_Implementation(AActor* Actor) override;
	
	virtual void ClearActor_Implementation() override;

protected:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintNativeEvent, Category="UR|UI")
	void SetNonHealth();
	
	UFUNCTION(BlueprintNativeEvent, Category="UR|UI")
	void HandleMaxHealthChanged(UURHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);
	
	UFUNCTION(BlueprintNativeEvent, Category="UR|UI")
	void HandleHealthChanged(UURHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);


	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
    TObjectPtr<class UProgressBar> PBHealth;
	
    TWeakObjectPtr<UURHealthComponent> BoundHealthComponentPtr;
};
