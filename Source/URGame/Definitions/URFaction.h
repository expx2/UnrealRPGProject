// Written by EXPx2

#pragma once

/** 진영 */
UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EURFaction : uint8
{
	None = 0 UMETA(Hidden),
	Neutral = 1 << 0,
	Ally = 1 << 1,
	Enemy = 1 << 2,
};
ENUM_CLASS_FLAGS(EURFaction)