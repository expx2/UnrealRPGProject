// Written by EXPx2


#include "Combat/HitTraceActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "AbilitySystem/GameplayEffect/URHitGameplayEffectContext.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Character/URHealthComponent.h"
#include "BlueprintLibrary/URCombatBlueprintLibrary.h"
#include "BlueprintLibrary/URCollisionBlueprintLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

AHitTraceActor::AHitTraceActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
	SetReplicateMovement(true);
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void AHitTraceActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (IsActorBeingDestroyed() ||
		TraceCount >= HitCount || 
		!AbilityComponent.IsValid())
	{
		SetActorTickEnabled(false);
		return;
	}
	
	// 프레임 지연으로 한 프레임에 여러번 가능
	const double CurrentSeconds = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	while (!IsActorBeingDestroyed() && 
		TraceCount < HitCount && 
		CurrentSeconds >= NextActionTime &&
		AbilityComponent.IsValid())
	{
		Trace();
	}
}

void AHitTraceActor::InitHitTraceActor(
	UAbilitySystemComponent* InASC,
	AActor* InTargetActor,
	const EURFaction InTargetFactions,
	const TArray<FURHitEffect>& InHitEffects,
	const FGameplayTag InHitImpactTag,
	const float ElapsedTime)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(InASC) || 
		!Shape.IsValid() || 
		MaxTargetCount < 1 || 
		HitCount < 1)
	{
		Destroy();
		return;
	}

	AbilityComponent = InASC;
	TargetActor = InTargetActor;
	TargetFactions = InTargetFactions;
	HitEffects = InHitEffects;
	HitImpactTag = InHitImpactTag;
	SetOwner(InASC->GetOwnerActor());

	TraceCount = 0;
	LastHitActors.Empty(MaxTargetCount);

	const double ActivationWorldSeconds = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() - FMath::Max(ElapsedTime, 0.0f);
	NextActionTime = ActivationWorldSeconds + PreDelay;
	
	if (AActor* AvatarActor = InASC->GetAvatarActor())
	{
		if (UURHealthComponent* HealthComponent = AvatarActor->FindComponentByClass<UURHealthComponent>())
		{
			HealthComponent->OnDeath.AddUniqueDynamic(this, &ThisClass::HandleAvatarActorDeath);
		}
	}
}


void AHitTraceActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetLifeSpan(LifeTime);
	SetActorTickEnabled(HasAuthority());
}

void AHitTraceActor::Trace()
{
	// @see UAbilityActionTask_MeleeTrace::Trace
	if (!HasAuthority())
	{
		return;
	}
	
	if (!AbilityComponent.IsValid())
	{
		TraceCount = HitCount;
		SetActorTickEnabled(false);
		return;
	}

	TArray<AActor*> TargetableActors;
	UURCollisionBlueprintLibrary::GetDamageableActors(
		this,
		GetActorLocation(),
		GetActorForwardVector().GetSafeNormal2D(),
		Shape,
		UURCombatBlueprintLibrary::GetRelativeTargetFaction(TargetFactions, AbilityComponent->GetAvatarActor()),
		AbilityComponent->GetAvatarActor(),
		TargetableActors);

	// SourceActor는 GetDamageableActors()결과에서 제외되어 오고, OwnerActor 추가로 제외한다.
	TargetableActors.Remove(AbilityComponent->GetOwnerActor());

	if (TargetableActors.Num() <= MaxTargetCount)
	{
		Hit(TargetableActors);
	}
	else
	{
		/*
		 * Note: 범위 내 대상 수가 MaxTargetCount보다 많을 경우 우선 순위를 통해 최대 대상 수 만큼만 지정한다.
		 * 1. 어빌리티 타겟: 스킬 사용 시 지정된 액터로 항상 최우선 순위로 삼는다.
		 * 2. 이전 히트에서 처리한 대상: 다회 공격할 때 이전 회차에서 공격한 대상을 대상으로 유지한다.
		 * 3. 그 외
		 */
		TArray<AActor*> HitActors;
		TArray<AActor*> PriorTargets = URUtilsLibrary::ToRawArray<AActor>(LastHitActors);
			
		UURCombatBlueprintLibrary::ResolveTargets(
			TargetableActors, 
			MaxTargetCount,
			TargetActor.Get(),
			PriorTargets,
			HitActors);
			
		// 타겟 설정 완료
		Hit(HitActors);
	}

	TraceCount++;

	if (TraceCount >= HitCount)
	{
		// 타격이 끝나도 LifeTime까지 액터를 유지한다.
		SetActorTickEnabled(false);
		return;
	}

	NextActionTime += HitInterval;
}

void AHitTraceActor::Hit(TArray<AActor*> TargetActors)
{
	LastHitActors.Empty();
	
	UAbilitySystemComponent* ASC = AbilityComponent.Get();
	if (!ensure(ASC))
	{
		return;
	}
	
	UURAbilitySystemComponent* URASC = Cast<UURAbilitySystemComponent>(ASC);
	if (URASC && URASC->IsDead())
	{
		return;
	}
	
	FGameplayEffectContextHandle EffectContext = URASC
		                                             ? URASC->MakeDamageEffectContext()
		                                             : ASC->MakeEffectContext();
	if (EffectContext.Get() &&
		EffectContext.Get()->GetScriptStruct()->IsChildOf(FURHitGameplayEffectContext::StaticStruct()))
	{
		if (FURHitGameplayEffectContext* DamageContext = static_cast<FURHitGameplayEffectContext*>(EffectContext.Get()))
		{
			DamageContext->HitImpactCueTag = HitImpactTag;
		}
	}
	
	// EffectCauser를 AvatarActor에서 HitTraceActor로 변경
	EffectContext.AddInstigator(AbilityComponent->GetOwnerActor(), this);
	
	int SetCount = 0;
	for (AActor* Actor : TargetActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}
		
		if (UURCombatBlueprintLibrary::Hit(ASC, Actor, HitEffects, EffectContext))
		{
			LastHitActors.Add(Actor);
		}

		SetCount++;
		if (SetCount >= MaxTargetCount)
		{
			// TargetActors로 제한된 수의 액터가 넘어오는게 정상 경로긴 함.
			return;
		}
	}
}

void AHitTraceActor::HandleAvatarActorDeath(AActor* Actor)
{
	TraceCount = HitCount;
	SetActorTickEnabled(false);
	
	if (UURHealthComponent* HealthComponent = Actor->FindComponentByClass<UURHealthComponent>())
	{
		HealthComponent->OnDeath.RemoveDynamic(this, &ThisClass::HandleAvatarActorDeath);
	}
}

