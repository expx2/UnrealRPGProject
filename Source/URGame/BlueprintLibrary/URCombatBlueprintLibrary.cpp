// Written by EXPx2


#include "BlueprintLibrary/URCombatBlueprintLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/AttributeSet/URCombatSet.h"
#include "AbilitySystem/GameplayEffect/Calculation/URDamageExecutionCalculation.h"
#include "GameplayEffect.h"
#include "AbilitySystem/GameplayEffect/GameplayEffect_Damage.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Definitions/URFaction.h"
#include "GameplayTags/URGameplayTags.h"
#include "Interface/URDamageableInterface.h"
#include "Interface/URTargetableInterface.h"


EURFaction UURCombatBlueprintLibrary::GetRelativeTargetFaction(const EURFaction OriginFaction, const AActor* SourceActor)
{
	EURFaction OwnerFaction = EURFaction::None;
	if (IsValid(SourceActor))
	{
		if (SourceActor->GetClass()->ImplementsInterface(UURTargetableInterface::StaticClass()))
		{
			OwnerFaction = IURTargetableInterface::Execute_GetFaction(SourceActor);
		}
		else if (SourceActor->GetClass()->ImplementsInterface(UURDamageableInterface::StaticClass()))
		{
			OwnerFaction = IURDamageableInterface::Execute_GetFaction(SourceActor);
		}
	}
	
	if (OwnerFaction != EURFaction::Enemy)
	{
		return OriginFaction;
	}
	// Enemy일 경우, Ally, Enemy 스위칭
	EURFaction Result = OriginFaction & ~(EURFaction::Ally | EURFaction::Enemy);
	if (EnumHasAnyFlags(OriginFaction, EURFaction::Ally))
	{
		Result |= EURFaction::Enemy;
	}
	if (EnumHasAnyFlags(OriginFaction, EURFaction::Enemy))
	{
		Result |= EURFaction::Ally;
	}
	return Result;
}

void UURCombatBlueprintLibrary::ResolveTargets(
	TArray<AActor*>& TargetableActors,
	const int MaxTargetCount,
	AActor* MainTargetActor, 
	TArray<AActor*>& PriorTargets, 
	TArray<AActor*>& OutHitActors)
{
	
	OutHitActors.Empty(MaxTargetCount);
	
	int SetCount = 0;
	// 메인 타겟 우선 등록 검사
	if (MainTargetActor && TargetableActors.Contains(MainTargetActor))
	{
		OutHitActors.Add(MainTargetActor);
		SetCount++;
	}
	
	// 이전 히트에서 처리한 대상
	TArray<AActor*> PriorTargetableActors;
	PriorTargetableActors.Reserve(MaxTargetCount - SetCount);
	int PriorTargetableSetCount = 0;
	
	// 그 외
	TArray<AActor*> NormalTargetableActors;
	NormalTargetableActors.Reserve(MaxTargetCount - SetCount);
	int NormalTargetableSetCount = 0;
	
	for (AActor* Actor : TargetableActors)
	{
		// 어빌리티 타겟
		if (const bool bIsMainTarget = Actor == MainTargetActor)
		{
			// 위에서 이미 추가함
			continue;
		}
				
		// 이전 히트에서 처리한 대상
		if (const bool bIsPriorTarget = PriorTargets.Contains(Actor))
		{
			PriorTargetableActors.Add(Actor);
			PriorTargetableSetCount++;
			if (SetCount + PriorTargetableSetCount >= MaxTargetCount)
			{
				// 우선권 가진 액터들로만 최대 타격 수를 모두 채움
				break;
			}
			continue;
		}
				
		// 그 외
		if (SetCount + PriorTargetableSetCount + NormalTargetableSetCount < MaxTargetCount)
		{
			// 수를 초과하는 액터 저장할 필요 없음
			NormalTargetableActors.Add(Actor);
			NormalTargetableSetCount++;
		}
		// Note: NormalTargetableActors로 MaxTargetCount를 모두 채웠더라도, 우선권을 가진 액터가 리스트 뒤에 더 있을 수 있기 때문에 반복을 유지한다.
	}
	
	// 저장한 액터들을 통해 최종적으로 공격할 HitActors 설정
	/*
	 * Note: ASC가 없는 타겟이어도 SetCount를 증가시키는데, 프랍을 포함시켜 타격하는 적의 수가 줄을 수 있음
	 * 프랍은 SetCount 설정 안 하게 수정할 경우 상단의 반복문 조기 종료 수정 필요
	 */
	for (AActor* Actor : PriorTargetableActors)
	{
		if (SetCount >= MaxTargetCount)
		{
			break;
		}
		OutHitActors.Add(Actor);
		SetCount++;
	}
			
	for (AActor* Actor : NormalTargetableActors)
	{
		if (SetCount >= MaxTargetCount)
		{
			break;
		}
		OutHitActors.Add(Actor);
		SetCount++;
	}
}

