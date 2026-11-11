// Written by EXPx2


#include "BlueprintLibrary/URCollisionBlueprintLibrary.h"
#include "Collisions/URColliderShape.h"
#include "Collisions/URCollisions.h"
#include "Engine/OverlapResult.h"
#include "Helpers/URDrawDebugHelpers.h"
#include "Interface/URDamageableInterface.h"
#include "Interface/URTargetableInterface.h"
#include "StructUtils/InstancedStruct.h"

/** 서버에서의 추가 허용치 */
namespace ServerTolerance
{
	constexpr float AcceptanceRadiusTolerance = 60.f;
	constexpr float VerticalToleranceTolerance = 30.f;
}

bool UURCollisionBlueprintLibrary::IsActorInRange(
	const FVector& Location,
	const float AcceptanceRadius,
	const float VerticalTolerance,
	const AActor* Actor)
{
	if (!IsValid(Actor) ||
		AcceptanceRadius <= SMALL_NUMBER ||
		VerticalTolerance <= SMALL_NUMBER)
	{
		return false;
	}

	float ActorRadius;
	float ActorHalfHeight;
	Actor->GetSimpleCollisionCylinder(ActorRadius, ActorHalfHeight);

	const FVector Delta = Actor->GetActorLocation() - Location;

	if (FMath::Abs(Delta.Z) > VerticalTolerance + ActorHalfHeight)
	{
		// 높이 판정 범위에 액터의 실린더가 조금이라도 걸치는지 검사
		return false;
	}

	if (Delta.SizeSquared2D() > FMath::Square(AcceptanceRadius + ActorRadius))
	{
		// 거리 판정 범위에 액터의 실린더가 조금이라도 걸치는지 검사
		return false;
	}

	return true;
}

void UURCollisionBlueprintLibrary::GetTargetableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const float AcceptanceRadius,
	const float VerticalTolerance,
	const EURFaction TargetFactions,
	const AActor* SourceActor,
	TArray<AActor*>& OutTargetableActors)
{
	if (AcceptanceRadius <= SMALL_NUMBER ||
		VerticalTolerance <= SMALL_NUMBER)
	{
		return;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World))
	{
		return;
	}

	/*
	 * Note: 서버에선 허용치 더 주기 필요한가?
	 * 요청하는 곳에서 값 설정하여 전송
	 */
	// const bool bIsForRemoteClient = IsForRemoteClient(SourceActor);
	const bool bIsForRemoteClient = true;
	const float ModifiedAcceptanceRadius = bIsForRemoteClient
		                                       ? AcceptanceRadius //  + ServerTolerance::AcceptanceRadiusTolerance
		                                       : AcceptanceRadius;
	const float ModifiedVerticalTolerance = bIsForRemoteClient
		                                        ? VerticalTolerance //  + ServerTolerance::VerticalToleranceTolerance
		                                        : VerticalTolerance;

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(URFindTarget), false, SourceActor);

	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		CCHANNEL_TARGETINGTRACE,
		FCollisionShape::MakeCapsule(ModifiedAcceptanceRadius, ModifiedAcceptanceRadius + ModifiedVerticalTolerance),
		QueryParams);

	if (OverlapResults.Num() < 1)
	{
		return;
	}

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();

		if (!IsValid(HitActor) || // 유효하지 않은 액터 
			OutTargetableActors.Contains(HitActor) ||
			!HitActor->GetClass()->ImplementsInterface(UURTargetableInterface::StaticClass()) || // 타겟팅 가능한 대상이 아님
			!EnumHasAnyFlags(TargetFactions, IURTargetableInterface::Execute_GetFaction(HitActor)) || // 대상 진영이 아님
			!IURTargetableInterface::Execute_CanBeTarget(HitActor)) // 타겟팅 가능한 상태가 아님
		{
			continue;
		}

		if (!IsActorInRange(Location, BIG_NUMBER, ModifiedVerticalTolerance, HitActor))
		{
			// OverlapResult는 캡슐 모양으로 검출한 내용이기 때문에 추가로 높이 검사를 진행하여 실린더 모양으로 제한함
			continue;
		}

		OutTargetableActors.Add(HitActor);
	}

	if (OutTargetableActors.Num() <= 0)
	{
		return;
	}

	// 거리순 정렬
	OutTargetableActors.Sort([&Location](const AActor& Left, const AActor& Right)
	{
		return FVector::DistSquared(Location, Left.GetActorLocation())
			< FVector::DistSquared(Location, Right.GetActorLocation());
	});
}

