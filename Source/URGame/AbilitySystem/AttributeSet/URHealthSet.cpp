// Written by EXPx2


#include "AbilitySystem/AttributeSet/URHealthSet.h"

#include "GameplayEffectExtension.h"
#include "URGame.h"
#include "Net/UnrealNetwork.h"

UURHealthSet::UURHealthSet() :
	MaxHealth(1000.0f),
	Health(1000.0f),
	HealthRegen(20.f),
	DamageTaken(0.0f),
	Healing(0.0f)
{
}

void UURHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UURHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UURHealthSet, Health, COND_None, REPNOTIFY_Always);
	// 스탯 표시 위해 오너만 복제
	// 이 프로젝트에서 필요 없긴 함
	DOREPLIFETIME_CONDITION_NOTIFY(UURHealthSet, HealthRegen, COND_OwnerOnly, REPNOTIFY_Always);
}

bool UURHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}
	
	// GE 실행 전 현재 값 저장
	MaxHealthBeforeAttributeChange = GetMaxHealth();
	HealthBeforeAttributeChange = GetHealth();
	
	return true;
}

void UURHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();
	
	bool bIsHealthChanged = false;
	if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
	{
		if (!Data.Target.HasMatchingGameplayTag(URGameplayTags::Character_State_Invincible))
		{
			OnDamageTaken.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude);

			if (Data.Target.HasMatchingGameplayTag(URGameplayTags::Cheat_InfiniteHealth))
			{
				// 체력 루프
				float CurrentHealth = GetHealth();
				float RemainingDamage = GetDamageTaken();
				while (RemainingDamage > 0.f)
				{
					if (CurrentHealth > RemainingDamage)
					{
						CurrentHealth -= RemainingDamage;
						RemainingDamage = 0.f;
					}
					else
					{
						RemainingDamage -= CurrentHealth;
						CurrentHealth = GetMaxHealth();
					}
					bIsHealthChanged = true;
				}
			
				SetHealth(FMath::Clamp(CurrentHealth, 0.0f, GetMaxHealth()));
			}
			else
			{
				SetHealth(FMath::Clamp(GetHealth() - GetDamageTaken(), 0.0f, GetMaxHealth()));
			}
		}
		SetDamageTaken(0.0f);
	}
	
	// GE 적용 후 값이 변경된 경우
	if (GetMaxHealth() != MaxHealthBeforeAttributeChange)
	{
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
	}
	
	bIsHealthChanged |= GetHealth() != HealthBeforeAttributeChange;
	if (bIsHealthChanged)
	{
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}
	
	if (!bIsOutOfHealth &&
		GetHealth() <= SMALL_NUMBER)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
		bIsOutOfHealth = true;
	}
}

void UURHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UURHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UURHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMaxHealthAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetHealth() > NewValue)
		{
			// 최대 체력이 바뀌었을 때 체력 상한 설정
			UURAbilitySystemComponent* URASC = GetOwningURAbilitySystemComponent();
			check(URASC);
	
			URASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bIsOutOfHealth && 
		GetHealth() > SMALL_NUMBER)
	{
		bIsOutOfHealth = false;
	}
}

void UURHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UURHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURHealthSet, MaxHealth, OldValue);

	const float CurrentMaxHealth = GetMaxHealth();
	const float EstimatedMagnitude = CurrentMaxHealth - OldValue.GetCurrentValue();
	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentMaxHealth);
}

void UURHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURHealthSet, Health, OldValue);
	
	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	// These events on the client should not be changing attributes

	const float CurrentHealth = GetHealth();
	const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
	
	OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
	
	if (CurrentHealth > SMALL_NUMBER)
	{
		bIsOutOfHealth = false;
	}
	else if (!bIsOutOfHealth)
	{
		OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
		bIsOutOfHealth = true;
	}
}

void UURHealthSet::OnRep_HealthRegen(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURHealthSet, HealthRegen, OldValue);
}
