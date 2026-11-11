// Written by EXPx2

#pragma once

#include "CoreMinimal.h"

#include "URColliderShape.generated.h"

USTRUCT(BlueprintType)
struct FURColliderShape
{
	GENERATED_BODY()

public:
	virtual ~FURColliderShape() = default;
	static UScriptStruct* GetScriptStruct() { return StaticStruct(); }

	/** 수직 높이 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="1", UIMin="1", Units="cm"))
	float HalfHeight = 100.f;
};

/** 원 */
USTRUCT(BlueprintType, meta=(DisplayName="Circle"))
struct FURCircleShape : public FURColliderShape
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float Radius = 300.f;
};

/** 부채꼴 */
USTRUCT(BlueprintType, meta=(DisplayName="Sector"))
struct FURSectorShape : public FURColliderShape
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float Radius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", ClampMax="360", Units="deg"))
	float AngleDegree = 90.f;
};

/** 도넛 */
USTRUCT(BlueprintType, meta=(DisplayName="Donut"))
struct FURDonutShape : public FURColliderShape
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float Radius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float InnerRadius = 150.f;

	float GetInnerHoleRatio() const { return InnerRadius / Radius; }
};

/** 도넛 부채꼴 */
USTRUCT(BlueprintType, meta=(DisplayName="Donut Sector"))
struct FURDonutSectorShape : public FURColliderShape
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float Radius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", ClampMax="360", Units="deg"))
	float AngleDegree = 90.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float InnerRadius = 150.f;
	
	float GetInnerHoleRatio() const { return InnerRadius / Radius; }
};


/** 사각형 */
USTRUCT(BlueprintType, meta=(DisplayName="Rectangle"))
struct FURRectangleShape : public FURColliderShape
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float Width = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", Units="cm"))
	float Length = 300.f;
};
