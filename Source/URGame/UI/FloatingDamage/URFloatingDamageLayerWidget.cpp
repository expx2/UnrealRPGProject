// Written by EXPx2


#include "UI/FloatingDamage/URFloatingDamageLayerWidget.h"

#include <BlueprintLibrary/URGameBlueprintLibrary.h>

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "UI/FloatingDamage/URFloatingDamageWidget.h"

void UURFloatingDamageLayerWidget::ShowDamage(const AActor* HitActor, const float Damage, const bool bIsCritical)
{
	if (!IsValid(HitActor))
	{
		return;
	}

	EURFloatingDamageType DamageType = EURFloatingDamageType::Normal;
	if (const APawn* Pawn = Cast<APawn>(HitActor))
	{
		if (Pawn->IsPlayerControlled() && Pawn->IsLocallyControlled())
		{
			DamageType = EURFloatingDamageType::PlayerNormal;
		}
	}
	
	if (bIsCritical)
	{
		// DamageType = static_cast<EURFloatingDamageType>(static_cast<uint8>(DamageType) + 1);
		switch (DamageType)
		{
		case EURFloatingDamageType::Normal:
			DamageType = EURFloatingDamageType::Critical;
			break;
		case EURFloatingDamageType::PlayerNormal:
			DamageType = EURFloatingDamageType::PlayerCritical;
			break;
		default:
			break;
		}
	}
	
	FVector WorldLocation;
	UURGameBlueprintLibrary::GetAimTargetLocation(HitActor, WorldLocation);
	FVector2D ScreenPosition;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		GetOwningPlayer(), 
		WorldLocation, 
		ScreenPosition, 
		true))
	{
		UURFloatingDamageWidget* Widget = RentWidget(DamageType);
		if (!Widget)
		{
			return;
		}
	
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot);
		if (!CanvasSlot)
		{
			// 생성된 Widget 제거
			return;
		}

		// Offset은 위쪽 반원만 사용
		const FVector2D PositionOffset = 
			FMath::RandRange(MinPositionOffsetRadius, MaxPositionOffsetRadius) *
			FVector2D(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 0.f)).GetSafeNormal();
		
		CanvasSlot->SetPosition(ScreenPosition + PositionOffset);
		Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
		Widget->ShowDamage(Damage);

		ActiveWidgets.Add(FActiveFloatingDamageWidget{Widget, WorldLocation, PositionOffset});
		return;
	}
}

void UURFloatingDamageLayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	/*
	 * Note: 추후 성능 개선 필요할 경우 카메라 업데이트 시에만 갱신하도록 처리
	 * 그 방법이 더 성능 잡아먹을 수도 있음 테스트 후 작업하도록
	 */

	UpdateActiveWidgets();
}

void UURFloatingDamageLayerWidget::ReleaseWidget(UURFloatingDamageWidget* Widget)
{
	if (!Widget)
	{
		return;
	}
	
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	EURFloatingDamageType DamageType = Widget->GetDamageType();
	WidgetPoolMap.FindOrAdd(DamageType).Widgets.Push(Widget);
	
	ActiveWidgets.RemoveAllSwap(
		[Widget](const FActiveFloatingDamageWidget& Entry)
		{
			return Entry.Widget == Widget;
		},
		EAllowShrinking::No);
}

void UURFloatingDamageLayerWidget::UpdateActiveWidgets()
{
	const APlayerController* PC = GetOwningPlayer();
	FVector2D ScreenPosition;
	for (FActiveFloatingDamageWidget& ActiveWidget : ActiveWidgets)
	{
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			PC,
			ActiveWidget.WorldLocation,
			ScreenPosition,
			true))
		{
			UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ActiveWidget.Widget->Slot);
			if (!CanvasSlot)
			{
				continue;
			}

			CanvasSlot->SetPosition(ScreenPosition + ActiveWidget.PositionOffset);
		}
	}
}

UURFloatingDamageWidget* UURFloatingDamageLayerWidget::RentWidget(const EURFloatingDamageType FloatingDamageType)
{
	if (FFloatingDamageWidgetPool* Pool = WidgetPoolMap.Find(FloatingDamageType))
	{
		if (!Pool->Widgets.IsEmpty())
		{
			return Pool->Widgets.Pop(EAllowShrinking::No).Get();
		}
	}

	return CreateNewWidget(FloatingDamageType);
}

UURFloatingDamageWidget* UURFloatingDamageLayerWidget::CreateNewWidget(const EURFloatingDamageType FloatingDamageType)
{
	TSubclassOf<class UURFloatingDamageWidget> WidgetClass = nullptr;
	switch (FloatingDamageType)
	{
	case EURFloatingDamageType::Normal:
		WidgetClass = NormalWidgetClass;
		break;
	case EURFloatingDamageType::Critical:
		WidgetClass = CriticalWidgetClass;
		break;
	case EURFloatingDamageType::PlayerNormal:
		WidgetClass = PlayerNormalWidgetClass;
		break;
	case EURFloatingDamageType::PlayerCritical:
		WidgetClass = PlayerCriticalWidgetClass;
		break;
	}

	if (!WidgetClass)
	{
		return nullptr;
	}

	UURFloatingDamageWidget* DamageWidget = CreateWidget<UURFloatingDamageWidget>(GetOwningPlayer(), WidgetClass);
	if (!DamageWidget)
	{
		return nullptr;
	}
	
	// BP에서 기본값으로 설정할 거지만 잘못 설정할 때를 대비
	DamageWidget->DamageType = FloatingDamageType;
	
	UCanvasPanelSlot* CanvasSlot = FloatingDamageCanvas->AddChildToCanvas(DamageWidget);
	CanvasSlot->SetAutoSize(true);
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	DamageWidget->Release.BindUObject(this, &ThisClass::ReleaseWidget);

	return DamageWidget;
}
