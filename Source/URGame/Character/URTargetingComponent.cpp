// Written by EXPx2


#include "Character/URTargetingComponent.h"

#include "URGame.h"
#include "Collisions/URCollisions.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interface/URTargetableInterface.h"
#include "SceneView.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"

UURTargetingComponent::UURTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	
	// 로컬에서만 사용되어 복제할 필요 없음
	SetIsReplicatedByDefault(false);
}

void UURTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* Controller = Cast<APlayerController>(GetOwner());
	if (!ensureMsgf(Controller, TEXT("TargetingComponent requires a Controller owner.")) ||
		!Controller->IsLocalController())
	{
		SetComponentTickEnabled(false);
		return;
	}

	OwnerPlayerControllerPtr = Controller;

	// 폰 변화 구독
	Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &ThisClass::HandlePossessedPawnChanged);

	// 구독 전에 이미 빙의되어 있을 수 있으므로 초기 상태도 처리.
	HandlePossessedPawnChanged(nullptr, Controller->GetPawn());
	
	// 정상 설정 시 틱 활성화
	SetComponentTickEnabled(true);
}

void UURTargetingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetComponentTickEnabled(false);

	if (APlayerController* PC = OwnerPlayerControllerPtr.Get())
	{
		PC->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePossessedPawnChanged);
	}
	OwnerPlayerControllerPtr.Reset();
	
	if (AActor* MainTargetActor = MainTargetActorPtr.Get())
	{
		MainTargetActor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleMainTargetEndPlay);
	}
	MainTargetActorPtr.Reset();
	RecentMainTargetActors.Reset();
	
	if (AActor* AimTargetActor = AimTargetActorPtr.Get())
	{
		AimTargetActor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleAimTargetEndPlay);
	}
	AimTargetActorPtr.Reset();
	
	Super::EndPlay(EndPlayReason);
}

void UURTargetingComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ValidateMainTargetActor();
	
	UpdateAimTarget();
}

bool UURTargetingComponent::SetMainTarget(AActor* NewTargetActor, bool bFromCandidates)
{
	const bool bIsNewActorValid = IsValid(NewTargetActor);
	AActor* OldTargetActor = MainTargetActorPtr.Get();
	
	if (bIsNewActorValid)
	{
		// 유효한 액터로 설정 시도
		if (OldTargetActor == NewTargetActor)
		{
			// 현재 타겟임
			return false;
		}
		
		if (!CanSetTarget(NewTargetActor))
		{
			return false;
		}
	}
	else
	{
		// 유효하지 않은 액터로 설정 시도
		if (bFromCandidates)
		{
			// 후보군에서 선택해 설정할 때는 null을 허용하지 않음
			return false;
		}
		if (MainTargetActorPtr.IsExplicitlyNull())
		{
			// 현재 타겟도 null임
			// if (!OldTargetActor)으로 검사하지 않는 이유는 액터가 만료되고 포인터만 남았을 경우에도 OldTargetActor는 nullptr이 되는데 그 경우에도 타겟 변경 브로드캐스트가 호출되어야 하기 때문
			return false;
		}

		// 유효하지 않은 액터로 설정하지 않고 nullptr로 설정
		NewTargetActor = nullptr;
	}
	
	if (!bFromCandidates)
	{
		// 후보군에서 선택된 게 아니라 지정된 액터를 이용해 타겟 설정하면 최근 타겟 배열을 초기화함.
		RecentMainTargetActors.Reset();
	}
	
	if (OldTargetActor)
	{
		// 바인딩 제거
		OldTargetActor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleMainTargetEndPlay);
	}
	
	MainTargetActorPtr = NewTargetActor;
	
	if (bIsNewActorValid)
	{
		// 최근 목록에 저장하고 바인딩 등록
		RecentMainTargetActors.Add(NewTargetActor);
		NewTargetActor->OnEndPlay.AddDynamic(this, &ThisClass::HandleMainTargetEndPlay);
	}

	OnMainTargetChanged.Broadcast(this, OldTargetActor, NewTargetActor);
	return true;
}

