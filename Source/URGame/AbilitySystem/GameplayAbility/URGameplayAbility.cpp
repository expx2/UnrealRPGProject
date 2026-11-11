// Written by EXPx2


#include "AbilitySystem/GameplayAbility/URGameplayAbility.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "GameplayTags/URGameplayTags.h"

UURGameplayAbility::UURGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UURGameplayAbility::DoesAbilitySatisfyTagRequirements(
	const UAbilitySystemComponent& AbilitySystemComponent,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	// LyraGameplayAbility 카피함
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}
	
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;
	
	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;
	
	// Expand our ability tags to add additional required/blocked tags
	GetActivationTagRequirements(AbilitySystemComponent, AllRequiredTags, AllBlockedTags);

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;
		
		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(URGameplayTags::Character_State_Dead))
			{
				// If player is dead and was rejected due to blocking tags, give that feedback
				OptionalRelevantTags->AddTag(URGameplayTags::TAG_Ability_Fail_IsDead);
			}

			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

void UURGameplayAbility::GetActivationTagRequirements(
	const UAbilitySystemComponent& AbilitySystemComponent,
	FGameplayTagContainer& OutRequiredTags, 
	FGameplayTagContainer& OutBlockedTags) const
{
	if (const UURAbilitySystemComponent* URASC = Cast<UURAbilitySystemComponent>(&AbilitySystemComponent))
	{
		URASC->GetAdditionalActivationTagRequirements(GetAssetTags(), OutRequiredTags, OutBlockedTags);
	}
}

void UURGameplayAbility::K2_EndAbilityAsCancelled()
{
	ensure(CurrentActorInfo);

	bool bReplicateEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}
