// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"

#if ENABLE_DRAW_DEBUG

/** Draws an extruded sector. InnerRadius > 0 draws a donut sector. Segments is the subdivision count for 360 degrees. */
URGAME_API void DrawDebugSector(const UWorld* InWorld,
	const FVector& Location,
	const FVector& Direction,
	float Radius,
	float InnerRadius,
	float AngleDegrees,
	float HalfHeight,
	int32 Segments,
	const FColor& Color,
	bool bPersistentLines = false,
	float LifeTime = -1.f,
	uint8 DepthPriority = 0,
	float Thickness = 0.f);

#elif !defined(SHIPPING_DRAW_DEBUG_ERROR) || !SHIPPING_DRAW_DEBUG_ERROR

FORCEINLINE void DrawDebugSector(const UWorld* InWorld,
	const FVector& Location,
	const FVector& Direction,
	float Radius,
	float InnerRadius,
	float AngleDegrees,
	float HalfHeight,
	int32 Segments,
	const FColor& Color,
	bool bPersistentLines = false,
	float LifeTime = -1.f,
	uint8 DepthPriority = 0,
	float Thickness = 0.f)
{
}

#endif
