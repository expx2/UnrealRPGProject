// Written by EXPx2

#include "URDrawDebugHelpers.h"

#if ENABLE_DRAW_DEBUG

void DrawDebugSector(const UWorld* InWorld,
	const FVector& Location,
	const FVector& Direction,
	const float Radius,
	const float InnerRadius,
	const float AngleDegrees,
	const float HalfHeight,
	const int32 Segments,
	const FColor& Color,
	const bool bPersistentLines,
	const float LifeTime,
	const uint8 DepthPriority,
	const float Thickness)
{
	if (!IsValid(InWorld))
	{
		return;
	}

	const float OuterRadius = FMath::Max(Radius, 0.f);
	const float ClampedInnerRadius = FMath::Clamp(InnerRadius, 0.f, OuterRadius);
	const float ClampedAngle = FMath::Clamp(AngleDegrees, 0.f, 360.f);
	const float ClampedHalfHeight = FMath::Max(HalfHeight, 0.f);
	const int32 FullCircleSegments = FMath::Max(Segments, 1);
	const int32 ArcSegments = FMath::Max(FMath::CeilToInt(FullCircleSegments * ClampedAngle / 360.f), 1);
	if (OuterRadius <= UE_SMALL_NUMBER || ClampedAngle <= UE_SMALL_NUMBER)
	{
		return;
	}

	FVector Forward = Direction.GetSafeNormal2D();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	const FVector HeightOffset = FVector::UpVector * ClampedHalfHeight;
	const float StartAngle = -ClampedAngle * 0.5f;
	FVector PreviousOuterBottom = FVector::ZeroVector;
	FVector PreviousOuterTop = FVector::ZeroVector;
	FVector PreviousInnerBottom = FVector::ZeroVector;
	FVector PreviousInnerTop = FVector::ZeroVector;

	for (int32 Index = 0; Index <= ArcSegments; ++Index)
	{
		const float Alpha = static_cast<float>(Index) / ArcSegments;
		const FVector ArcDirection = Forward.RotateAngleAxis(StartAngle + ClampedAngle * Alpha, FVector::UpVector);
		const FVector OuterCenter = Location + ArcDirection * OuterRadius;
		const FVector OuterBottom = OuterCenter - HeightOffset;
		const FVector OuterTop = OuterCenter + HeightOffset;

		DrawDebugLine(InWorld, OuterBottom, OuterTop, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);

		FVector InnerBottom = Location - HeightOffset;
		FVector InnerTop = Location + HeightOffset;
		if (ClampedInnerRadius > UE_SMALL_NUMBER)
		{
			const FVector InnerCenter = Location + ArcDirection * ClampedInnerRadius;
			InnerBottom = InnerCenter - HeightOffset;
			InnerTop = InnerCenter + HeightOffset;
			DrawDebugLine(InWorld, InnerBottom, InnerTop, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		}

		if (Index > 0)
		{
			DrawDebugLine(InWorld, PreviousOuterBottom, OuterBottom, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
			DrawDebugLine(InWorld, PreviousOuterTop, OuterTop, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);

			if (ClampedInnerRadius > UE_SMALL_NUMBER)
			{
				DrawDebugLine(InWorld, PreviousInnerBottom, InnerBottom, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
				DrawDebugLine(InWorld, PreviousInnerTop, InnerTop, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
			}
		}

		if (Index == 0 || Index == ArcSegments)
		{
			DrawDebugLine(InWorld, InnerBottom, OuterBottom, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
			DrawDebugLine(InWorld, InnerTop, OuterTop, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
		}

		PreviousOuterBottom = OuterBottom;
		PreviousOuterTop = OuterTop;
		PreviousInnerBottom = InnerBottom;
		PreviousInnerTop = InnerTop;
	}

	if (ClampedInnerRadius <= UE_SMALL_NUMBER && ClampedAngle < 360.f - UE_KINDA_SMALL_NUMBER)
	{
		DrawDebugLine(
			InWorld,
			Location - HeightOffset,
			Location + HeightOffset,
			Color,
			bPersistentLines,
			LifeTime,
			DepthPriority,
			Thickness);
	}
}

#endif
