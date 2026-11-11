// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSet/URAttributeSet.h"
#include "URHealthSet.generated.h"

DECLARE_MULTICAST_DELEGATE_FourParams(FURDamageTakenEvent,
									 AActor* /*EffectInstigator*/,
									 AActor* /*EffectCauser*/,
									 const FGameplayEffectSpec* /*EffectSpec*/,
									 float /*EffectMagnitude*/);

/**
 * 
 */
UCLASS()
class URGAME_API UURHealthSet : public UURAttributeSet
{
	GENERATED_BODY()
	
public:
	UURHealthSet();
	
	ATTRIBUTE_ACCESSORS(UURHealthSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UURHealthSet, Health);
	ATTRIBUTE_ACCESSORS(UURHealthSet, HealthRegen);
	ATTRIBUTE_ACCESSORS(UURHealthSet, DamageTaken);
	ATTRIBUTE_ACCESSORS(UURHealthSet, Healing);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	/*
	 * Note: AttributeSet은 어트리뷰트의 수정을 막기 위해 const로만 얻을 수 있다.
	 * 델리게이트에 등록/삭제를 위해 mutable로 선언한다.
	 */
	// Delegate when takes damage
	mutable FURDamageTakenEvent OnDamageTaken;
	
	// Delegate when max health changes
	mutable FURAttributeEvent OnMaxHealthChanged;
	
	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FURAttributeEvent OnHealthChanged;
	
	// Delegate to broadcast when the health attribute reaches zero
	mutable FURAttributeEvent OnOutOfHealth;
	
protected:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& OldValue);
	
	// 최대 체력
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="UR")
	FGameplayAttributeData MaxHealth;
	
	// 체력
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="UR")
	FGameplayAttributeData Health;
	
	// 체력 회복량
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HealthRegen, Category="UR")
	FGameplayAttributeData HealthRegen;
	
	// Used to track when the health reaches 0.
	bool bIsOutOfHealth;
	
	// Store the health before any changes 
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;
	
	// 메타 어트리뷰트
	
	// 받은 대미지
	UPROPERTY(BlueprintReadOnly, Category="UR")
	FGameplayAttributeData DamageTaken;
	
	// 받은 치유량
	UPROPERTY(BlueprintReadOnly, Category="UR")
	FGameplayAttributeData Healing;
};