void UURCollisionBlueprintLibrary::K2_GetTargetableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const float AcceptanceRadius,
	const float VerticalTolerance,
	const int32 TargetFactions,
	const AActor* SourceActor,
	TArray<AActor*>& OutTargetableActors)
{
	GetTargetableActors(WorldContextObject, Location, AcceptanceRadius, VerticalTolerance,
	                    static_cast<EURFaction>(TargetFactions), SourceActor, OutTargetableActors);
}

void UURCollisionBlueprintLibrary::GetDamageableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const FVector& Direction,
	const TInstancedStruct<struct FURColliderShape> Shape,
	const EURFaction TargetFactions,
	const AActor* SourceActor,
	TArray<AActor*>& OutDamageableActors)
{
	OutDamageableActors.Empty();

	const UScriptStruct* ShapeType = Shape.GetScriptStruct();
	if (ShapeType == FURCircleShape::StaticStruct())
	{
		GetDamageableActors(WorldContextObject, Location, Direction, Shape.Get<FURCircleShape>(), TargetFactions, SourceActor, OutDamageableActors);
	}
	else if (ShapeType == FURSectorShape::StaticStruct())
	{
		GetDamageableActors(WorldContextObject, Location, Direction, Shape.Get<FURSectorShape>(), TargetFactions, SourceActor, OutDamageableActors);
	}
	else if (ShapeType == FURDonutShape::StaticStruct())
	{
		GetDamageableActors(WorldContextObject, Location, Direction, Shape.Get<FURDonutShape>(), TargetFactions, SourceActor, OutDamageableActors);
	}
	else if (ShapeType == FURDonutSectorShape::StaticStruct())
	{
		GetDamageableActors(WorldContextObject, Location, Direction, Shape.Get<FURDonutSectorShape>(), TargetFactions, SourceActor, OutDamageableActors);
	}
	else if (ShapeType == FURRectangleShape::StaticStruct())
	{
		GetDamageableActors(WorldContextObject, Location, Direction, Shape.Get<FURRectangleShape>(), TargetFactions, SourceActor, OutDamageableActors);
	}
}

void UURCollisionBlueprintLibrary::GetDamageableActors(
	const UObject* WorldContextObject, 
	const FVector& Location,
	const float AcceptanceRadius, 
	const float VerticalTolerance,
	const EURFaction TargetFactions, 
	const AActor* SourceActor, 
	TArray<AActor*>& OutDamageableActors)
{
	FURCircleShape Shape = FURCircleShape();
	Shape.Radius = AcceptanceRadius;
	Shape.HalfHeight = VerticalTolerance;

	GetDamageableActors(
		WorldContextObject,
		Location,
		FVector::ZeroVector,
		Shape,
		TargetFactions,
		SourceActor,
		OutDamageableActors);
}

void UURCollisionBlueprintLibrary::K2_GetDamageableActors(
	const UObject* WorldContextObject, 
	const FVector& Location,
	const float AcceptanceRadius, 
	const float VerticalTolerance, 
	const int32 TargetFactions,
	const AActor* SourceActor, 
	TArray<AActor*>& OutDamageableActors)
{
	GetDamageableActors(WorldContextObject, Location, AcceptanceRadius, VerticalTolerance, static_cast<EURFaction>(TargetFactions), SourceActor, OutDamageableActors);
}

bool UURCollisionBlueprintLibrary::IsForRemoteClient(const AActor* Actor)
{
	if (Actor && Actor->HasAuthority())
	{
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			if (!Pawn->IsLocallyControlled())
			{
				return true;
			}
		}
	}

	return false;
}

namespace 
{
	FCollisionQueryParams GetDamageableQueryParams(const AActor* SourceActor)
	{
		return FCollisionQueryParams(SCENE_QUERY_STAT(URGetDamageables), false, SourceActor);
	}

