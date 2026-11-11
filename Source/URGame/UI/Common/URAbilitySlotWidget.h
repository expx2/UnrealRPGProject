// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "URAbilitySlotWidget.generated.h"

class UURGameplayAbility_ActiveAbility;
/**
 * 
 */
UCLASS()
class URGAME_API UURAbilitySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	virtual void SetAbility(UURGameplayAbility_ActiveAbility* Ability);
	
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UImage> ImgIcon;
};
