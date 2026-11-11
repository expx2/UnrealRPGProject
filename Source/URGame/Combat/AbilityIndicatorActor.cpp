// Written by EXPx2


#include "Combat/AbilityIndicatorActor.h"

#include "URGame.h"
#include "Collisions/URColliderShape.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "UObject/UObjectGlobals.h"

AAbilityIndicatorActor::AAbilityIndicatorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	
	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	DecalComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	DecalComponent->DecalSize = FVector(200.f, 100.f, 100.f);
	DecalComponent->bDestroyOwnerAfterFade = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RectangleMaterialRef(TEXT("/Game/_ProjectUR/Materials/AbilityIndicator/M_Indicator_Rectangle.M_Indicator_Rectangle"));
	RectangleMaterial = RectangleMaterialRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CircleMaterialRef(TEXT("/Game/_ProjectUR/Materials/AbilityIndicator/M_Indicator_Circle.M_Indicator_Circle"));
	CircleMaterial = CircleMaterialRef.Object;
}

AAbilityIndicatorActor* AAbilityIndicatorActor::Spawn(UWorld* World, const FTransform& Transform,
	const TInstancedStruct<FURColliderShape>& Shape, const double StartServerTime, const float Duration)
{
	if (!World || World->GetNetMode() == NM_Client)
	{
		return nullptr;
	}

	UClass* IndicatorClass = LoadClass<AAbilityIndicatorActor>(nullptr, TEXT("/Game/_ProjectUR/Misc/BP_AbilityIndicator.BP_AbilityIndicator_C"));
	if (!IndicatorClass)
	{
		return nullptr;
	}

	AAbilityIndicatorActor* Indicator = World->SpawnActorDeferred<AAbilityIndicatorActor>(IndicatorClass, Transform);
	if (Indicator)
	{
		Indicator->Initialize(Shape, StartServerTime, Duration);
		Indicator->FinishSpawning(Transform);
	}
	return Indicator;
}

void AAbilityIndicatorActor::Initialize(const TInstancedStruct<FURColliderShape>& Shape,
	const double StartServerTime, const float Duration)
{
	const UScriptStruct* ShapeType = Shape.GetScriptStruct();
	VisualState.bRectangle = ShapeType == FURRectangleShape::StaticStruct();
	VisualState.StartServerTime = StartServerTime;
	VisualState.Duration = Duration;

	if (VisualState.bRectangle)
	{
		// Scale 1 == Width 2
		const FURRectangleShape RectangleShape = Shape.Get<FURRectangleShape>();
		VisualState.Scale = FVector(1.f, RectangleShape.Width * 0.005f, RectangleShape.Length * 0.005f);
	}
	else
	{
		// Scale 1 == Radius 100
		float Radius = 100.f;
		
		if (ShapeType == FURCircleShape::StaticStruct())
		{
			Radius = Shape.Get<FURCircleShape>().Radius;
		}
		else if (ShapeType == FURSectorShape::StaticStruct())
		{
			const FURSectorShape SectorShape = Shape.Get<FURSectorShape>();
			Radius = SectorShape.Radius;
			VisualState.Angle = SectorShape.AngleDegree;
		}
		else if (ShapeType == FURDonutShape::StaticStruct())
		{
			const FURDonutShape DonutShape = Shape.Get<FURDonutShape>();
			Radius = DonutShape.Radius;
			VisualState.InnerHoleRatio = DonutShape.GetInnerHoleRatio();
		}
		else if (ShapeType == FURDonutSectorShape::StaticStruct())
		{
			const FURDonutSectorShape DonutSectorShape = Shape.Get<FURDonutSectorShape>();
			Radius = DonutSectorShape.Radius;
			VisualState.Angle = DonutSectorShape.AngleDegree;
			VisualState.InnerHoleRatio = DonutSectorShape.GetInnerHoleRatio();
		}
		
		const float Scale = Radius * 0.01f; // centimeter to meter
		VisualState.Scale = FVector(1.f, Scale, Scale);
	}
	VisualState.bInitialized = true;
}

void AAbilityIndicatorActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisualState();
	SetActorTickEnabled(GetNetMode() != NM_DedicatedServer);
}

void AAbilityIndicatorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (VisualState.bInitialized && VisualState.Duration > SMALL_NUMBER)
	{
		if (const AGameStateBase* GameState = GetWorld()->GetGameState())
		{
			UpdateProgress((GameState->GetServerWorldTimeSeconds() - VisualState.StartServerTime) / VisualState.Duration);
		}
	}
}

void AAbilityIndicatorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAbilityIndicatorActor, VisualState);
}

void AAbilityIndicatorActor::OnRep_VisualState()
{
	ApplyVisualState();
}

void AAbilityIndicatorActor::ApplyVisualState()
{
	if (!VisualState.bInitialized)
	{
		return;
	}

	SetActorScale3D(VisualState.Scale);
	DecalComponent->SetDecalMaterial(VisualState.bRectangle ? RectangleMaterial : CircleMaterial);
	DynamicInstance = DecalComponent->CreateDynamicMaterialInstance();
	if (DynamicInstance)
	{
		DynamicInstance->SetScalarParameterValue(TEXT("Angle"), VisualState.Angle);
		DynamicInstance->SetScalarParameterValue(TEXT("InnerHoleRatio"), VisualState.InnerHoleRatio);
	}
}

void AAbilityIndicatorActor::UpdateProgress(const float Progress)
{
	if (DynamicInstance)
	{
		DynamicInstance->SetScalarParameterValue("Progress", FMath::Clamp(Progress, 0.f, 1.f));
	}
}

