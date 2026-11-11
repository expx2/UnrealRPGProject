// Written by EXPx2


#include "UI/URUISubsystem.h"

#include "URGame.h"
#include "URPopupWidget.h"
#include "URUILayer.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "DataAssets/URPopupWidgetMapping.h"
#include "GameplayTags/URGameplayTags.h"
#include "HUD/URGameHUD.h"
#include "HUD/URUIRootWidget.h"

UURUISubsystem::UURUISubsystem()
{
	static ConstructorHelpers::FObjectFinder<UURPopupWidgetMapping> MappingRef(TEXT("/Game/_ProjectUR/DataAssets/DA_PopupWidgetMapping.DA_PopupWidgetMapping"));
	Mapping = MappingRef.Object;
	
	static ConstructorHelpers::FClassFinder<UURUIRootWidget> PopupWidgetMappingRef(TEXT("/Game/_ProjectUR/UI/WBP_RootWidget"));
	RootWidgetClass = PopupWidgetMappingRef.Class;
}

void UURUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UR_LOG(LogUR, Log, TEXT("UISubSystem Ready"));
}

void UURUISubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UURUISubsystem::TestFunction()
{
	UR_LOG(LogUR, Log, TEXT("UISubSystem Test"));
	
}

void UURUISubsystem::InitRootWidget()
{
	if (HasRootWidget())
	{
		return;
	}
	
	ClearRootWidget();
	
	if (!ensure(RootWidgetClass.Get()))
	{
		return;
	}
	
	if (APlayerController* PC = GetLocalPlayer()->PlayerController)
	{
		// 위젯 생성
		RootWidget = CreateWidget<UURUIRootWidget>(PC, RootWidgetClass);
		if (ensure(RootWidget))
		{
			RootWidget->AddToPlayerScreen(URUILayer::RootLayer);
		}
	}
}

bool UURUISubsystem::HasRootWidget()
{
	return IsValid(RootWidget) && RootWidget->IsInViewport();
}

void UURUISubsystem::AddSceneHUD(UUserWidget* Widget)
{
	if (Widget)
	{
		InitRootWidget();

		if (RootWidget)
		{
			if (UOverlaySlot* Slot = RootWidget->SceneHUDLayer.Get()->AddChildToOverlay(Widget))
			{
				Slot->SetHorizontalAlignment(HAlign_Fill);
				Slot->SetVerticalAlignment(VAlign_Fill);
			}
		}
	}
}

void UURUISubsystem::AddGameWidget(UUserWidget* Widget)
{
	if (Widget)
	{
		InitRootWidget();

		if (RootWidget)
		{
			if (UCanvasPanelSlot* Slot = RootWidget->GameLayer.Get()->AddChildToCanvas(Widget))
			{
				// Slot->SetAnchors(FAnchors(0.5f, 0.5f));
				Slot->SetAutoSize(true);
				Slot->SetAlignment(FVector2D(0.5f, 0.5f));
			}
		}
	}
}

UURUIRootWidget* UURUISubsystem::GetRootWidget()
{
	InitRootWidget();
	return RootWidget;
}

void UURUISubsystem::ToastMessage(const FText& Message)
{
	InitRootWidget();
	if (RootWidget)
	{
		RootWidget->ToastMessage(Message);
	}
}