	void FilterDamageableActors(
		const TArray<FOverlapResult>& OverlapResults,
		const FVector& Location,
		const float Height,
		const EURFaction TargetFactions,
		const TFunctionRef<bool(AActor*)>& AdditionalFilter,
		TArray<AActor*>& OutDamageableActors)
	{
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* HitActor = OverlapResult.GetActor();

			if (!IsValid(HitActor) ||
				OutDamageableActors.Contains(HitActor) ||
				!UURCollisionBlueprintLibrary::IsActorInRange(Location, BIG_NUMBER, Height, HitActor) || // 캡슐 모양으로 검출한 OverlapResult를 높이 검사를 통해 실린더 모양으로 제한
				!HitActor->GetClass()->ImplementsInterface(UURDamageableInterface::StaticClass()) ||
				!EnumHasAnyFlags(TargetFactions, IURDamageableInterface::Execute_GetFaction(HitActor)) ||
				!IURDamageableInterface::Execute_CanTakeDamage(HitActor))
				
			{
				continue;
			}
			
			if (!AdditionalFilter(HitActor))
			{
				// 추가 필터 검증 실패
				continue;
			}
			
			OutDamageableActors.Add(HitActor);
		}
	}
	
}

void UURCollisionBlueprintLibrary::GetDamageableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const FVector& Direction,
	const FURCircleShape& Shape,
	const EURFaction TargetFactions,
	const AActor* SourceActor,
	TArray<AActor*>& OutDamageableActors)
{
#if ENABLE_DRAW_DEBUG
	DrawDebugCylinder(
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull),
		Location + FVector(0.0f, 0.0f, Shape.HalfHeight),
		Location - FVector(0.0f, 0.0f, Shape.HalfHeight),
		Shape.Radius,
		16,
		FColor::Red,
		false,
		3.0f);
#endif
	
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}
	
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		CCHANNEL_DAMAGETRACE,
		FCollisionShape::MakeCapsule(Shape.Radius, Shape.Radius + Shape.HalfHeight),
		GetDamageableQueryParams(SourceActor));

	FilterDamageableActors(
		OverlapResults,
		Location,
		Shape.HalfHeight,
		TargetFactions,
		[](AActor*)
		{
			return true;
		},
		OutDamageableActors);
}

void UURCollisionBlueprintLibrary::GetDamageableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const FVector& Direction,
	const FURSectorShape& Shape,
	const EURFaction TargetFactions,
	const AActor* SourceActor,
	TArray<AActor*>& OutDamageableActors)
{
#if ENABLE_DRAW_DEBUG
	DrawDebugSector(
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull),
		Location,
		Direction,
		Shape.Radius,
		0.f,
		Shape.AngleDegree,
		Shape.HalfHeight,
		16,
		FColor::Red,
		false,
		3.f);
#endif
	
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}
	
	const float Radius = Shape.Radius;
	
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		CCHANNEL_DAMAGETRACE,
		FCollisionShape::MakeCapsule(Radius, Radius + Shape.HalfHeight),
		GetDamageableQueryParams(SourceActor));
	
	const FVector Direction2D = Direction.GetSafeNormal2D();
	const float HalfRadian = FMath::DegreesToRadians(Shape.AngleDegree * 0.5f);
	const FVector LeftEdge = Direction2D.RotateAngleAxisRad(HalfRadian, FVector::UpVector);
	const FVector RightEdge = Direction2D.RotateAngleAxisRad(-HalfRadian, FVector::UpVector);
	
	
	FilterDamageableActors(
		OverlapResults,
		Location,
		Shape.HalfHeight,
		TargetFactions,
		[&Radius, &Location, &Direction2D, &HalfRadian, &LeftEdge, &RightEdge](const AActor* HitActor)
		{
			const float ActorRadius = HitActor->GetSimpleCollisionRadius();

			const FVector Delta2D = (HitActor->GetActorLocation() - Location) * FVector(1.f, 1.f, 0.f);
			const float Distance = Delta2D.Size2D();
			if (Distance <= ActorRadius)
			{
				// 원이 부채꼴 원점을 포함
				return true;
			}

			/*
			 * Acos 대신 동일하게 비교하는 최적화 코드
			 * 등호가 반대라 직관성이 떨어져 현재는 사용하지 않음
			 */
			// const float CosHalfRadian = FMath::Cos(HalfRadian);
			// if (FVector::DotProduct(Direction, Delta2D) >= Distance * CosHalfRadian)
			// {
			// 	return true;
			// }
			if (FMath::Acos(FMath::Clamp(FVector::DotProduct(Direction2D, Delta2D) / Distance, -1.f, 1.f)) <=
				HalfRadian)
			{
				// 원 중심이 부채꼴 내부에 있음
				return true;
			}


			const float PowRadius = ActorRadius * ActorRadius;
			// 부채꼴의 변에서 원과 가장 가까운 점 구한 뒤 거리가 Radius보다 가까우면 감지
			const float LeftLength = FMath::Clamp(FVector::DotProduct(Delta2D, LeftEdge), 0, Radius);
			const FVector LeftClosestPoint = LeftEdge * LeftLength;
			if (FVector::DistSquared2D(LeftClosestPoint, Delta2D) <= PowRadius)
			{
				return true;
			}

			const float RightLength = FMath::Clamp(FVector::DotProduct(Delta2D, RightEdge), 0, Radius);
			const FVector RightClosestPoint = RightEdge * RightLength;
			if (FVector::DistSquared2D(RightClosestPoint, Delta2D) <= PowRadius)
			{
				return true;
			}

			return false;
		},
		OutDamageableActors);
}

