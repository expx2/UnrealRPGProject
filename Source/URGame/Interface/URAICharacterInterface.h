// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "URAICharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UURAICharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class URGAME_API IURAICharacterInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|AI")
	float GetPatrolRadius() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|AI")
	bool GetIsAggressive() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|AI")
	float GetDetectRange() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|AI")
	float GetActiveRange() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|AI")
	class UAIAbilityOrderSet* GetAbilityOrderSet() const;
	
	virtual float GetPatrolRadius_Implementation() const;
	virtual bool GetIsAggressive_Implementation() const;
	virtual float GetDetectRange_Implementation() const;
	virtual float GetActiveRange_Implementation() const;
	virtual class UAIAbilityOrderSet* GetAbilityOrderSet_Implementation() const;
};
