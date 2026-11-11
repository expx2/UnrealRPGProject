// Written by EXPx2


#include "UI/Common/URAbilitySlotWidget.h"

#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "Components/Image.h"

void UURAbilitySlotWidget::SetAbility(UURGameplayAbility_ActiveAbility* Ability)
{
	if (IsValid(Ability))
	{
		ImgIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		ImgIcon->SetBrushFromTexture(Ability->GetAbilityIcon());
	}
	else
	{
		ImgIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}
