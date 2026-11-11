// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayAbilityTargetData_Time.generated.h"

/** Target data with a time */
USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_Time : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()
	
	FGameplayAbilityTargetData_Time() { }
	FGameplayAbilityTargetData_Time(double InTime)
		: Time(InTime)
	{ }
	
	UPROPERTY()
	double Time = 0.0;
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_Time::StaticStruct();
	}
	
	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FGameplayAbilityTargetData_Time %.2f"), Time);
	}

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << Time;
		bOutSuccess = !Ar.IsError();
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_Time> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_Time>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};
