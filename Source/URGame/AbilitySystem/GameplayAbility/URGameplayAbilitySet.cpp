// Written by EXPx2


#include "AbilitySystem/GameplayAbility/URGameplayAbilitySet.h"

#include "AbilitySystemComponent.h"
#include "URGame.h"

UURGameplayAbilitySet::UURGameplayAbilitySet()
{
}

void UURGameplayAbilitySet::GiveAbilities(
	UAbilitySystemComponent* AbilitySystemComponent,
	TArray<FGameplayAbilitySpecHandle>& OutGrantedSpecHandles)
{
	if (!AbilitySystemComponent || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}
	
	for (const FAbilityBindInfo& BindInfo : Abilities)
	{
		if (BindInfo.GameplayAbilityClass)
		{
			FGameplayAbilitySpec Spec(BindInfo.GameplayAbilityClass, 1, BindInfo.Command);
			OutGrantedSpecHandles.Add(Spec.Handle);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}