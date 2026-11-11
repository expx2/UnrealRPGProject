// Written by EXPx2


#include "UI/FloatingDamage/URFloatingDamageWidget.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"

void UURFloatingDamageWidget::ShowDamage(const float Damage)
{
	if (TxtDamage)
	{
		TxtDamage->SetText(FText::AsNumber(FMath::RoundToInt(Damage)));
	}

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RemoveTimerHandle, this, &ThisClass::RemoveDamage, 2.f, false);
	}
	
	OnShowDamage(Damage);
}

void UURFloatingDamageWidget::NativeDestruct()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RemoveTimerHandle);
	}
	RemoveTimerHandle.Invalidate();
	
	
	Super::NativeDestruct();
}

void UURFloatingDamageWidget::RemoveDamage()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RemoveTimerHandle);
	}
	RemoveTimerHandle.Invalidate();
	
	Release.ExecuteIfBound(this);
}

