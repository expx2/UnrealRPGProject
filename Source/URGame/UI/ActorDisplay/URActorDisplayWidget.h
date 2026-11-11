// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "URActorDisplayWidget.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURActorDisplayWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetName(const FName InName) const;
	
	void SetNameColor(const FColor InColor) const;
	
	void SetHealth(const float Health, const float MaxHealth) const;
	
	void SetIsTargeted(const bool bInValue);
	
	void SetIsHit(const bool bInValue);
	
	void SetIsPlayer(const bool bInValue);

	bool IsHighlighted() const { return bIsHit || bIsTargeted;}
	
protected:
	virtual void NativeConstruct() override;
	
	void UpdateBackplateVisible() const;
	
	void UpdateHealthVisible() const;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TxtName;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UProgressBar> PBHealth;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> ImgBackplate;
	
	bool bIsTargeted = false;
	bool bIsHit = false;
	bool bIsPlayer = false;
};
