// Written by EXPx2


#include "Character/URHealthComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/URHealthSet.h"
#include "AbilitySystem/GameplayEffect/URHitGameplayEffectContext.h"
#include "AbilitySystem/GameplayEffect/GameplayEffect_Dead.h"
#include "GameFramework/PlayerState.h"
#include "Player/URGamePlayerController.h"
#include "Player/URGamePlayerState.h"

UURHealthComponent::UURHealthComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	
	AbilityComponent.Reset();
	HealthSet.Reset();
	
	static ConstructorHelpers::FClassFinder<UGameplayEffect> HealthRegenEffectRef(TEXT("/Game/_ProjectUR/GAS/GameplayEffects/BPGE_HealthRegen"));
	HealthRegenEffect = HealthRegenEffectRef.Class;
}

void UURHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	Super::EndPlay(EndPlayReason);
}

void UURHealthComponent::ReceiveAbilitySystemComponent_Implementation(UAbilitySystemComponent* AbilitySystemComponent)
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	
	AbilityComponent = AbilitySystemComponent;
	
	// if (GetOwner() && GetOwner()->HasAuthority())
	// {
	// 	// 체력 초기화
	// 	const float OldHealth = GetHealth();
	// 	AbilityComponent->SetNumericAttributeBase(UURHealthSet::GetHealthAttribute(), GetMaxHealth());
	// 	const float NewHealth = GetHealth();
	//
	// 	HandleHealthChanged(nullptr, nullptr, nullptr, NewHealth - OldHealth, OldHealth, NewHealth);
	// 	HandleMaxHealthChanged(nullptr, nullptr, nullptr, NewHealth - OldHealth, OldHealth, NewHealth);
	// }
	
	HealthSet = AbilitySystemComponent->GetSet<UURHealthSet>();
	
	if (HealthSet.IsValid())
	{
		DamageTakenDelegateHandle = HealthSet->OnDamageTaken.AddUObject(this, &ThisClass::HandleDamageTaken);
		MaxHealthChangedDelegateHandle = HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
		HealthChangedDelegateHandle = HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
		OutOfHealthDelegateHandle = HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

		// ASC 연결 전에 놓친 초기 상태를 리스너에게 전달
		const float CurrentMaxHealth = GetMaxHealth();
		const float CurrentHealth = GetHealth();
		
		OnMaxHealthChanged.Broadcast(this, CurrentMaxHealth, CurrentMaxHealth, nullptr);
		OnHealthChanged.Broadcast(this, CurrentHealth, CurrentHealth, nullptr);
		
		UpdateHealthRegen();
	}
}

void UURHealthComponent::ClearAbilitySystemComponent_Implementation()
{
	// 델리게이트 해제
	if (HealthSet.IsValid())
	{
		HealthSet->OnDamageTaken.Remove(DamageTakenDelegateHandle);
		HealthSet->OnMaxHealthChanged.Remove(MaxHealthChangedDelegateHandle);
		HealthSet->OnHealthChanged.Remove(HealthChangedDelegateHandle);
		HealthSet->OnOutOfHealth.Remove(OutOfHealthDelegateHandle);
	}
	DamageTakenDelegateHandle.Reset();
	MaxHealthChangedDelegateHandle.Reset();
	HealthChangedDelegateHandle.Reset();
	OutOfHealthDelegateHandle.Reset();
	
	// 리젠 GE 삭제
	if (AbilityComponent.IsValid() && HealthRegenSpecHandle.IsValid())
	{
		AbilityComponent->RemoveActiveGameplayEffect(HealthRegenSpecHandle);
	}
	HealthRegenSpecHandle.Invalidate();
	
	// 초기화
	AbilityComponent.Reset();
	HealthSet = nullptr;
}

// void UURHealthComponent::ResetHealth()
// {
// 	if (GetOwner() && GetOwner()->HasAuthority() && URASC)
// 	{
// 		const float OldHealth = GetHealth();
// 		URASC->SetNumericAttributeBase(UURHealthSet::GetHealthAttribute(), GetMaxHealth());
// 		const float NewHealth = GetHealth();
//
// 		HandleHealthChanged(nullptr, nullptr, nullptr, NewHealth - OldHealth, OldHealth, NewHealth);
// 		HandleMaxHealthChanged(nullptr, nullptr, nullptr, NewHealth - OldHealth, OldHealth, NewHealth);
// 	}
// }

float UURHealthComponent::GetMaxHealth() const
{
	return HealthSet.IsValid() ? HealthSet->GetMaxHealth() : 0.0f;
}

float UURHealthComponent::GetHealth() const
{
	return HealthSet.IsValid() ? HealthSet->GetHealth() : 0.0f;
}

void UURHealthComponent::SetHealthToMax()
{
	if (GetOwner() && 
		GetOwner()->HasAuthority() && 
		AbilityComponent.IsValid())
	{
		const float OldHealth = GetHealth();
		AbilityComponent->SetNumericAttributeBase(UURHealthSet::GetHealthAttribute(), GetMaxHealth());
		const float NewHealth = GetHealth();
		
		HandleHealthChanged(nullptr, nullptr, nullptr, NewHealth - OldHealth, OldHealth, NewHealth);
	}
}

