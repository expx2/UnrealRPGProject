// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "URFloatingDamageWidget.generated.h"

class UURFloatingDamageWidget;

UENUM(BlueprintType)
enum class EURFloatingDamageType : uint8
{
	/** 몬스터 일반 대미지 */
	Normal = 0,
	
	/** 몬스터 크리티컬 대미지 */
	Critical = 1,
	
	/** 플레이어 일반 대미지 */
	PlayerNormal = 10,
	
	/** 플레이어 크리티컬 대미지 */
	PlayerCritical = 11,
};

DECLARE_DELEGATE_OneParam(FFloatingDamageWidgetReleasedEvent, UURFloatingDamageWidget* /* Widget */);

/**
 * 
 */
UCLASS()
class URGAME_API UURFloatingDamageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowDamage(const float Damage);
	
	UFUNCTION(BlueprintImplementableEvent, Category="UR|UI")
	void OnShowDamage(const float Damage);
	
	EURFloatingDamageType GetDamageType() const { return DamageType;}
	
	FFloatingDamageWidgetReleasedEvent Release;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	EURFloatingDamageType DamageType = EURFloatingDamageType::Normal;
	
protected:
	virtual void NativeDestruct() override;
	
	void RemoveDamage();
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UTextBlock> TxtDamage;
	
	FTimerHandle RemoveTimerHandle;
};
