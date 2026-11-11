// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

#include "URHitRange.generated.h"


/** 기준 액터 */
UENUM(BlueprintType)
enum class EURRangeOrigin : uint8
{
	SourceActor,
	TargetActor,
};

USTRUCT(BlueprintType)
struct FURHitRange
{
	GENERATED_BODY()
	
public:
	/** 기준 액터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	EURRangeOrigin Origin = EURRangeOrigin::SourceActor;
	
	/** 타겟을 향한 방향에서 회전 값 오프셋<br>0일 경우 타겟을 향한 방향으로 사용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ClampMin=-180.f, ClampMax=180.f, UIMin=-180.f, UIMax=180.f, Units="deg"))
	float DirectionOffset = 0.f;
	
	/** 모양 */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, NoClear, meta=(BaseStruct="/Script/URGame.URColliderShape", ExcludeBaseStruct))
	FInstancedStruct Shape;
};