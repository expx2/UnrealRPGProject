// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "URDamageableInterface.generated.h"

enum class EURFaction : uint8;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UURDamageableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class URGAME_API IURDamageableInterface
{
	GENERATED_BODY()

public:
	/** 진영 리턴 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="UR")
	EURFaction GetFaction() const;
	
	/** 타겟 대상으로 지정될 수 있는지 여부 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="UR")
	bool CanTakeDamage() const;
	
	/** ASC가 없는 액터의 대미지 받기 처리 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="UR")
	void NativeTakeDamage(const float Damage, const bool bIsCritical);
	
	virtual EURFaction GetFaction_Implementation() const;
	
	virtual void NativeTakeDamage_Implementation(const float Damage, const bool bIsCritical);
	
private:
	bool CanTakeDamage_Implementation() const;
};