void UURCollisionBlueprintLibrary::GetDamageableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const FVector& Direction,
	const FURDonutShape& Shape,
	const EURFaction TargetFactions,
	const AActor* SourceActor,
	TArray<AActor*>& OutDamageableActors)
{
#if ENABLE_DRAW_DEBUG
	DrawDebugCylinder(
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull),
		Location + FVector(0.0f, 0.0f, Shape.HalfHeight),
		Location - FVector(0.0f, 0.0f, Shape.HalfHeight),
		Shape.Radius,
		16,
		FColor::Red,
		false,
		3.0f);
	DrawDebugCylinder(
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull),
		Location + FVector(0.0f, 0.0f, Shape.HalfHeight),
		Location - FVector(0.0f, 0.0f, Shape.HalfHeight),
		Shape.Radius,
		16,
		FColor::Red,
		false,
		3.0f);
#endif
	
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}
	
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		CCHANNEL_DAMAGETRACE,
		FCollisionShape::MakeCapsule(Shape.Radius, Shape.Radius + Shape.HalfHeight),
		GetDamageableQueryParams(SourceActor));
	
	const float InnerRadius = Shape.InnerRadius;
	
	FilterDamageableActors(
		OverlapResults,
		Location,
		Shape.HalfHeight,
		TargetFactions,
		[&InnerRadius, &Location](const AActor* HitActor)
		{
			// 거리가 (도넛의 InnerRadius - 캡슐 Radius)보다 크면 충돌
			const float ActorRadius = HitActor->GetSimpleCollisionRadius();

			if (ActorRadius >= InnerRadius)
			{
				// InnerRadius보다 캡슐의 크기가 커서 InnerRadius 안에 들어갈 수 없음
				return true;
			}

			return FVector::DistSquared2D(HitActor->GetActorLocation(), Location) >=
				FMath::Square(InnerRadius - ActorRadius);
		},
		OutDamageableActors);
}

void UURCollisionBlueprintLibrary::GetDamageableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const FVector& Direction,
	const FURDonutSectorShape& Shape,
	const EURFaction TargetFactions,
	const AActor* SourceActor,
	TArray<AActor*>& OutDamageableActors)
{
#if ENABLE_DRAW_DEBUG
	DrawDebugSector(
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull),
		Location,
		Direction,
		Shape.Radius,
		Shape.InnerRadius,
		Shape.AngleDegree,
		Shape.HalfHeight,
		16,
		FColor::Red,
		false,
		3.f);
