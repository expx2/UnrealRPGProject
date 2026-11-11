// Written by EXPx2


#include "Character/URPlayerResourceComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/URPlayerResourceSet.h"

UURPlayerResourceComponent::UURPlayerResourceComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	AbilityComponent.Reset();
	PlayerResourceSet.Reset();
	
	static ConstructorHelpers::FClassFinder<UGameplayEffect> ManaRegenEffectRef(TEXT("/Game/_ProjectUR/GAS/GameplayEffects/BPGE_ManaRegen"));
	ManaRegenEffect = ManaRegenEffectRef.Class;
}

void UURPlayerResourceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	Super::EndPlay(EndPlayReason);
}

void UURPlayerResourceComponent::ReceiveAbilitySystemComponent_Implementation(UAbilitySystemComponent* AbilitySystemComponent)
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	
	AbilityComponent = AbilitySystemComponent;
	
	// if (GetOwner() && GetOwner()->HasAuthority())
	// {
	// 	// 마나 초기화
	// 	const float OldMana = GetMana();
	// 	AbilitySystemComponent->SetNumericAttributeBase(UURPlayerResourceSet::GetManaAttribute(), GetMaxMana());
	// 	const float NewMana = GetMana();
	//
	// 	HandleManaChanged(NewMana - OldMana, OldMana, NewMana);
	// 	HandleMaxManaChanged(NewMana - OldMana, OldMana, NewMana);
	// }
	
	PlayerResourceSet = AbilitySystemComponent->GetSet<UURPlayerResourceSet>();
	
	if (PlayerResourceSet.IsValid())
	{
		MaxManaChangedDelegateHandle = PlayerResourceSet->OnMaxManaChanged.AddUObject(this, &ThisClass::HandleMaxManaChanged);
		ManaChangedDelegateHandle = PlayerResourceSet->OnManaChanged.AddUObject(this, &ThisClass::HandleManaChanged);

		// ASC 연결 전에 놓친 초기 상태를 리스너에게 전달
		const float CurrentMaxMana = GetMaxMana();
		const float CurrentMana = GetMana();

		OnMaxManaChanged.Broadcast(this, CurrentMaxMana, CurrentMaxMana);
		OnManaChanged.Broadcast(this, CurrentMana, CurrentMana);

		UpdateManaRegen();
	}
}

void UURPlayerResourceComponent::ClearAbilitySystemComponent_Implementation()
{
	// 델리게이트 해제
	if (PlayerResourceSet.IsValid())
	{
		PlayerResourceSet->OnMaxManaChanged.Remove(MaxManaChangedDelegateHandle);
		PlayerResourceSet->OnManaChanged.Remove(ManaChangedDelegateHandle);
	}
	MaxManaChangedDelegateHandle.Reset();
	ManaChangedDelegateHandle.Reset();
	
	// 리젠 GE 삭제
	if (AbilityComponent.IsValid() && ManaRegenSpecHandle.IsValid())
	{
		AbilityComponent->RemoveActiveGameplayEffect(ManaRegenSpecHandle);
	}
	ManaRegenSpecHandle.Invalidate();
	
	// 초기화
	AbilityComponent.Reset();
	PlayerResourceSet.Reset();
}

float UURPlayerResourceComponent::GetMaxMana() const
{
	return PlayerResourceSet.IsValid() ? PlayerResourceSet->GetMaxMana() : 0.0f;
}

float UURPlayerResourceComponent::GetMana() const
{
	return PlayerResourceSet.IsValid() ? PlayerResourceSet->GetMana() : 0.0f;
}

void UURPlayerResourceComponent::SetManaToMax()
{
	if (GetOwner() &&
		GetOwner()->HasAuthority() &&
		AbilityComponent.IsValid())
	{
		const float OldMana = GetMana();
		AbilityComponent->SetNumericAttributeBase(UURPlayerResourceSet::GetManaAttribute(), GetMaxMana());
		const float NewMana = GetMana();

		HandleManaChanged(NewMana - OldMana, OldMana, NewMana);
	}
}

void UURPlayerResourceComponent::HandleMaxManaChanged(float EffectMagnitude, float OldValue, float NewValue)
{
	UpdateManaRegen();

	OnMaxManaChanged.Broadcast(this, OldValue, NewValue);
}

void UURPlayerResourceComponent::HandleManaChanged(float EffectMagnitude, float OldValue, float NewValue)
{
	UpdateManaRegen();

	OnManaChanged.Broadcast(this, OldValue, NewValue);
}

void UURPlayerResourceComponent::UpdateManaRegen()
{
	if (!ManaRegenEffect ||
		!AbilityComponent.IsValid() || 
		!PlayerResourceSet.IsValid())
	{
		return;
	}
	
	if (UURAbilitySystemComponent* URASC = Cast<UURAbilitySystemComponent>(AbilityComponent.Get()))
	{
		if (URASC->IsDead())
		{
			return;
		}
	}
	
	const bool bIsMax = PlayerResourceSet->GetMana() >= PlayerResourceSet->GetMaxMana();
	if (bIsMax)
	{
		if (ManaRegenSpecHandle.IsValid())
		{
			// GE 삭제
			AbilityComponent->RemoveActiveGameplayEffect(ManaRegenSpecHandle);
		}
		ManaRegenSpecHandle.Invalidate();
	}
	else
	{
		if (!ManaRegenSpecHandle.IsValid())
		{
			// GE 적용
			const FGameplayEffectSpecHandle Handle = AbilityComponent->MakeOutgoingSpec(ManaRegenEffect, 1, AbilityComponent->MakeEffectContext());
			ManaRegenSpecHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*Handle.Data);
		}
	}
}
