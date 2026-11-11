// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Definitions/URPopupType.h"
#include "URPopupWidget.generated.h"

enum class EPopupType : uint8;
/**
 * 
 */
UCLASS()
class URGAME_API UURPopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UURPopupWidget(const FObjectInitializer& ObjectInitializer);

	void Close();
	
protected:
	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;
};
