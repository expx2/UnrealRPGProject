// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "URTargetableInterface.generated.h"

enum class EURFaction : uint8;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UURTargetableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class URGAME_API IURTargetableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** 이름 리턴 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="UR")
	FName GetDisplayName() const;
	
	/** 진영 리턴 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="UR")
	EURFaction GetFaction() const;
	
	/** 타겟 대상으로 지정될 수 있는지 여부 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="UR")
	bool CanBeTarget() const;
	
	virtual EURFaction GetFaction_Implementation() const;
	
private:
	virtual bool CanBeTarget_Implementation() const;
};
