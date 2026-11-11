// Written by EXPx2


#include "UI/Common/URHealthBarWidget.h"

#include "Character/URHealthComponent.h"
#include "Components/ProgressBar.h"

void UURHealthBarWidget::ReceiveActor_Implementation(AActor* Actor)
{
	IURActorReceiverInterface::Execute_ClearActor(this);
	
	if (Actor)
	{
		if (UURHealthComponent* HealthComponent = Actor->FindComponentByClass<UURHealthComponent>())
		{
			BoundHealthComponentPtr = HealthComponent;
			HealthComponent->OnMaxHealthChanged.AddUniqueDynamic(this, &ThisClass::HandleMaxHealthChanged);
			HealthComponent->OnHealthChanged.AddUniqueDynamic(this, &ThisClass::HandleHealthChanged);

			HandleMaxHealthChanged(HealthComponent, 0.f, 0.f, nullptr);
			return;
		}
	}
	SetNonHealth();
}

void UURHealthBarWidget::ClearActor_Implementation()
{
	if (UURHealthComponent* HealthComponent = BoundHealthComponentPtr.Get())
	{
		HealthComponent->OnMaxHealthChanged.RemoveDynamic(this, &ThisClass::HandleMaxHealthChanged);
		HealthComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
	}
	BoundHealthComponentPtr.Reset();
	
	SetNonHealth();
}

void UURHealthBarWidget::NativeDestruct()
{
	IURActorReceiverInterface::Execute_ClearActor(this);
	
	Super::NativeDestruct();
}

void UURHealthBarWidget::SetNonHealth_Implementation()
{
	PBHealth->SetPercent(0.0f);
}

void UURHealthBarWidget::HandleMaxHealthChanged_Implementation(
	UURHealthComponent* HealthComponent, 
	float OldValue, 
	float NewValue,
	AActor* Instigator)
{
	PBHealth->SetPercent(HealthComponent->GetPercent());
}

void UURHealthBarWidget::HandleHealthChanged_Implementation(
	UURHealthComponent* HealthComponent, 
	float OldValue, 
	float NewValue,
	AActor* Instigator)
{
	PBHealth->SetPercent(HealthComponent->GetPercent());
}
