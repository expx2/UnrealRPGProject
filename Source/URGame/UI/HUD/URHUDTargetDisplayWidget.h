// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/URActorReceiverInterface.h"
#include "URHUDTargetDisplayWidget.generated.h"

class APawn;
class APlayerController;
class UURTargetingComponent;
class UURHealthComponent;

/**
 * 
 */
UCLASS()
class URGAME_API UURHUDTargetDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;
	
	void SetTarget(AActor* Actor);
	
	void ClearCurrentTarget();
	
	void UpdateDistance() const;
	
	UFUNCTION()
	void HandleTargetChanged(UURTargetingComponent* TargetingComponent, AActor* OldTargetActor, AActor* NewTargetActor);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TxtName;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TxtDistance;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UURHealthBarWidget> HealthBar;
	
	TWeakObjectPtr<UURTargetingComponent> BoundTargetingComponent;
	
	TWeakObjectPtr<AActor> CurrentTargetPtr;
	
	FTimerHandle UpdateDistanceTimerHandle;
};
