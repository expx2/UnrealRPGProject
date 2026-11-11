// Written by EXPx2


#include "UI/ActorDisplay/URActorDisplayWidgetComponent.h"

#include "URActorDisplayWidget.h"
#include "URGame.h"
#include "Character/URHealthComponent.h"
#include "Character/URPlayerCharacter.h"
#include "Character/URTargetingComponent.h"
#include "Interface/URTargetableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UI/URUILayer.h"
#include "UI/URUISubsystem.h"

UURActorDisplayWidgetComponent::UURActorDisplayWidgetComponent()
{
	static ConstructorHelpers::FClassFinder<UURActorDisplayWidget> DisplayWidgetRef(TEXT("/Game/_ProjectUR/UI/Game/Misc/WBP_ActorDisplay"));
	if (DisplayWidgetRef.Class)
	{
		SetWidgetClass(DisplayWidgetRef.Class);
	}
	
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawSize(FVector2D(200.f, 50.f));
	SetDrawAtDesiredSize(true);
	SetPivot(FVector2D(0.5f, 1.0f));
	
	SetInitialSharedLayerName(TEXT("URActorDisplayLayer"));
	SetInitialLayerZOrder(URUILayer::WorldLayer);
}

void UURActorDisplayWidgetComponent::InitWidget()
{
	Clear();
	
	Super::InitWidget();
	
	UURActorDisplayWidget* ActorDisplayWidget = Cast<UURActorDisplayWidget>(GetWidget());
	if (!ActorDisplayWidget)
	{
		return;
	}
	
	AActor* Owner = GetOwner();
	
	// 이름 표시
	ActorDisplayWidget->SetName(IURTargetableInterface::Execute_GetDisplayName(Owner));
	RefreshNameColor();
	
	// 체력 델리게이트 등록
	if (UURHealthComponent* HealthComponent = Owner->FindComponentByClass<UURHealthComponent>())
	{
		BoundHealthComponent = HealthComponent;
		HealthComponent->OnMaxHealthChanged.AddUniqueDynamic(this, &ThisClass::HandleMaxHealthChanged);
		HealthComponent->OnHealthChanged.AddUniqueDynamic(this, &ThisClass::HandleHealthChanged);
		HealthComponent->OnDeath.AddUniqueDynamic(this, &ThisClass::HandleDeath);
		
		ActorDisplayWidget->SetHealth(HealthComponent->GetHealth(), HealthComponent->GetMaxHealth());
	}
	else
	{
		ActorDisplayWidget->SetHealth(0.f, 1.f);
	}
	
	// 타겟 표시
	ActorDisplayWidget->SetIsTargeted(false);
	ActorDisplayWidget->SetIsHit(false);
	
	// 플레이어 캐릭터의 타겟팅 컴포넌트에 연결
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->IsLocalController())
		{
			if (UURTargetingComponent* TargetingComponent = PC->FindComponentByClass<UURTargetingComponent>())
			{
				BoundTargetingComponent = TargetingComponent;
				TargetingComponent->OnMainTargetChanged.AddUniqueDynamic(this, &ThisClass::HandleTargetChanged);
				ActorDisplayWidget->SetIsTargeted(TargetingComponent->GetMainTargetActor() == Owner);
			}
		}
		
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				UISystem->OnCinematicModeChanged.AddDynamic(this, &ThisClass::HandleCinematicModeChanged);
			}
		}
		
		bCinematicMode = PC->bCinematicMode;
	}
	
	ActorDisplayWidget->SetIsPlayer(Owner->IsA(AURPlayerCharacter::StaticClass()));
	
	// Visibility 업데이트 타이머
	GetWorld()->GetTimerManager().SetTimer(RefreshVisibilityHandle, this, &ThisClass::RefreshVisibility, VisibilityRefreshInterval, true);
}

void UURActorDisplayWidgetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Clear();
	
	Super::EndPlay(EndPlayReason);
}

void UURActorDisplayWidgetComponent::RefreshNameColor() const
{
	if (!GetWidget())
	{
		return;
	}
	
	FColor NameColor = FColor::White;
	const AActor* Owner = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(Owner);
	if (OwnerPawn &&
		OwnerPawn->IsPlayerControlled() &&
		OwnerPawn->IsLocallyControlled())
	{
		// 플레이어면 초록색
		NameColor = FColor::Green;
	}
	else
	{
		switch (IURTargetableInterface::Execute_GetFaction(Owner))
		{
		case EURFaction::Enemy:
			// 적은 빨간색
			NameColor = FColor::Red;
			break;
		case EURFaction::Ally:
			// 아군은 청록색
			NameColor = FColor::Cyan;
			break;
		case EURFaction::Neutral:
		default:
			// 중립은 흰색
			break;
		}
	}
	
	const UURActorDisplayWidget* ActorDisplayWidget = Cast<UURActorDisplayWidget>(GetWidget());
	if (!ensure(ActorDisplayWidget))
	{
		return;
	}

	// 이름 표시
	ActorDisplayWidget->SetNameColor(NameColor);
}