namespace
{
	FGameplayEffectSpecHandle MakeHitSpec(
		UAbilitySystemComponent* SourceASC, 
		TSubclassOf<UGameplayEffect> EffectClass,
		const FGameplayEffectContextHandle& EffectContext)
	{
		// UGameplayAbility::MakeOutgoingGameplayEffectSpec 참고
		FGameplayEffectSpecHandle NewHandle = SourceASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
		if (NewHandle.IsValid())
		{
			// Ability::MakeOutgoingGameplayEffectSpec가 전달하던 어빌리티 태그와 SetByCaller 값을 유지한다.
			if (const UGameplayAbility* Ability = EffectContext.GetAbilityInstance_NotReplicated())
			{
				FGameplayAbilitySpec* AbilitySpec = SourceASC->FindAbilitySpecFromHandle(Ability->GetCurrentAbilitySpecHandle());
				Ability->ApplyAbilityTagsToGameplayEffectSpec(*NewHandle.Data, AbilitySpec);
				
				// Copy over set by caller magnitudes
				if (AbilitySpec)
				{
					NewHandle.Data->SetByCallerTagMagnitudes = AbilitySpec->SetByCallerTagMagnitudes;
				}
			}
		}
		return NewHandle;
	}
}


bool UURCombatBlueprintLibrary::Hit(
	UAbilitySystemComponent* SourceASC, 
	AActor* TargetActor,
	const TArray<FURHitEffect>& HitEffects, 
	const FGameplayEffectContextHandle& EffectContext)
{
	if (!IsValid(SourceASC) || 
		!IsValid(SourceASC->GetOwnerActor()) || 
		!SourceASC->GetOwnerActor()->HasAuthority() || 
		!UURGameBlueprintLibrary::IsDamageableActor(TargetActor))
	{
		return false;
	}

	// Note: 유효한 대상일 경우 항상 명중한다. 명중 규칙이 생긴다면 이 지점에서 판정한다.
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	if (!TargetASC)
	{
		// Non ASC Damageable
		// HitEffect에 대미지 GE있을 경우 NativeTakeDamage 호출
		for (const FURHitEffect& HitEffect : HitEffects)
		{
			if (HitEffect.EffectClass && 
				HitEffect.EffectClass->IsChildOf(UGameplayEffect_Damage::StaticClass()))
			{
				if (!IsValid(SourceASC) ||
					!IsValid(TargetActor) ||
					!IURDamageableInterface::Execute_CanTakeDamage(TargetActor))
				{
					// 이펙트 적용 중 상태 변경 가능성 있음
					break;
				}
				
				float DamageMultiplier = 1.0f;
				for (const TPair<FGameplayTag, float>& Value : HitEffect.SetByCallerValues)
				{
					// 대미지 계수 설정
					if (Value.Key.MatchesTagExact(URGameplayTags::SetByCaller_DamageMultiplier))
					{
						DamageMultiplier = Value.Value;
					}
				}
				
				bool bIsCritical = false;
				const float Damage = UURDamageExecutionCalculation::CalculateAttackPower(
					SourceASC->GetNumericAttribute(UURCombatSet::GetAttackPowerAttribute()),
					SourceASC->GetNumericAttribute(UURCombatSet::GetAdditionalAttackPowerAttribute()),
					SourceASC->GetNumericAttribute(UURCombatSet::GetCriticalRateAttribute()),
					SourceASC->GetNumericAttribute(UURCombatSet::GetCriticalDamageAttribute()),
					bIsCritical);
				IURDamageableInterface::Execute_NativeTakeDamage(TargetActor, Damage * DamageMultiplier, bIsCritical);
			}
		}
		
		// Non ASC 대상에게 사용할 경우 Damage가 없어도 true
		return true;
	}
	
	// URHitGameplayEffectContext에 개별 값 설정하기 위해 복제
	const FGameplayEffectContextHandle HitContext = EffectContext.Duplicate();
	
	for (const FURHitEffect& HitEffect : HitEffects)
	{
		if (!HitEffect.EffectClass)
		{
			continue;
		}
		
		if (!IsValid(SourceASC) || 
			!IsValid(TargetActor) || 
			!IsValid(TargetASC) || 
			!IURDamageableInterface::Execute_CanTakeDamage(TargetActor))
		{
			// 이펙트 적용 중 상태 변경 가능성 있음
			break;
		}
		
		FGameplayEffectSpecHandle SpecHandle = MakeHitSpec(SourceASC, HitEffect.EffectClass, HitContext);
		if (!SpecHandle.IsValid())
		{
			continue;
		}
		
		// SetByCaller 값 설정
		for (const TPair<FGameplayTag, float>& Value : HitEffect.SetByCallerValues)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Value.Key, Value.Value);
		}
		
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
	}
	
	return true;
}
