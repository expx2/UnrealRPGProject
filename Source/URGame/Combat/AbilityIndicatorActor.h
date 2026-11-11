// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructUtils/InstancedStruct.h"
#include "AbilityIndicatorActor.generated.h"

UENUM(BlueprintType)
enum class EAbilityIndicatorType : uint8
{
	Circle,
	Donut,
	Sector,
	DonutSector,
	Rectangle,
};

USTRUCT()
struct FAbilityIndicatorVisualState
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Scale = FVector::OneVector;

	UPROPERTY()
	float Angle = 360.f;

	UPROPERTY()
	float InnerHoleRatio = 0.f;

	UPROPERTY()
	double StartServerTime = 0.0;

	UPROPERTY()
	float Duration = 0.f;

	UPROPERTY()
	bool bRectangle = false;

	UPROPERTY()
	bool bInitialized = false;
};

UCLASS()
class URGAME_API AAbilityIndicatorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAbilityIndicatorActor();
	
	static AAbilityIndicatorActor* Spawn(UWorld* World, const FTransform& Transform,
		const TInstancedStruct<struct FURColliderShape>& Shape, double StartServerTime, float Duration);

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	void Initialize(const TInstancedStruct<struct FURColliderShape>& Shape, double StartServerTime, float Duration);
	void ApplyVisualState();
	void UpdateProgress(float Progress);

	UFUNCTION()
	void OnRep_VisualState();

	// Shape parameters and timing replicate; each client computes Progress locally.
	UPROPERTY(ReplicatedUsing=OnRep_VisualState)
	FAbilityIndicatorVisualState VisualState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<UDecalComponent> DecalComponent;
\
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UMaterialInterface> CircleMaterial;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UMaterialInterface> RectangleMaterial;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicInstance;
};