void UURTargetingComponent::ResolveMainTarget(const TArray<AActor*>& Candidates)
{
	// 유효하지 않은 최근 목록 제거
	RecentMainTargetActors.RemoveAll([this](const TWeakObjectPtr<AActor>& Actor)
	{
		return !CanSetTarget(Actor.Get());
	});
	
	for (AActor* Actor : Candidates)
	{
		if (RecentMainTargetActors.Contains(Actor))
		{
			// 최근 목록에 있음
			continue;
		}

		if (SetMainTarget(Actor, true))
		{
			return;
		}
	}

	// 타겟 지정 못 했을 경우 최근 목록 지워가며 설정 시도
	
	while (!RecentMainTargetActors.IsEmpty())
	{
		AActor* OldestTarget = RecentMainTargetActors[0].Get();
		RecentMainTargetActors.RemoveAt(0);
		
		if (Candidates.Contains(OldestTarget))
		{
			if (SetMainTarget(OldestTarget, true))
			{
				return;
			}
		}
	}
	
	// 타겟 지정 실패. 별도 액션 하지 않음
}

void UURTargetingComponent::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	SetMainTarget(nullptr);
	SetAimTarget(nullptr);
}

void UURTargetingComponent::ValidateMainTargetActor()
{
	// MainTargetActor가 
	// 1. Destroy 등으로 !IsValid
	// 2. 타겟팅 대상이 될 수 없음
	// 3. 거리가 많이 벌어졌을 경우
	// 메인 타겟을 Reset하고 OnMainTargetChanged 발송
	
	if (MainTargetActorPtr.IsExplicitlyNull())
	{
		// nullptr일 경우 검사 필요 없음
		return;
	}
	
	check(OwnerPlayerControllerPtr.IsValid());

	const AActor* MainTargetActor = MainTargetActorPtr.Get();
	
	if (!CanSetTarget(MainTargetActor))
	{
		SetMainTarget(nullptr);
	}
}

void UURTargetingComponent::UpdateAimTarget()
{
	check(OwnerPlayerControllerPtr.IsValid());
	APlayerController* Controller = OwnerPlayerControllerPtr.Get();
	
	UWorld* World = GetWorld();
	check(World);
	
	const APawn* Owner = Controller->GetPawn();
	AActor* NewTargetActor = nullptr;
	
	if (MaxDistance <= KINDA_SMALL_NUMBER ||
		!Controller->IsLocalController() ||
		!IsValid(Owner))
	{
		// null로 설정
		SetAimTarget(NewTargetActor);
		return;
	}
	
	const FVector OwnerLocation = Owner->GetActorLocation();
	
	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
	const FVector CameraDirection = CameraRotation.Vector();
	// MaxDistance 검사는 카메라가 아닌 캐릭터 기준이기 때문에 캐릭터와 카메라 간의 거리를 더한다
	const float SweepDistance = FVector::Distance(OwnerLocation, CameraLocation) + MaxDistance;
	const FVector SweepEnd = CameraLocation + (CameraDirection * SweepDistance);

	// 플레이어 캐릭터 제외
	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(URAimTarget),
		false,
		Owner);
	
	TArray<FHitResult> Hits;
	World->SweepMultiByChannel(
		Hits,
		CameraLocation,
		SweepEnd,
		FQuat::Identity,
		CCHANNEL_TARGETINGTRACE,
		FCollisionShape::MakeSphere(AimLineCheckRadius),
		QueryParams,
		FCollisionResponseParams(ECR_Overlap));
	
	if (!Hits.IsEmpty())
	{
		float MinScore = FLT_MAX;
		TSet<AActor*> CheckedActor;
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!IsValid(HitActor) ||
				HitActor == Owner ||
				CheckedActor.Contains(HitActor))
			{
				continue;
			}

			CheckedActor.Add(HitActor);

			if (!CanSetTarget(HitActor, EURFaction::Enemy))
			{
				continue;
			}
			
			FVector DeltaToOwner = HitActor->GetActorLocation() - CameraLocation;
			const float Dot = CameraDirection | DeltaToOwner;
			if (Dot < 0.f)
			{
				// 카메라 뒤에 있으면 제외
				continue;
			}
		
			FVector AimLocation;
			UURGameBlueprintLibrary::GetAimTargetLocation(HitActor, AimLocation);
		
			// 에임과의 거리점수만 사용하기 변경하여 굳이 0 ~ 1 범위로 설정할 필요 없음
			const float DistToAimLine = FMath::PointDistToLine(AimLocation, CameraDirection, CameraLocation);
			const float AimLineScore = DistToAimLine / FMath::Max(AimLineCheckRadius, 1.f);
			if (AimLineScore >= MinScore)
			{
				continue;
	
			}
			
			// 벽에 가리는지 체크
			FHitResult VisibilityHit;
			const bool bBlocked = World->LineTraceSingleByChannel(
				VisibilityHit,
				CameraLocation,
				AimLocation,
				ECC_Visibility,
				QueryParams);

			if (bBlocked && 
				VisibilityHit.GetActor() != HitActor)
			{
				continue;
			}
			
			MinScore = AimLineScore;
			NewTargetActor = HitActor;
		}
	}
	
	SetAimTarget(NewTargetActor);
	
}