void UURActorDisplayWidgetComponent::Clear()
{
	if (UURHealthComponent* HealthComponent = BoundHealthComponent.Get())
	{
		HealthComponent->OnMaxHealthChanged.RemoveDynamic(this, &ThisClass::HandleMaxHealthChanged);
		HealthComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
		HealthComponent->OnDeath.RemoveDynamic(this, &ThisClass::HandleDeath);
	}
	BoundHealthComponent.Reset();
	
	if (UURTargetingComponent* TargetingComponent = BoundTargetingComponent.Get())
	{
		TargetingComponent->OnMainTargetChanged.RemoveDynamic(this, &ThisClass::HandleTargetChanged);
	}
	BoundTargetingComponent.Reset();
	
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				UISystem->OnCinematicModeChanged.RemoveDynamic(this, &ThisClass::HandleCinematicModeChanged);
			}
		}
	}
	
	GetWorld()->GetTimerManager().ClearTimer(RefreshVisibilityHandle);
}

void UURActorDisplayWidgetComponent::ResetIsHit()
{
	if (UURActorDisplayWidget* ActorDisplayWidget = Cast<UURActorDisplayWidget>(GetWidget()))
	{
		ActorDisplayWidget->SetIsHit(false);
		RefreshVisibility();
	}
}

void UURActorDisplayWidgetComponent::RefreshVisibility()
{
	if (bCinematicMode)
	{
		SetVisibility(false);
		return;
	}
	
	UURActorDisplayWidget* ActorDisplayWidget = Cast<UURActorDisplayWidget>(GetWidget());
	if (!ensure(ActorDisplayWidget))
	{
		return;
	}
	
	if (ActorDisplayWidget->IsHighlighted())
	{
		SetVisibility(true, true);
		return;
	}
	
	const AActor* Actor = GetOwner();
	if (!IsValid(Actor))
	{
		SetVisibility(false);
		return;
	}

	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		SetVisibility(false);
		return;
	}

	// if (FVector::DistSquared(PC->GetPawn()->GetActorLocation(), Actor->GetActorLocation()) > FMath::Square(MaxDistanceToVisible))
	// {
	// 	SetVisibility(false);
	// 	return;
	// }

	FVector2D ScreenPosition;
	if (!PC->ProjectWorldLocationToScreen(GetComponentLocation(), ScreenPosition, true))
	{
		// Screen 포지션 얻기 실패
		SetVisibility(false);
		return;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);

	if (ScreenPosition.X < 0.0f ||
		ScreenPosition.Y < 0.0f ||
		ScreenPosition.X > ViewportX ||
		ScreenPosition.Y > ViewportY)
	{
		// 뷰포트 밖에 있음
		SetVisibility(false);
		return;
	}
	
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DisplayVisibility), true);
	QueryParams.AddIgnoredActor(PC->GetPawn());
	QueryParams.AddIgnoredActor(Actor);

	FHitResult VisibilityHit;
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		VisibilityHit,
		CameraLocation,
		GetComponentLocation(), // 이름표가 달린 위치
		ECC_Visibility,
		QueryParams);

	if (bBlocked && 
		VisibilityHit.GetActor() != Actor)
	{
		SetVisibility(false);
		return;
	}

	SetVisibility(true, true);
}

void UURActorDisplayWidgetComponent::HandleTargetChanged(
	UURTargetingComponent* TargetingComponent,
	AActor* OldTargetActor,
	AActor* NewTargetActor)
{
	if (UURActorDisplayWidget* ActorDisplayWidget = Cast<UURActorDisplayWidget>(GetWidget()))
	{
		ActorDisplayWidget->SetIsTargeted(NewTargetActor == GetOwner());
		RefreshVisibility();
	}
}

void UURActorDisplayWidgetComponent::HandleCinematicModeChanged(const bool bInCinematicMode)
{
	bCinematicMode = bInCinematicMode;
	RefreshVisibility();
}

void UURActorDisplayWidgetComponent::HandleMaxHealthChanged(
	UURHealthComponent* HealthComponent, 
	float OldValue,
	float NewValue, 
	AActor* Instigator)
{
	if (UURActorDisplayWidget* ActorDisplayWidget = Cast<UURActorDisplayWidget>(GetWidget()))
	{
		ActorDisplayWidget->SetHealth(HealthComponent->GetHealth(), HealthComponent->GetMaxHealth());
	} 
}

void UURActorDisplayWidgetComponent::HandleHealthChanged(
	UURHealthComponent* HealthComponent, 
	float OldValue,
	float NewValue, 
	AActor* Instigator)
{
	if (UURActorDisplayWidget* ActorDisplayWidget = Cast<UURActorDisplayWidget>(GetWidget()))
	{
		ActorDisplayWidget->SetHealth(HealthComponent->GetHealth(), HealthComponent->GetMaxHealth());
	} 
}

void UURActorDisplayWidgetComponent::HandleDeath(AActor* Actor)
{
	if (GetOwner() != Actor)
	{
		return;
	}
	
	Clear();
}
