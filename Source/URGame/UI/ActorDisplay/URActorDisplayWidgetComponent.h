// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Character/URASCCharacter.h"
#include "UI/URWidgetComponent.h"
#include "URActorDisplayWidgetComponent.generated.h"

class APawn;
class AController;
class UURHealthComponent;
class UURTargetingComponent;
/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URGAME_API UURActorDisplayWidgetComponent : public UURWidgetComponent
{
	GENERATED_BODY()
	
public:
	UURActorDisplayWidgetComponent();
	
	virtual void InitWidget() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void RefreshNameColor() const;
	
	void Clear();
	
	void ResetIsHit();
	
	void RefreshVisibility();

protected:
	
	UFUNCTION()
	void HandleMaxHealthChanged(UURHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);
	
	UFUNCTION()
	void HandleHealthChanged(UURHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);
	
	UFUNCTION()
	void HandleDeath(AActor* Actor);
	
	UFUNCTION()
	void HandleTargetChanged(UURTargetingComponent* TargetingComponent, AActor* OldTargetActor, AActor* NewTargetActor);
	
	UFUNCTION()
	void HandleCinematicModeChanged(const bool bInCinematicMode);
	
	UPROPERTY(EditDefaultsOnly, Category="UR")
	float VisibilityRefreshInterval = 0.1f;

	TWeakObjectPtr<UURHealthComponent> BoundHealthComponent;
	TWeakObjectPtr<UURTargetingComponent> BoundTargetingComponent;
	FTimerHandle RefreshVisibilityHandle;
	
	bool bCinematicMode = false;
	
};
