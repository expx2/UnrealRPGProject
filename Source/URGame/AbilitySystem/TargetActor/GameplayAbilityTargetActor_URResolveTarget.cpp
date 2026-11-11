// Written by EXPx2


#include "AbilitySystem/TargetActor/GameplayAbilityTargetActor_URResolveTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Character/URTargetingComponent.h"


AGameplayAbilityTargetActor_URResolveTarget::AGameplayAbilityTargetActor_URResolveTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	ShouldProduceTargetDataOnServer = false;
}

void AGameplayAbilityTargetActor_URResolveTarget::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
}

void AGameplayAbilityTargetActor_URResolveTarget::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData();
		TargetDataReadyDelegate.Broadcast(Handle);
	}
}

FGameplayAbilityTargetDataHandle AGameplayAbilityTargetActor_URResolveTarget::MakeTargetData() const
{
	TArray<AActor*> TargetActors;
	if (const APlayerController* PC = PrimaryPC)
	{
		if (const UURTargetingComponent* TargetingComponent = PC->FindComponentByClass<UURTargetingComponent>())
		{
			
			if (AActor* MainTargetActor = TargetingComponent->GetMainTargetActor())
			{
				TargetActors.Add(MainTargetActor);
			}
				
			if (AActor* AimTargetActor = TargetingComponent->GetAimTargetActor())
			{
				TargetActors.Add(AimTargetActor);
			}
		}
	}
	
	return UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors, false);
}
