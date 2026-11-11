// Written by EXPx2


#include "Character/URCombatComponent.h"

#include "URCharacterMovementComponent.h"
#include "URHealthComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayTags/URGameplayTags.h"
#include "Net/UnrealNetwork.h"

UURCombatComponent::UURCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);
}

void UURCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent = GetOwner()->FindComponentByClass<UURHealthComponent>();
	if (HealthComponent.IsValid())
	{
		HealthComponent->OnDamageTaken.AddUniqueDynamic(this, &ThisClass::HandleDamageTaken);
	}
	
	CharacterMovement = GetOwner()->FindComponentByClass<UURCharacterMovementComponent>();
}

void UURCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	if (HealthComponent.IsValid())
	{
		HealthComponent->OnDamageTaken.RemoveDynamic(this, &ThisClass::HandleDamageTaken);
	}
	HealthComponent.Reset();
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CombatTargetActor.Reset();
	}
	UpdateRotation();

	// CharacterMovement의 RotationResolver에 해제해야 하니 반드시 UpdateRotation 이후 호출
	CharacterMovement.Reset();
	
	Super::EndPlay(EndPlayReason);
}

void UURCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UURCombatComponent, CombatTargetActor);
}

void UURCombatComponent::ReceiveAbilitySystemComponent_Implementation(UAbilitySystemComponent* AbilitySystemComponent)
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	
	AbilityComponent = AbilitySystemComponent;
	
	InCombatTagChangedDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(URGameplayTags::Character_State_InCombat, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::HandleInCombatTagChanged);

	UpdateRotation();
}

void UURCombatComponent::ClearAbilitySystemComponent_Implementation()
{
	if (AbilityComponent.IsValid() && InCombatTagChangedDelegateHandle.IsValid())
	{
		AbilityComponent->UnregisterGameplayTagEvent(InCombatTagChangedDelegateHandle, URGameplayTags::Character_State_InCombat, EGameplayTagEventType::NewOrRemoved);
	}
	InCombatTagChangedDelegateHandle.Reset();
	
	AbilityComponent.Reset();
}

void UURCombatComponent::SetCombatTargetActor(AActor* Actor)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Actor = GetValid(Actor);
		CombatTargetActor = Actor;
		UpdateRotation();
	}
}

bool UURCombatComponent::GetIsInCombat() const
{
	if (AbilityComponent.IsValid())
	{
		return AbilityComponent->HasMatchingGameplayTag(URGameplayTags::Character_State_InCombat);
	}

	return false;
}

void UURCombatComponent::Clear()
{
	CombatTargetActor.Reset();
	UpdateRotation();
}

void UURCombatComponent::OnRep_CombatTarget()
{
	UpdateRotation();
}

void UURCombatComponent::UpdateRotation() const
{
	if (CharacterMovement.IsValid())
	{
		const bool bLookTarget = GetIsInCombat() && CombatTargetActor.IsValid();
		if (bLookTarget)
		{
			const FURRotationContext Context(CombatTargetActor.Get());
			CharacterMovement->RotationResolver.Set(EURRotationSource::Combat, Context);
		}
		else
		{
			CharacterMovement->RotationResolver.CancelSource(EURRotationSource::Combat);
		}
	}
}

void UURCombatComponent::HandleInCombatTagChanged(FGameplayTag GameplayTag, int Count)
{
	UpdateRotation();

	if (!GetIsInCombat() && 
		GetOwner() && 
		GetOwner()->HasAuthority())
	{
		// 전투 모드 종료
		SetCombatTargetActor(nullptr);
	}
}

void UURCombatComponent::HandleDamageTaken(AActor* Instigator, AActor* Causer, float Magnitude)
{
	// if (GetOwner()->HasAuthority())
	// {
	// 	// 전투 타겟 없을 경우 설정
	// 	if (!CombatTargetActor.IsValid())
	// 	{
	// 		// 플레이어의 GAS Instigator는 Pawn 대신 PlayerState일 수 있다.
	// 		if (const UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator))
	// 		{
	// 			Instigator = InstigatorASC->GetAvatarActor();
	// 		}
	// 		// Causer로 하면 HitTraceActor같이 생성된 액터에서 ASC가 아닌 HitTraceActor가 저장됨.
	// 		SetCombatTargetActor(Instigator);
	// 	}
	// }
}