void UURHealthComponent::HandleDamageTaken(
	AActor* EffectInstigator,
	AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec,
	float EffectMagnitude)
{
	// 대미지 처리는 서버에서만 진행되므로 해당 함수도 서버에서만 실행된다.
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || 
		!Owner->HasAuthority() ||
		EffectMagnitude <= SMALL_NUMBER)
	{
		return;
	}

	OnDamageTaken.Broadcast(EffectInstigator, EffectCauser, EffectMagnitude);
	
	bool bIsCritical = false;
	
	if (EffectSpec)
	{
		if (FGameplayEffectContext* Context = EffectSpec->GetContext().Get())
		{
			// 크리티컬 여부 등 추가 정보 얻기 위해 HitGEContext 검사
			if (Context->GetScriptStruct()->IsChildOf(FURHitGameplayEffectContext::StaticStruct()))
			{
				const FURHitGameplayEffectContext* DamageContext = static_cast<FURHitGameplayEffectContext*>(Context);
				bIsCritical = DamageContext->bIsCritical;
				
				if (AbilityComponent.IsValid() && DamageContext->HitImpactCueTag.IsValid())
				{
					// 피격 Cue 전달
					FGameplayCueParameters Params;
					Params.EffectContext = EffectSpec->GetContext();
					Params.Instigator = EffectInstigator;
					Params.EffectCauser = EffectCauser;

					AbilityComponent->ExecuteGameplayCue(DamageContext->HitImpactCueTag, Params);
				}
			}
		}
	}
	
	// RPC 전송
	// 로컬 플레이어일 경우에만 RPC 전송
	
	// 플레이어가 피격 당함
	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (Pawn->IsPlayerControlled())
		{
			if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(Pawn->GetController()))
			{
				GamePC->ClientDamageTakenFeedback(EffectInstigator, Owner, EffectMagnitude, bIsCritical);
			}
		}
	}
	
	// 플레이어가 타격함
	if ((EffectInstigator && EffectInstigator != Owner) ||
		(EffectCauser && EffectCauser != Owner))
	{
		// 플레이어가 타격했을 경우 Instigator는 PlayerState임.
		if (AURGamePlayerState* GamePS = Cast<AURGamePlayerState>(EffectInstigator))
		{
			if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(GamePS->GetPlayerController()))
			{
				GamePC->ClientDamageTakenFeedback(EffectInstigator, Owner, EffectMagnitude, bIsCritical);
			}
		}
	}
}

void UURHealthComponent::HandleMaxHealthChanged(
	AActor* EffectInstigator,
	AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec,
	float EffectMagnitude,
	float OldValue,
	float NewValue)
{
	UpdateHealthRegen();
	
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, EffectInstigator);
}

void UURHealthComponent::HandleHealthChanged(
	AActor* EffectInstigator,
	AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec,
	float EffectMagnitude,
	float OldValue,
	float NewValue)
{
	UpdateHealthRegen();
	
	OnHealthChanged.Broadcast(this, OldValue, NewValue, EffectInstigator);
}

void UURHealthComponent::HandleOutOfHealth(
	AActor* EffectInstigator,
	AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec,
	float EffectMagnitude,
	float OldValue,
	float NewValue)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (UURAbilitySystemComponent* URASC = Cast<UURAbilitySystemComponent>(AbilityComponent.Get()))
		{
			if (URASC->IsDead())
			{
				return;
			}
		}
	}

	// Death Event  
	// FGameplayEventData Payload;
	// Payload.EventTag = URGameplayTags::GameplayEvent_Death;
	// Payload.Instigator = EffectInstigator;
	// Payload.Target = URASC->GetAvatarActor();
	// if (EffectSpec)
	// {
	// 	Payload.OptionalObject = EffectSpec->Def;
	// 	Payload.ContextHandle = EffectSpec->GetEffectContext();
	// 	Payload.InstigatorTags = *EffectSpec->CapturedSourceTags.GetAggregatedTags();
	// 	Payload.TargetTags = *EffectSpec->CapturedTargetTags.GetAggregatedTags();
	// }
	// Payload.EventMagnitude = EffectMagnitude;
	//
	// FScopedPredictionWindow NewScopedWindow(URASC, true);
	// URASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	
	OnDeath.Broadcast(GetOwner());
}

void UURHealthComponent::UpdateHealthRegen()
{
	if (!HealthRegenEffect ||
		!AbilityComponent.IsValid() || 
		!HealthSet.IsValid())
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
	
	const bool bIsMax = HealthSet->GetHealth() >= HealthSet->GetMaxHealth();
	if (bIsMax)
	{
		if (HealthRegenSpecHandle.IsValid())
		{
			// GE 삭제
			AbilityComponent->RemoveActiveGameplayEffect(HealthRegenSpecHandle);
		}
		HealthRegenSpecHandle.Invalidate();
	}
	else
	{
		if (!HealthRegenSpecHandle.IsValid())
		{
			// GE 적용
			const FGameplayEffectSpecHandle Handle = AbilityComponent->MakeOutgoingSpec(HealthRegenEffect, 1, AbilityComponent->MakeEffectContext());
			HealthRegenSpecHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*Handle.Data);
		}
	}
}



