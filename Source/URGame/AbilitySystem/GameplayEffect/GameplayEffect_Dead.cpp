// Written by EXPx2


#include "AbilitySystem/GameplayEffect/GameplayEffect_Dead.h"
#include "GameplayEffectComponents/CancelAbilityTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayTags/URGameplayTags.h"
#include "GameplayTags/URGameplayTags_Ability.h"

UGameplayEffect_Dead::UGameplayEffect_Dead()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	UTargetTagsGameplayEffectComponent* GrantedTagsComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("GrantedTags"));
	GEComponents.Add(GrantedTagsComponent);

	FInheritedTagContainer GrantedTags;
	GrantedTags.AddTag(URGameplayTags::Character_State_CantTakeDamage);
	GrantedTags.AddTag(URGameplayTags::Character_State_Dead);
	GrantedTags.AddTag(URGameplayTags::Character_State_Untargetable);
	GrantedTagsComponent->SetAndApplyTargetTagChanges(GrantedTags);

	// The component defaults to OnApplication.
	UCancelAbilityTagsGameplayEffectComponent* CancelAbilitiesComponent = CreateDefaultSubobject<UCancelAbilityTagsGameplayEffectComponent>(TEXT("CancelAbilities"));
	GEComponents.Add(CancelAbilitiesComponent);

	FInheritedTagContainer CancelAbilitiesWithTags;
	CancelAbilitiesWithTags.AddTag(URGameplayTags::Ability);
	CancelAbilitiesComponent->SetAndApplyCanceledAbilityTagChanges(CancelAbilitiesWithTags, FInheritedTagContainer());
}

