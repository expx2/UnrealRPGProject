// Written by EXPx2


#include "UI/URPopupWidget.h"

#include "URUISubsystem.h"
#include "HUD/URGameHUD.h"

UURPopupWidget::UURPopupWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UURPopupWidget::Close()
{
	RemoveFromParent();
}

void UURPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				UISystem->AddPopup(this);
			}
		}
	}
}

void UURPopupWidget::NativeDestruct()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				UISystem->RemovePopup(this);
			}
		}
	}
	
	Super::NativeDestruct();
}
