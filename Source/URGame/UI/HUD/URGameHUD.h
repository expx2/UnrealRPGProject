// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Definitions/URPopupType.h"
#include "GameFramework/HUD.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "Interface/URActorReceiverInterface.h"
#include "URGameHUD.generated.h"

class UURFloatingDamageLayerWidget;
class AURASCCharacter;
class UURGameHUDWidget;

/**
 * 
 */
UCLASS()
class URGAME_API AURGameHUD : public AHUD,
                              public IURActorReceiverInterface,
                              public IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()
	
public:
	AURGameHUD();
	
	// ~Begin ReceiverInterface
	virtual void ReceiveActor_Implementation(AActor* Actor) override;
	virtual void ClearActor_Implementation() override;
	virtual void ReceiveAbilitySystemComponent_Implementation(UAbilitySystemComponent* AbilitySystemComponent) override;
	virtual void ClearAbilitySystemComponent_Implementation() override;
	// ~End ReceiverInterface
	
	void UpdateVisibility() const;
	
	/** 대미지 플로터 표시 */
	UFUNCTION(BlueprintCallable, Category="UR|HUD")
	void ShowDamage(const AActor* HitActor, const float Damage, const bool bIsCritical) const;
	
	/** 타겟 액터 사용 인디케이터 활성화/비활성화 */
	UFUNCTION(BlueprintCallable, Category="UR|HUD")
	void SetTargetActorIndicatorEnable(const bool bValue) const;
	
	void SetIsSpectating(bool bValue);
	
	void SetViewTarget(AActor* ViewTarget);

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void InitializeWidgets();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TSubclassOf<UURGameHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category="UR")
	TObjectPtr<UURGameHUDWidget> HUDWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TSubclassOf<UURFloatingDamageLayerWidget> FloatingDamageLayerWidgetClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category="UR")
	TObjectPtr<UURFloatingDamageLayerWidget> FloatingDamageLayerWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TSubclassOf<UUserWidget> AimTargetIndicatorWidgetClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category="UR")
	TObjectPtr<UUserWidget> AimTargetIndicatorWidget;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsSpectating = false;
};
