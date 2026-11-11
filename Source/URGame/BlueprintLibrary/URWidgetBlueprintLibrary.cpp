// Written by EXPx2


#include "BlueprintLibrary/URWidgetBlueprintLibrary.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "Interface/URActorReceiverInterface.h"
#include "UI/URUISubsystem.h"

UURPopupWidget* UURWidgetBlueprintLibrary::OpenPopup(APlayerController* PlayerController, EPopupType PopupType)
{
	if (PlayerController)
	{
		if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				return UISystem->OpenPopup(PopupType);
			}
		}
	}
	return nullptr;
}

void UURWidgetBlueprintLibrary::ClosePopup(APlayerController* PlayerController, EPopupType PopupType)
{
	if (PlayerController)
	{
		if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				UISystem->ClosePopup(PopupType);
			}
		}
	}
}

void UURWidgetBlueprintLibrary::ClosePopupAll(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				UISystem->CloseAllPopups();
			}
		}
	}
}

void UURWidgetBlueprintLibrary::DeriveActorToChild(UUserWidget* UserWidget, AActor* Actor)
{
	if (UserWidget)
	{
		if (UWidgetTree* WidgetTree = UserWidget->WidgetTree.Get())
		{
			WidgetTree->ForEachWidget([UserWidget, Actor](UWidget* ChildWidget)
			{
				if (UserWidget != ChildWidget)
				{
					if (ChildWidget->GetClass()->ImplementsInterface(UURActorReceiverInterface::StaticClass()))
					{
						IURActorReceiverInterface::Execute_ReceiveActor(ChildWidget, Actor);
					}
					if (UUserWidget* ChildUserWidget = Cast<UUserWidget>(ChildWidget))
					{
						DeriveActorToChild(ChildUserWidget, Actor);
					}
				}
			});
		}
	}
}

void UURWidgetBlueprintLibrary::DeriveAbilitySystemComponentToChild(
	UUserWidget* UserWidget,
	UAbilitySystemComponent* AbilitySystemComponent)
{
	if (UserWidget)
	{
		if (UWidgetTree* WidgetTree = UserWidget->WidgetTree.Get())
		{
			WidgetTree->ForEachWidget([UserWidget, AbilitySystemComponent](UWidget* ChildWidget)
			{
				if (UserWidget != ChildWidget)
				{
					if (ChildWidget->GetClass()->ImplementsInterface(UURAbilitySystemComponentReceiverInterface::StaticClass()))
					{
						IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(ChildWidget, AbilitySystemComponent);
					}
					if (UUserWidget* ChildUserWidget = Cast<UUserWidget>(ChildWidget))
					{
						DeriveAbilitySystemComponentToChild(ChildUserWidget, AbilitySystemComponent);
					}
				}
			});
		}
	}
}
