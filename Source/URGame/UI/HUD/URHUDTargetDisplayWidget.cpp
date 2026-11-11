// Written by EXPx2


#include "UI/HUD/URHUDTargetDisplayWidget.h"

#include "Character/URTargetingComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interface/URTargetableInterface.h"
#include "UI/Common/URHealthBarWidget.h"

void UURHUDTargetDisplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetVisibility(ESlateVisibility::Collapsed);

	if (const AController* Controller = GetOwningPlayer())
	{
		if (UURTargetingComponent* TargetingComponent = Controller->FindComponentByClass<UURTargetingComponent>())
		{
			BoundTargetingComponent = TargetingComponent;
			TargetingComponent->OnMainTargetChanged.AddUniqueDynamic(this, &ThisClass::HandleTargetChanged);
			SetTarget(TargetingComponent->GetMainTargetActor());
		}
	}
}

void UURHUDTargetDisplayWidget::NativeDestruct()
{

	if (UURTargetingComponent* TargetingComponent = BoundTargetingComponent.Get())
	{
		TargetingComponent->OnMainTargetChanged.RemoveDynamic(this, &ThisClass::HandleTargetChanged);
	}
	BoundTargetingComponent.Reset();
	
	ClearCurrentTarget();
	
	Super::NativeDestruct();
}

void UURHUDTargetDisplayWidget::SetTarget(AActor* Actor)
{
	if (IsValid(Actor) && CurrentTargetPtr.Get() == Actor)
	{
		return;
	}
	
	ClearCurrentTarget();
	
	CurrentTargetPtr = Actor;
	
	if (!IsValid(Actor))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	TxtName->SetText(FText::FromName(IURTargetableInterface::Execute_GetDisplayName(Actor)));
	
	IURActorReceiverInterface::Execute_ReceiveActor(HealthBar, Actor);
	
	// 거리 업데이트 타이머 등록
	constexpr float DistanceUpdateInterval = 0.1f;
	GetWorld()->GetTimerManager().SetTimer(UpdateDistanceTimerHandle, this, &ThisClass::UpdateDistance, DistanceUpdateInterval, true);
	// 초기 설정
	UpdateDistance();
	
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UURHUDTargetDisplayWidget::ClearCurrentTarget()
{
	IURActorReceiverInterface::Execute_ClearActor(HealthBar);
	
	// 거리 업데이트 타이머 제거
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateDistanceTimerHandle);
	}
	UpdateDistanceTimerHandle.Invalidate();
	
	CurrentTargetPtr.Reset();
}

void UURHUDTargetDisplayWidget::UpdateDistance() const
{
	const APawn* Pawn = GetOwningPlayerPawn();
	if (!IsValid(Pawn) ||
		!CurrentTargetPtr.IsValid())
	{
		TxtDistance->SetText(FText::FromString("-m"));
		return;
	}
	
	const float Distance = FVector::Distance(Pawn->GetActorLocation(), CurrentTargetPtr->GetActorLocation());
	TxtDistance->SetText(FText::FromString(FString::Printf(TEXT("%dm"), FMath::RoundToInt(Distance * 0.01f))));
}

void UURHUDTargetDisplayWidget::HandleTargetChanged(UURTargetingComponent* TargetingComponent, AActor* OldTargetActor, AActor* NewTargetActor)
{
	SetTarget(NewTargetActor);
}