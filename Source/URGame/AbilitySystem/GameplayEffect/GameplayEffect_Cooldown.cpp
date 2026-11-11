// Written by EXPx2


#include "AbilitySystem/GameplayEffect/GameplayEffect_Cooldown.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayTags/URGameplayTags.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(Dummy_Cooldown, "Dummy.Cooldown");

UGameplayEffect_Cooldown::UGameplayEffect_Cooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat CooldownDuration;
	CooldownDuration.DataTag = URGameplayTags::SetByCaller_Duration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(CooldownDuration);

	// Note: DefaultEngine.ini 설정으로 해결함
	// // 어빌리티에서 쿨다운 GE의 GrantedTags가 비어 있으면 컴파일이 안 됨. @see UGameplayAbility::IsDataValid
	// // 통과를 위한 Dummy 태그. GE 적용 시 이 태그는 무시한다.
	// UTargetTagsGameplayEffectComponent* GrantedTagsComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("GrantedTags"));
	// GEComponents.Add(GrantedTagsComponent);
	//
	// FInheritedTagContainer GrantedTags;
	// GrantedTags.Added.AddTag(Dummy_Cooldown);
	// GrantedTagsComponent->SetAndApplyTargetTagChanges(GrantedTags);
}
