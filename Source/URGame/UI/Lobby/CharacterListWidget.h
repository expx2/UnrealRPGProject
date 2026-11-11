// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterListWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterListClickedDelegate, UObject*, Item);

/**
 * 
 */
UCLASS()
class URGAME_API UCharacterListWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category="UR|UI")
	void SetListView(class UPlayerCharactersSet* PlayerCharactersSet);
	
	UPROPERTY(BlueprintAssignable, Category="UR|UI")
	FCharacterListClickedDelegate OnClicked;
	
protected:
	void HandleItemClicked(UObject* Item);
	
	UFUNCTION(BlueprintImplementableEvent, Category="UR|UI")
	void K2_HandleItemClicked(UObject* Item);
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UListView> ListView;
};
