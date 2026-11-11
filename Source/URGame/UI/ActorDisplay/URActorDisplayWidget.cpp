// Written by EXPx2


#include "UI/ActorDisplay/URActorDisplayWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UURActorDisplayWidget::SetName(const FName InName) const
{
	TxtName->SetText(FText::FromName(InName));
}

void UURActorDisplayWidget::SetNameColor(const FColor InColor) const
{
	TxtName->SetColorAndOpacity(FSlateColor(InColor));
}

void UURActorDisplayWidget::SetHealth(const float Health, const float MaxHealth) const
{
	if (Health < SMALL_NUMBER || 
		MaxHealth < SMALL_NUMBER)
	{
		PBHealth->SetPercent(0.f);
		return;
	}
	
	PBHealth->SetPercent(FMath::Clamp(Health/MaxHealth, 0.f , 1.f));
}

void UURActorDisplayWidget::SetIsTargeted(const bool bInValue)
{
	bIsTargeted = bInValue;
	UpdateBackplateVisible();
	UpdateHealthVisible();
}

void UURActorDisplayWidget::SetIsHit(const bool bInValue)
{
	bIsHit = bInValue;
	// UpdateBackplateVisible();
	UpdateHealthVisible();
}

void UURActorDisplayWidget::SetIsPlayer(const bool bInValue)
{
	bIsPlayer = bInValue;
	UpdateHealthVisible();
}

void UURActorDisplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 화면에 다시 추가될 때도 호출되므로 타겟/피격 상태를 유지한다.
	UpdateBackplateVisible();
	UpdateHealthVisible();
}

void UURActorDisplayWidget::UpdateBackplateVisible() const
{
	// 현재 타겟 구분이 잘 안되어 IsHit 조건 뺌
	const bool bIsVisible = bIsTargeted;//  || bIsHit;
	ImgBackplate->SetVisibility(bIsVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UURActorDisplayWidget::UpdateHealthVisible() const
{
	const bool bIsVisible = bIsTargeted || bIsHit || bIsPlayer;
	PBHealth->SetVisibility(bIsVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}