void UURUISubsystem::AbilityFailedToastMessage(const struct FGameplayTagContainer& FailureReason)
{
	FText Message;
	if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_Cooldown))
	{
		Message = FText::FromString(TEXT("아직 사용할 수 없습니다."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_Cost))
	{
		Message = FText::FromString(TEXT("마나가 부족합니다."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_Networking))
	{
		Message = FText::FromString(TEXT("네트워크 오류가 발생했습니다."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_Undefined))
	{
		Message = FText::FromString(TEXT("알 수 없는 오류가 발생했습니다."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_NoTarget))
	{
		Message = FText::FromString(TEXT("대상을 지정해야 합니다."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_InvalidTarget))
	{
		Message = FText::FromString(TEXT("사용할 수 없는 대상입니다."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_MoveFailed))
	{
		Message = FText::FromString(TEXT("대상에게 이동을 실패했습니다. 수동으로 이동해 주세요."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_PathFollowingFailed))
	{
		Message = FText::FromString(TEXT("사용할 수 없는 위치에 있습니다."));
	}
	else if (FailureReason.HasTag(URGameplayTags::TAG_Ability_Fail_Condition))
	{
		Message = FText::FromString(TEXT("사용할 수 없습니다."));
	}
	
	if (!Message.IsEmpty())
	{
		ToastMessage(Message);
	}
}

UURPopupWidget* UURUISubsystem::OpenPopup(EPopupType PopupType)
{
	if (!Mapping) 
	{
		return nullptr;
	}
	
	InitRootWidget();
	
	if (!RootWidget) 
	{
		return nullptr;
	}

	UClass* Class = Mapping.Get()->GetWidgetClass(PopupType);

	auto OpenedWidget = OpenPopups.FindByPredicate([Class](UURPopupWidget* Widget)
	{
		return Widget->GetClass() == Class;
	});
	if (OpenedWidget)
	{
		// 이미 열려 있음
		return OpenedWidget->Get();
	}
	
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (APlayerController* PC = LP->PlayerController)
		{
			// 위젯 생성
			UURPopupWidget* Widget = CreateWidget<UURPopupWidget>(PC, Class);
			if (Widget)
			{
				if (UOverlaySlot* Slot = RootWidget->PopupLayer.Get()->AddChildToOverlay(Widget))
				{
					Slot->SetHorizontalAlignment(HAlign_Fill);
					Slot->SetVerticalAlignment(VAlign_Fill);
				}
			
				return Widget;
			}
		}
	}
	
	return nullptr;
}

class UURPopupWidget* UURUISubsystem::GetPopup(EPopupType PopupType)
{
	if (!Mapping)
	{
		return nullptr;
	}
	
	UClass* Class = Mapping.Get()->GetWidgetClass(PopupType);
	if (!Class)
	{
		return nullptr;
	}
	
	auto OpenedWidget = OpenPopups.FindByPredicate([Class](UURPopupWidget* Widget)
	{
		return Widget->GetClass() == Class;
	});
	if (OpenedWidget)
	{
		// 이미 열려 있음
		return OpenedWidget->Get();
	}
	return nullptr;
}

void UURUISubsystem::ClosePopup(EPopupType PopupType)
{
	if (!Mapping)
	{
		return;
	}
	
	UClass* WidgetClass = Mapping.Get()->GetWidgetClass(PopupType);
	
	auto OpenWidget = OpenPopups.FindByPredicate([WidgetClass](UURPopupWidget* Widget)
	{
		return Widget->GetClass() == WidgetClass;
	});
	UR_LOG(LogUR, Log, TEXT("ClosePopup %s (%d)"), OpenWidget ? TEXT("O") : TEXT("X"), OpenPopups.Num());
	if (OpenWidget)
	{
		OpenWidget->Get()->Close();
	}
}

void UURUISubsystem::CloseAllPopups()
{
	for (int i = OpenPopups.Num() - 1; i >= 0; i--)
	{
		OpenPopups[i]->Close();
	}
}

void UURUISubsystem::AddPopup(UURPopupWidget* Popup)
{
	bool bIsAlreadyAdded = OpenPopups.ContainsByPredicate([Popup](UURPopupWidget* Widget)
	{
		return Widget->GetClass() == Popup->GetClass();
	});
	
	if (bIsAlreadyAdded)
	{
		return;
	}
	
	OpenPopups.Add(Popup);
	
	UpdateInputModeByPopup();
	
	UR_LOG(LogUR, Log, TEXT("Popup Added %s (%d)"), *Popup->GetName(), OpenPopups.Num());
}

bool UURUISubsystem::RemovePopup(UURPopupWidget* Popup)
{
	if ( OpenPopups.Remove(Popup) > 0)
	{
		UpdateInputModeByPopup();
		return true;
	}
	return false;
}

void UURUISubsystem::UpdateInputModeByPopup()
{
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (APlayerController* PC = LP->GetPlayerController(GetWorld()))
		{
			const bool bHasOpenedPopup = !OpenPopups.IsEmpty();
		
			if (bHasOpenedPopup)
			{
				UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, OpenPopups.Last().Get());
			}
			else
			{
				UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
			}
		
			PC->SetShowMouseCursor(bHasOpenedPopup);
		}
	}
}

void UURUISubsystem::UpdateCinematicMode(bool bInCinematicMode)
{
	InitRootWidget();
	if (RootWidget)
	{
		RootWidget->SetVisibility(bInCinematicMode ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
	
	OnCinematicModeChanged.Broadcast(bInCinematicMode);
}

void UURUISubsystem::ClearRootWidget()
{
	if (IsValid(RootWidget))
	{
		RootWidget.Get()->RemoveFromParent();
	}
	RootWidget = nullptr;
}
