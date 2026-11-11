// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/FloatingDamage/URFloatingDamageWidget.h"
#include "URFloatingDamageLayerWidget.generated.h"

USTRUCT()
struct FActiveFloatingDamageWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<UURFloatingDamageWidget> Widget = nullptr;
	
	UPROPERTY()
	FVector WorldLocation = FVector::ZeroVector;
	
	UPROPERTY()
	FVector2D PositionOffset = FVector2D::ZeroVector;
};

USTRUCT()
struct FFloatingDamageWidgetPool
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UURFloatingDamageWidget>> Widgets;
};

/**
 * 
 */
UCLASS()
class URGAME_API UURFloatingDamageLayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowDamage(const AActor* HitActor, const float Damage, const bool bIsCritical);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UURFloatingDamageWidget* RentWidget(const EURFloatingDamageType FloatingDamageType);
	
	UURFloatingDamageWidget* CreateNewWidget(const EURFloatingDamageType FloatingDamageType);
	
	void ReleaseWidget(UURFloatingDamageWidget* Widget);
	
	void UpdateActiveWidgets();
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UCanvasPanel> FloatingDamageCanvas;

	UPROPERTY(EditDefaultsOnly, Category="UR", meta=(ClampMin=0, UIMin=0))
	float MinPositionOffsetRadius = 70.f;
	
	UPROPERTY(EditDefaultsOnly, Category="UR", meta=(ClampMin=0, UIMin=0))
	float MaxPositionOffsetRadius = 200.f;
	
	/** 몬스터 일반 대미지 용 위젯 */
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TSubclassOf<UURFloatingDamageWidget> NormalWidgetClass;
	
	/** 몬스터 크리티컬 대미지 용 위젯 */
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TSubclassOf<UURFloatingDamageWidget> CriticalWidgetClass;
	
	/** 플레이어 일반 대미지 용 위젯 */
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TSubclassOf<UURFloatingDamageWidget> PlayerNormalWidgetClass;
	
	/** 플레이어 크리티컬 대미지 용 위젯 */
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TSubclassOf<UURFloatingDamageWidget> PlayerCriticalWidgetClass;
	
	UPROPERTY(Transient)
	TArray<FActiveFloatingDamageWidget> ActiveWidgets;
	
	// 비활성(대기) 위젯 풀
	UPROPERTY(Transient)
	TMap<EURFloatingDamageType, FFloatingDamageWidgetPool> WidgetPoolMap;
};
