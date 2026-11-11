// Written by EXPx2


#include "Character/URAggroComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "URGame.h"
#include "URHealthComponent.h"
#include "AbilitySystem/AttributeSet/URCombatSet.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"

// Sets default values for this component's properties
UURAggroComponent::UURAggroComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UURAggroComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (UURHealthComponent* HealthComponent = GetOwner()->FindComponentByClass<UURHealthComponent>())
	{
		HealthComponent->OnDamageTaken.AddUniqueDynamic(this, &ThisClass::HandleDamageTaken);
	}
}

void UURAggroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UURHealthComponent* HealthComponent = GetOwner()->FindComponentByClass<UURHealthComponent>())
	{
		HealthComponent->OnDamageTaken.RemoveDynamic(this, &ThisClass::HandleDamageTaken);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UURAggroComponent::Clear()
{
	AggroMap.Reset();
}

void UURAggroComponent::CleanInvalidPair()
{
	for (auto Iterator = AggroMap.CreateIterator(); Iterator; ++Iterator)
	{
		const TWeakObjectPtr<AActor>& TargetActor = Iterator.Key();
		const float AggroValue = Iterator.Value();
		
		if (AggroValue <= SMALL_NUMBER ||
			!UURGameBlueprintLibrary::IsDamageableActor(TargetActor.Get()) ||
			FVector::DistSquared2D(TargetActor->GetActorLocation(), GetOwner()->GetActorLocation()) > FMath::Square(MaxDistance))
		{
			Iterator.RemoveCurrent();
			continue;
		}
	}
}

AActor* UURAggroComponent::GetTargetActor()
{
	CleanInvalidPair();
	
	AActor* TargetActor = nullptr;
	float Highest = 0.f;

	for (const auto& Pair : AggroMap)
	{
		AActor* Actor = Pair.Key.Get();
		const float Aggro = Pair.Value;
		
		if (TargetActor == nullptr || Aggro > Highest)
		{
			Highest = Aggro;
			TargetActor = Actor;
		}
	}

	return TargetActor;
}

AActor* UURAggroComponent::GetRandomTargetActor(const bool bIncludeMost)
{
	CleanInvalidPair();
	
	AActor* MostTargetActor = bIncludeMost ? nullptr : GetTargetActor();

	TArray<AActor*> Candidates;
	Candidates.Reserve(AggroMap.Num());

	for (const auto& Pair : AggroMap)
	{
		AActor* Actor = Pair.Key.Get();
		
		if (!bIncludeMost && Actor == MostTargetActor)
		{
			continue;
		}

		Candidates.Add(Actor);
	}

	if (Candidates.IsEmpty())
	{
		return nullptr;
	}

	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

void UURAggroComponent::AddAggro(AActor* Instigator, const float AggroValue)
{
	if (!Instigator ||
		!UURGameBlueprintLibrary::IsDamageableActor(Instigator))
	{
		return;
	}
	
	AggroMap.FindOrAdd(Instigator) += AggroValue;
	
	// LogAggroMap();
}

void UURAggroComponent::ResetAggro()
{
	AggroMap.Reset();
}

void UURAggroComponent::HandleDamageTaken(AActor* Instigator, AActor* Causer, float Magnitude)
{
	// 대미지 피격 시 처리
	if (!Instigator || !Causer)
	{
		// 서버에서 정상 경로의 DamageTaken은 항상 Instigator가 온다. 
		return;
	}
	
	UR_LOG(LogUR, Log, TEXT("DamageTaken from %s | %s"), *Instigator->GetName(), *Causer->GetName());
	float Addition = FMath::Max(Magnitude, 1.f);
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	if (ASC)
	{
		// 플레이어의 Instigator는 Pawn 대신 PlayerState일 수 있다.
		// Causer로 하면 HitTraceActor같이 생성된 액터에서 ASC가 아닌 HitTraceActor가 저장됨.
		Instigator = ASC->GetAvatarActor();
		Addition *= ASC->GetSet<UURCombatSet>()->GetAggroMultiplier();
	}
	
	// OwnerActor가 아닌 
	AddAggro(Instigator, Addition);

	// LogAggroMap();
}

void UURAggroComponent::LogAggroMap()
{
	for (auto& [Actor, Aggro] : AggroMap)
	{
		UR_LOG(LogUR, Log, TEXT("%s Aggro | %s: %f"), *GetName(), *Actor->GetName(), Aggro)
	}
}