void UURTargetingComponent::SetAimTarget(AActor* NewTargetActor)
{
	const bool bIsNewActorValid = IsValid(NewTargetActor);
	AActor* OldTargetActor = AimTargetActorPtr.Get();
	
	if (bIsNewActorValid)
	{
		// 유효한 액터로 설정 시도
		if (OldTargetActor == NewTargetActor)
		{
			// 현재 타겟임
			return;
		}
		
		// Note: 타겟팅 가능 대상 검사와 거리 검사는 진행되어 넘어오긴 하지만 다른 곳에서 호출 가능성 있어 남김
		if (!CanSetTarget(NewTargetActor, EURFaction::Enemy))
		{
			return;
		}
	}
	else
	{
		// 유효하지 않은 액터로 설정 시도
		if (AimTargetActorPtr.IsExplicitlyNull())
		{
			// 현재 타겟도 null임
			// if (!OldTargetActor)으로 검사하지 않는 이유는 액터가 만료되고 포인터만 남았을 경우에도 OldTargetActor는 nullptr이 되는데 그 경우에도 타겟 변경 브로드캐스트가 호출되어야 하기 때문
			return;
		}

		// 유효하지 않은 액터로 설정하지 않고 nullptr로 설정
		NewTargetActor = nullptr;
	}
	
	if (OldTargetActor)
	{
		// 바인딩 제거
		OldTargetActor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleAimTargetEndPlay);
	}
	
	AimTargetActorPtr = NewTargetActor;
	
	if (bIsNewActorValid)
	{
		// 바인딩 등록
		NewTargetActor->OnEndPlay.AddDynamic(this, &ThisClass::HandleAimTargetEndPlay);
	}

	OnAimTargetChanged.Broadcast(this, OldTargetActor, NewTargetActor);
}

bool UURTargetingComponent::CanSetTarget(const AActor* InActor, const EURFaction TargetFactions) const
{
	if (!IsValid(InActor))
	{
		// 유효하지 않은 액터
		return false;
	}
	
	if (!InActor->GetClass()->ImplementsInterface(UURTargetableInterface::StaticClass()) ||
		!EnumHasAnyFlags(TargetFactions, IURTargetableInterface::Execute_GetFaction(InActor)) ||
		!IURTargetableInterface::Execute_CanBeTarget(InActor))
	{
		// 타겟팅 가능한 대상이 아님
		return false;
	}
	
	check(OwnerPlayerControllerPtr.IsValid());

	if (const APawn* Pawn = OwnerPlayerControllerPtr->GetPawn())
	{
		if (FVector::DistSquared(Pawn->GetActorLocation(), InActor->GetActorLocation()) > MaxDistance * MaxDistance)
		{
			// 거리가 너무 멂
			return false;
		}
	}
	
	return true;
}

void UURTargetingComponent::HandleMainTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	// Actor가 현재 MainTargetActor가 아니더라도 등록되어 호출되었으므로 바인딩 제거
	Actor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleMainTargetEndPlay);
	
	if (Actor == MainTargetActorPtr.Get())
	{
		SetMainTarget(nullptr);
	}
	
}

void UURTargetingComponent::HandleAimTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	// Actor가 현재 AimTargetActor가 아니더라도 등록되어 호출되었으므로 바인딩 제거
	Actor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleAimTargetEndPlay);
	
	if (Actor == AimTargetActorPtr.Get())
	{
		SetAimTarget(nullptr);
	}
}
