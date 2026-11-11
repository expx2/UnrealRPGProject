// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffect.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbilitySet.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "Blueprint/UserWidget.h"
#include "URGameHUDAbilitySlotWidget.generated.h"

class UURAbilitySystemComponent;
class UURGameplayAbility_ActiveAbility;
class UURGameplayAbility;

namespace EAbilityInputBinds
{
	enum Type : int;
}

/**
 * 
 */
UCLASS()
class URGAME_API UURGameHUDAbilitySlotWidget : public UUserWidget,
                                               public IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()
	
public:
	// ~Begin ReceiverInterface
	virtual void ReceiveAbilitySystemComponent_Implementation(class UAbilitySystemComponent* AbilitySystemComponent) override;
	virtual void ClearAbilitySystemComponent_Implementation() override;
	// ~End ReceiverInterface
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	virtual void SetAbility(const FGameplayAbilitySpecHandle& AbilitySpecHandle);
	
protected:
	virtual void NativeDestruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	void RegisterDelegates();
	
	void UnRegisterDelegates();
	
	void SetEmpty();
	
	void ResolveAbility();
	
	/** 현재 표시되고 있는 어빌리티가 사용 가능한지 체크하고 ImgDisabled 설정 */
	void UpdateIsAbilityEnabled() const;
	
	void HandleGameplayTagChanged(FGameplayTag GameplayTag, int Count);
	
	void HandleAbilityListChanged();
	
	void HandleManaChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UURAbilitySlotWidget> AbilitySlotWidget;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UImage> ImgDisabled;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UImage> ImgCooldown;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UImage> ImgAvailableFill;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UR")
	TEnumAsByte<EAbilityInputBinds::Type> Command;
	
	TWeakObjectPtr<UURAbilitySystemComponent> AbilityComponent;
	
	// 현재 보여지고 있는 어빌리티
	TWeakObjectPtr<UURGameplayAbility_ActiveAbility> ShowingAbility;
	
	FDelegateHandle TagCountChangedDelegateHandle;
	FDelegateHandle AbilityListChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
	
	// =================================================================================================================
	// 쿨다운
	// =================================================================================================================
protected:
	void ResolveCooldown();
	void ClearCooldown();
	void RefreshCooldown();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|UI")
	void SetCooldown(const float Percentage) const;

	bool bHasCooldownGE;
	float CooldownEndTime = 0.f;
	float CooldownDuration = 0.f;
	
	// =================================================================================================================
	// 연계 스킬 사용 가능 시간
	// =================================================================================================================
protected:
	void ResolveChainTime();
	void ClearChainTime();
	void RefreshChainTime();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|UI")
	void SetChainTime(const float Percentage) const;
	
	bool bHasChainGE;
	float ChainEndTime = 0.f;
	float ChainDuration = 0.f;
};
