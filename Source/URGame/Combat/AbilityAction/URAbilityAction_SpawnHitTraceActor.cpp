// Written by EXPx2


#include "URAbilityAction_SpawnHitTraceActor.h"

#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "Combat/HitTraceActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

bool FURAbilityAction_SpawnHitTraceActor::HasValidData() const
{
	return HitTraceActorClass != nullptr;
}

FURAbilityActionStartResult FURAbilityAction_SpawnHitTraceActor::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext,
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	AActor* OwnerActor = Ability->GetOwningActorFromActorInfo();
	if (!IsValid(OwnerActor))
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	// 서버에서만 생성하고 클라이언트에는 액터 복제로 전달한다.
	if (!OwnerActor->HasAuthority())
	{
		return FURAbilityActionStartResult::Completed();
	}
	
	AActor* SourceActor = Ability->GetAvatarActorFromActorInfo();
	
	FTransform SpawnTransform;
	USkeletalMeshComponent* MeshComp = Ability->GetOwningComponentFromActorInfo();
	
	if (bAttach)
	{
		SpawnTransform = IsValid(MeshComp)
			                 ? MeshComp->GetSocketTransform(SocketName)
			                 : SourceActor->GetActorTransform();
	}
	else
	{
		const FRotator SourceYawRotation(0.0f, SourceActor->GetActorRotation().Yaw, 0.0f);
		SpawnTransform = FTransform(SourceYawRotation, InContext.Location);
	}
	
	SpawnTransform.SetLocation(SpawnTransform.GetLocation() + LocationOffset);
	
	AHitTraceActor* HitTraceActor = SourceActor->GetWorld()->SpawnActorDeferred<AHitTraceActor>(
		HitTraceActorClass,
		SpawnTransform,
		OwnerActor,
		Cast<APawn>(SourceActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	
	if (!IsValid(HitTraceActor))
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	if (bAttach)
	{
		bool bIsAttachedSuccessfully = false;
		if (IsValid(MeshComp))
		{
			bIsAttachedSuccessfully = HitTraceActor->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepWorldTransform, SocketName);
			if (bIsAttachedSuccessfully)
			{
				HitTraceActor->AddTickPrerequisiteComponent(MeshComp);
			}
		}
		else
		{
			bIsAttachedSuccessfully = HitTraceActor->AttachToActor(SourceActor, FAttachmentTransformRules::KeepWorldTransform, SocketName);
		}
		
		if (!bIsAttachedSuccessfully)
		{
			HitTraceActor->Destroy();
			return FURAbilityActionStartResult::Failed();
		}
	}
	HitTraceActor->InitHitTraceActor(
		Ability->GetAbilitySystemComponentFromActorInfo(),
		InContext.TargetActorPtr.Get(),
		Ability->GetTargetFactions(),
		// Ability->GetAbilityLevel(),
		HitEffects,
		HitImpactTag,
		GetActionElapsedTime(TimelineElapsed));
	
	if (!IsValid(HitTraceActor))
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	HitTraceActor->FinishSpawning(SpawnTransform);

	return FURAbilityActionStartResult::Completed();
}
