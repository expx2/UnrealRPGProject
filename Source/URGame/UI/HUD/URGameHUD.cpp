// Written by EXPx2


#include "UI/HUD/URGameHUD.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Player/URGamePlayerState.h"
#include "UI/URUISubsystem.h"
#include "UI/FloatingDamage/URFloatingDamageLayerWidget.h"
#include "UI/HUD/URGameHUDWidget.h"
#include "UObject/ConstructorHelpers.h"

AURGameHUD::AURGameHUD()
{
	static ConstructorHelpers::FClassFinder<UURGameHUDWidget> HUDWidgetRef(TEXT("/Game/_ProjectUR/UI/Game/HUD/WBP_GameHUD"));
	HUDWidgetClass = HUDWidgetRef.Class;
	
	static ConstructorHelpers::FClassFinder<UURFloatingDamageLayerWidget> FloatingDamageLayerWidgetRef(TEXT("/Game/_ProjectUR/UI/Game/FloatingDamage/WBP_FloatingDamageLayer"));
	FloatingDamageLayerWidgetClass = FloatingDamageLayerWidgetRef.Class;
	
	static ConstructorHelpers::FClassFinder<UUserWidget> AimTargetIndicatorWidgetRef(TEXT("/Game/_ProjectUR/UI/Game/Misc/WBP_AimTargetIndicator"));
	AimTargetIndicatorWidgetClass = AimTargetIndicatorWidgetRef.Class;
}

void AURGameHUD::ReceiveActor_Implementation(AActor* Actor)
{
	if (HUDWidget)
	{
		IURActorReceiverInterface::Execute_ReceiveActor(HUDWidget, Actor);
	}
}

void AURGameHUD::ClearActor_Implementation()
{
	if (HUDWidget)
	{
		IURActorReceiverInterface::Execute_ClearActor(HUDWidget);
	}
}

void AURGameHUD::ReceiveAbilitySystemComponent_Implementation(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (HUDWidget)
	{
		IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(HUDWidget, AbilitySystemComponent);
	}
}

void AURGameHUD::ClearAbilitySystemComponent_Implementation()
{
	if (HUDWidget)
	{
		IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(HUDWidget);
	}
}

void AURGameHUD::UpdateVisibility() const
{
	HUDWidget->SetVisibility(bShowHUD ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}


void AURGameHUD::ShowDamage(const AActor* HitActor, const float Damage, const bool bIsCritical) const
{
	if (IsValid(FloatingDamageLayerWidget) && IsValid(HitActor))
	{
		FloatingDamageLayerWidget->ShowDamage(HitActor, Damage, bIsCritical);
	}
}

void AURGameHUD::SetTargetActorIndicatorEnable(const bool bValue) const
{
	if (HUDWidget)
	{
		HUDWidget->SetTargetActorIndicatorEnable(bValue);
	}
}

void AURGameHUD::SetIsSpectating(const bool bValue)
{
	bIsSpectating = bValue;
	if (HUDWidget)
	{
		HUDWidget->SetIsSpectating(bValue);
	}
}

void AURGameHUD::SetViewTarget(AActor* ViewTarget)
{
	if (HUDWidget)
	{
		HUDWidget->SetViewTarget(ViewTarget);
	}
}

void AURGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeWidgets();
	
	if (APlayerController* PC = GetOwningPlayerController())
	{
		IURActorReceiverInterface::Execute_ReceiveActor(this, PC->GetPawn());
		if (AURGamePlayerState* GamePS = PC->GetPlayerState<AURGamePlayerState>())
		{
			IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(this, GamePS->GetAbilitySystemComponent());
		}
	}
}

void AURGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(HUDWidget))
	{
		// HUDWidget->Cleanup();
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}
	
	if (IsValid(AimTargetIndicatorWidget))
	{
		AimTargetIndicatorWidget->RemoveFromParent();
		AimTargetIndicatorWidget = nullptr;
	}

	
	if (IsValid(FloatingDamageLayerWidget))
	{
		FloatingDamageLayerWidget->RemoveFromParent();
		FloatingDamageLayerWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AURGameHUD::InitializeWidgets()
{
	APlayerController* PC = GetOwningPlayerController();
	ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr;
	UURUISubsystem* UISystem = LP ? LP->GetSubsystem<UURUISubsystem>() : nullptr;
	if (!PC || !LP || !UISystem)
	{
		return;
	}
	
	UR_LOG(LogUR, Log, TEXT("Init Widgets"));
	
	if (HUDWidgetClass && !HUDWidget)
	{
		HUDWidget = CreateWidget<UURGameHUDWidget>(PC, HUDWidgetClass);
		if (ensure(HUDWidget))
		{
			UISystem->AddSceneHUD(HUDWidget);
		}
	}
	
	if (AimTargetIndicatorWidgetClass && !AimTargetIndicatorWidget)
	{
		AimTargetIndicatorWidget = CreateWidget<UUserWidget>(PC, AimTargetIndicatorWidgetClass);
		if (ensure(AimTargetIndicatorWidget))
		{
			UISystem->AddGameWidget(AimTargetIndicatorWidget);
		}
	}

	if (FloatingDamageLayerWidgetClass && !FloatingDamageLayerWidget)
	{
		FloatingDamageLayerWidget = CreateWidget<UURFloatingDamageLayerWidget>(PC, FloatingDamageLayerWidgetClass);
		if (ensure(FloatingDamageLayerWidget))
		{
			UISystem->AddGameWidget(FloatingDamageLayerWidget);
		}
	}
	
	
}