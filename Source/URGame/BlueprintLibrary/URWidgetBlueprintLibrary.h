// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "URWidgetBlueprintLibrary.generated.h"

enum class EPopupType : uint8;
/**
 * 
 */
UCLASS()
class URGAME_API UURWidgetBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	static class UURPopupWidget* OpenPopup(APlayerController* PlayerController, EPopupType PopupType);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	static void ClosePopup(APlayerController* PlayerController, EPopupType PopupType);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	static void ClosePopupAll(APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	static void DeriveActorToChild(class UUserWidget* UserWidget, AActor* Actor);
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	static void DeriveAbilitySystemComponentToChild(class UUserWidget* UserWidget, class UAbilitySystemComponent* AbilitySystemComponent);
};
