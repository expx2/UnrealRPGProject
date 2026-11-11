// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSet/URAttributeSet.h"
#include "URPlayerResourceSet.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURPlayerResourceSet : public UURAttributeSet
{
	GENERATED_BODY()
	
public:
	UURPlayerResourceSet();
	
	ATTRIBUTE_ACCESSORS(UURPlayerResourceSet, MaxMana);
	ATTRIBUTE_ACCESSORS(UURPlayerResourceSet, Mana);
	ATTRIBUTE_ACCESSORS(UURPlayerResourceSet, ManaRegen);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	// AttributeSet은 const로 얻으므로 델리게이트 등록/삭제를 위해 mutable로 선언한다.
	mutable FURAttributeSimpleEvent OnMaxManaChanged;
	mutable FURAttributeSimpleEvent OnManaChanged;
	
protected:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_ManaRegen(const FGameplayAttributeData& OldValue);
	
	// 최대 마나
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMana, Category="UR")
	FGameplayAttributeData MaxMana;
	
	// 마나
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mana, Category="UR")
	FGameplayAttributeData Mana;
	
	// 마나 회복량
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_ManaRegen, Category="UR")
	FGameplayAttributeData ManaRegen;
	
	// GE 실행 전 값
	float MaxManaBeforeAttributeChange;
	float ManaBeforeAttributeChange;
};
