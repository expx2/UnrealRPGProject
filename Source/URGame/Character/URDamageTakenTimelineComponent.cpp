// Written by EXPx2


#include "Character/URDamageTakenTimelineComponent.h"

#include "URGame.h"

namespace
{
	FName DamageParameterName = FName("Damage");
}

UURDamageTakenTimelineComponent::UURDamageTakenTimelineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	static ConstructorHelpers::FObjectFinder<UCurveFloat> FlashCurveRef(TEXT("/Game/_ProjectUR/Misc/Curve_DamageTaken.Curve_DamageTaken"));
	FlashCurve = FlashCurveRef.Object;
}

void UURDamageTakenTimelineComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}
	
	if (!ensure(MeshComp))
	{
		return;
	}
	
	if (FlashCurve == nullptr)
	{
		FlashCurve = NewObject<UCurveFloat>();

		const FKeyHandle StartKey = FlashCurve->FloatCurve.AddKey(0.0f, 0.0f);
		const FKeyHandle MiddleKey = FlashCurve->FloatCurve.AddKey(0.5f, 0.8f);
		const FKeyHandle EndKey   = FlashCurve->FloatCurve.AddKey(1.0f, 0.0f);
		FlashCurve->FloatCurve.SetKeyInterpMode(StartKey, RCIM_Linear);
		FlashCurve->FloatCurve.SetKeyInterpMode(MiddleKey, RCIM_Linear);
		FlashCurve->FloatCurve.SetKeyInterpMode(EndKey, RCIM_Linear);
	}
	
	FOnTimelineFloat TimelineFloatDelegate;
	TimelineFloatDelegate.BindDynamic(this, &ThisClass::HandleTimelineUpdate);
	Timeline.AddInterpFloat(FlashCurve, TimelineFloatDelegate);
	
	FOnTimelineEvent TimelineFinishedDelegate;
	TimelineFinishedDelegate.BindDynamic(this, &ThisClass::HandleTimelineFinished);
	Timeline.SetTimelineFinishedFunc(TimelineFinishedDelegate);
}

void UURDamageTakenTimelineComponent::TickComponent(
	float DeltaTime, 
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Timeline.TickTimeline(DeltaTime);
}

void UURDamageTakenTimelineComponent::SetMeshComponent(UMeshComponent* InMeshComponent)
{
	MeshComp = InMeshComponent;
}

void UURDamageTakenTimelineComponent::PlayFromStart()
{
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		return;
	}
	
	if (ensure(MeshComp))
	{
		Timeline.PlayFromStart();
		PrimaryComponentTick.SetTickFunctionEnable(true);
	}
}

void UURDamageTakenTimelineComponent::HandleTimelineUpdate(float Value)
{
	if (MeshComp)
	{
		MeshComp->SetScalarParameterValueOnMaterials(DamageParameterName, Value);
	}
	else
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

void UURDamageTakenTimelineComponent::HandleTimelineFinished()
{
	if (MeshComp)
	{
		MeshComp->SetScalarParameterValueOnMaterials(DamageParameterName, 0.f);
	}
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

