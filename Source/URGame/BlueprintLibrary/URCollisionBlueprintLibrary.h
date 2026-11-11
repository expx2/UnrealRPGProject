// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "URCollisionBlueprintLibrary.generated.h"

enum class EURFaction : uint8;
/**
 * 
 */
UCLASS()
class URGAME_API UURCollisionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * 액터가 지정된 범위 내에 있는지 검사한다. 액터의 실린더가 조금이라도 걸치면 참으로 판단한다.
	 * @param Location 검사 기준 위치
	 * @param AcceptanceRadius 검사 기준 위치로부터 허용 XY거리
	 * @param VerticalTolerance 검사 기준 위치로부터 허용 높이
	 * @param Actor 검사 대상 액터
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "UR|Collision")
	static bool IsActorInRange(
		const FVector& Location, 
		const float AcceptanceRadius, 
		const float VerticalTolerance,
		const AActor* Actor);


	/**
	 * 범위 내의 ITargetableInterface를 상속받은 액터들을 구한다. 
	 * @param WorldContextObject 
	 * @param Location 
	 * @param AcceptanceRadius 
	 * @param VerticalTolerance 
	 * @param TargetFactions 
	 * @param SourceActor 
	 * @param OutTargetableActors 
	 */
	static void GetTargetableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const float AcceptanceRadius,
		const float VerticalTolerance,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutTargetableActors);
	
	UFUNCTION(BlueprintCallable, Category = "UR|Collision", DisplayName="Get Targetable Actors", meta=(WorldContext="WorldContextObject"))
	static void K2_GetTargetableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const float AcceptanceRadius,
		const float VerticalTolerance,
		UPARAM(meta=(Bitmask, BitmaskEnum="/Script/URGame.EURFaction"))
		const int32 TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutTargetableActors);
	
	/**
	 * 범위 내의 IDamageableInterface를 상속받은 액터들을 구한다. 
	 */
	static void GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Direction,
		const TInstancedStruct<struct FURColliderShape> Shape,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);
	
	static void GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const float AcceptanceRadius,
		const float VerticalTolerance,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);
	
	UFUNCTION(BlueprintCallable, Category = "UR|Collision", DisplayName="Get Damageable Actors", meta=(WorldContext="WorldContextObject"))
	static void K2_GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const float AcceptanceRadius,
		const float VerticalTolerance,
		UPARAM(meta=(Bitmask, BitmaskEnum="/Script/URGame.EURFaction"))
		const int32 TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);
	
protected:
	static bool IsForRemoteClient(const AActor* Actor);

	static void GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Direction,
		const struct FURCircleShape& Shape,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);

	static void GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Direction,
		const struct FURSectorShape& Shape,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);

	static void GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Direction,
		const struct FURDonutShape& Shape,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);

	static void GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Direction,
		const struct FURDonutSectorShape& Shape,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);

	static void GetDamageableActors(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Direction,
		const struct FURRectangleShape& Shape,
		const EURFaction TargetFactions,
		const AActor* SourceActor,
		TArray<AActor*>& OutDamageableActors);
};
