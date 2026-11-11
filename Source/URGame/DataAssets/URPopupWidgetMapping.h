// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "URPopupWidgetMapping.generated.h"

enum class EPopupType : uint8;

/**
 * 
 */
UCLASS(BlueprintType, Category="UR|UI")
class URGAME_API UURPopupWidgetMapping : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UClass* GetWidgetClass(EPopupType Type);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|UI")
	TMap<EPopupType, TSubclassOf<class UURPopupWidget>> BindingMap;
};
