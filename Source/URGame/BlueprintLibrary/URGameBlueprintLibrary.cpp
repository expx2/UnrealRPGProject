// Written by EXPx2


#include "BlueprintLibrary/URGameBlueprintLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "Interface/URDamageableInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "UI/HUD/URGameHUD.h"

namespace
{
	const FName AimSocketName(TEXT("Aim_Target"));
}

FVector UURGameBlueprintLibrary::GetFeetLocation(const AActor* Actor)
{
	if (Actor)
	{
		FVector ActorLocation = Actor->GetActorLocation();
		const float HalfHeight = Actor->GetSimpleCollisionHalfHeight();
		ActorLocation.Z -= HalfHeight;
		return ActorLocation;
	}
	return FVector::ZeroVector;
}

void UURGameBlueprintLibrary::GetAimTargetLocation(const AActor* Actor, FVector& ResultLocation)
{
	TInlineComponentArray<UMeshComponent*> MeshComponents;
	Actor->GetComponents(MeshComponents);
	for (const UMeshComponent* Mesh : MeshComponents)
	{
		if (IsValid(Mesh) && Mesh->DoesSocketExist(AimSocketName))
		{
			ResultLocation = Mesh->GetSocketLocation(AimSocketName);
			return;
		}
	}
	
	ResultLocation = Actor->GetActorLocation();
}

UPathFollowingComponent* UURGameBlueprintLibrary::GetPathFollowingComponent(AController* Controller, const bool bCreateIfDontHave)
{
	// AIBlueprintHelperLibrary::InitNavigationControl
	AAIController* AsAIController = Cast<AAIController>(Controller);
	UPathFollowingComponent* PathFollowingComp = nullptr;

	if (AsAIController)
	{
		PathFollowingComp = AsAIController->GetPathFollowingComponent();
	}
	else
	{
		PathFollowingComp = Controller->FindComponentByClass<UPathFollowingComponent>();
		if (bCreateIfDontHave && PathFollowingComp == nullptr)
		{
			PathFollowingComp = NewObject<UPathFollowingComponent>(Controller);
			PathFollowingComp->RegisterComponentWithWorld(Controller->GetWorld());
			PathFollowingComp->Initialize();
		}
	}

	return PathFollowingComp;
}

bool UURGameBlueprintLibrary::IsActorInRangeUsingPathFollowingComponent(const UPathFollowingComponent* PathFollowingComponent, const AActor* Actor, const float Range)
{
	/*
	 * 액터의 Radius를 거리에 포함시켜 범위안에 걸쳐도 InRange로 판단함
	 * HasReached 구현을 보면 INavAgentInterface일 경우에만 대상의 Radius를 포함 시켜 다른 액터에도 대응되도록 함.
	 */
	const float TargetRadius = Actor->GetSimpleCollisionRadius();
	
	return PathFollowingComponent->HasReached(*Actor, EPathFollowingReachMode::ExactLocation, Range + TargetRadius);
}

bool UURGameBlueprintLibrary::IsDamageableActor(const AActor* Actor)
{
	return IsValid(Actor) &&
		Actor->GetClass()->ImplementsInterface(UURDamageableInterface::StaticClass()) &&
		IURDamageableInterface::Execute_CanTakeDamage(Actor);
}

bool UURGameBlueprintLibrary::TriggerAbilityWithTarget(
	UAbilitySystemComponent* AbilitySystemComponent,
	FGameplayAbilitySpecHandle AbilityToTrigger,
	AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	FGameplayEventData EventData;
	EventData.Instigator = AbilitySystemComponent->GetOwnerActor();
	EventData.InstigatorTags = AbilitySystemComponent->GetOwnedGameplayTags();
	EventData.Target = TargetActor;
	EventData.TargetTags = TargetASC ? TargetASC->GetOwnedGameplayTags() : FGameplayTagContainer();
	
	return AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
		AbilityToTrigger,
		AbilitySystemComponent->AbilityActorInfo.Get(),
		FGameplayTag(),
		&EventData,
		*AbilitySystemComponent);
}