#endif
	
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}
	const float Radius = Shape.Radius;
	
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		FQuat::Identity,
		CCHANNEL_DAMAGETRACE,
		FCollisionShape::MakeCapsule(Radius, Radius + Shape.HalfHeight),
		GetDamageableQueryParams(SourceActor));
	
	
	const float InnerRadius = Shape.InnerRadius;
	const FVector Direction2D = Direction.GetSafeNormal2D();
	const float HalfRadian = FMath::DegreesToRadians(Shape.AngleDegree * 0.5f);
	const FVector LeftEdge = Direction2D.RotateAngleAxisRad(HalfRadian, FVector::UpVector);
	const FVector RightEdge = Direction2D.RotateAngleAxisRad(-HalfRadian, FVector::UpVector);
	
	FilterDamageableActors(
		OverlapResults,
		Location,
		Shape.HalfHeight,
		TargetFactions,
		[&Radius, &InnerRadius, &Location, &Direction2D, &HalfRadian, &LeftEdge, &RightEdge](const AActor* HitActor)
		{
			const float ActorRadius = HitActor->GetSimpleCollisionRadius();

			const FVector Delta2D = (HitActor->GetActorLocation() - Location) * FVector(1.f, 1.f, 0.f);
			const float Distance = Delta2D.Size2D();

			if (Distance + ActorRadius < InnerRadius)
			{
				// 가까워서 도넛홀 내부에 있음
				return false;
			}

			if (Distance <= SMALL_NUMBER)
			{
				// Divided by zero 방지
				// 중심이 같을 경우, 도넛홀 내부에 있는 경우는 이미 체크했으므로 겹친다.
				return true;
			}

			/*
			 * Acos 대신 동일하게 비교하는 최적화 코드
			 * 등호가 반대라 직관성이 떨어져 현재는 사용하지 않음
			 */
			// const float CosHalfRadian = FMath::Cos(HalfRadian);
			// if (FVector::DotProduct(Direction, Delta2D) >= Distance * CosHalfRadian)
			// {
			// 	return true;
			// }
			if (FMath::Acos(FMath::Clamp(FVector::DotProduct(Direction2D, Delta2D) / Distance, -1.f, 1.f)) <= HalfRadian)
			{
				// 원 중심이 부채꼴 내부에 있음
				return true;
			}

			const float PowRadius = ActorRadius * ActorRadius;
			// 부채꼴의 변에서 원과 가장 가까운 점 구한 뒤 거리가 Radius보다 가까우면 감지
			const float LeftLength = FMath::Clamp(FVector::DotProduct(Delta2D, LeftEdge), InnerRadius, Radius);
			const FVector LeftClosestPoint = LeftEdge * LeftLength;
			if (FVector::DistSquared2D(LeftClosestPoint, Delta2D) <= PowRadius)
			{
				return true;
			}

			const float RightLength = FMath::Clamp(FVector::DotProduct(Delta2D, RightEdge), InnerRadius, Radius);
			const FVector RightClosestPoint = RightEdge * RightLength;
			if (FVector::DistSquared2D(RightClosestPoint, Delta2D) <= PowRadius)
			{
				return true;
			}

			return false;
		},
		OutDamageableActors);
}

void UURCollisionBlueprintLibrary::GetDamageableActors(
	const UObject* WorldContextObject,
	const FVector& Location,
	const FVector& Direction,
	const FURRectangleShape& Shape,
	const EURFaction TargetFactions, const
	AActor* SourceActor,
	TArray<AActor*>& OutDamageableActors)
{
#if ENABLE_DRAW_DEBUG
	DrawDebugBox(
		GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull),
		Location,
		FVector(Shape.Length * 0.5f, Shape.Width * 0.5f, Shape.HalfHeight),
		Direction.Rotation().Quaternion(),
		FColor::Red,
		false,
		3.0f);
#endif
	
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}
	
	const FVector Direction2D = Direction.GetSafeNormal2D();
	
	TArray<FOverlapResult> OverlapResults;
	World->OverlapMultiByChannel(
		OverlapResults,
		Location,
		Direction2D.Rotation().Quaternion(),
		CCHANNEL_DAMAGETRACE,
		FCollisionShape::MakeBox(FVector(Shape.Length * 0.5f, Shape.Width * 0.5f, Shape.HalfHeight)),
		GetDamageableQueryParams(SourceActor));
	
	FilterDamageableActors(
		OverlapResults,
		Location,
		Shape.HalfHeight,
		TargetFactions,
		[](AActor*)
		{
			return true;
		},
		OutDamageableActors);
}
