// Written by EXPx2


#include "Character/URDeathTimelineComponent.h"

#include "URGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	FName DeathFadeParameterName = FName("FadeOut");
}

UURDeathTimelineComponent::UURDeathTimelineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	static ConstructorHelpers::FObjectFinder<UCurveFloat> FadeCurveRef(TEXT("/Game/_ProjectUR/Misc/Curve_DeathFade.Curve_DeathFade"));
	FadeCurve = FadeCurveRef.Object;
}


void UURDeathTimelineComponent::BeginPlay()
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
	
	if (FadeCurve == nullptr)
	{
		FadeCurve = NewObject<UCurveFloat>();

		const FKeyHandle StartKey = FadeCurve->FloatCurve.AddKey(0.0f, 0.0f);
		const FKeyHandle EndKey   = FadeCurve->FloatCurve.AddKey(1.0f, 1.0f);
		FadeCurve->FloatCurve.SetKeyInterpMode(StartKey, RCIM_Linear);
		FadeCurve->FloatCurve.SetKeyInterpMode(EndKey, RCIM_Linear);
	}
	
	FOnTimelineFloat TimelineFloatDelegate;
	TimelineFloatDelegate.BindDynamic(this, &ThisClass::HandleTimelineUpdate);
	Timeline.AddInterpFloat(FadeCurve, TimelineFloatDelegate);
	
	FOnTimelineEvent TimelineFinishedDelegate;
	TimelineFinishedDelegate.BindDynamic(this, &ThisClass::HandleTimelineFinished);
	Timeline.SetTimelineFinishedFunc(TimelineFinishedDelegate);
}

void UURDeathTimelineComponent::TickComponent(
	float DeltaTime, 
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Timeline.TickTimeline(DeltaTime);
}

void UURDeathTimelineComponent::SetMeshComponent(UMeshComponent* InMeshComponent)
{
	MeshComp = InMeshComponent;
}

void UURDeathTimelineComponent::PlayFromStart()
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

void UURDeathTimelineComponent::HandleTimelineUpdate(float Value)
{
	if (MeshComp) 
	{
		MeshComp->SetScalarParameterValueOnMaterials(DeathFadeParameterName, Value);
	}
	else
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

void UURDeathTimelineComponent::HandleTimelineFinished()
{
	if (MeshComp)
	{
		MeshComp->SetScalarParameterValueOnMaterials(DeathFadeParameterName, 1.f);
		MeshComp->SetVisibility(false);
	}
	if (FinishParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FinishParticle, GetOwner()->GetActorLocation());
	}
	PrimaryComponentTick.SetTickFunctionEnable(false);
}


