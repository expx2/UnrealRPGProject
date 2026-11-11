// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "URHitGameplayEffectContext.generated.h"

/**
 * 
 */
USTRUCT()
struct URGAME_API FURHitGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
	
public:
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}
	
	virtual FURHitGameplayEffectContext* Duplicate() const override
	{
		FURHitGameplayEffectContext* NewContext = new FURHitGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

		Ar.SerializeBits(&bIsCritical, 1);
		HitImpactCueTag.NetSerialize(Ar, Map, bOutSuccess);
		bOutSuccess = !Ar.IsError();

		return true;
	}
	
	
	UPROPERTY()
	bool bIsCritical = false;

	UPROPERTY()
	FGameplayTag HitImpactCueTag;
};

template<>
struct TStructOpsTypeTraits<FURHitGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FURHitGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
