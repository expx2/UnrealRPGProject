// Written by EXPx2

#include "URGame.h"
#include "URGamePlayerController.h"
#include "URGamePlayerState.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "BlueprintLibrary/URWidgetBlueprintLibrary.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "GameStates/URGameGameState.h"
#include "UI/HUD/URGameHUD.h"

void AURGamePlayerController::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
	if (IsLocalController())
	{
		if (bIsUsingDeathCamera && NewViewTarget != DeathCamera)
		{
			// DeathCamera 종료 시 설정하도록 저장
			PendingViewTarget = NewViewTarget;
			PendingViewTargetTransitionParams = TransitionParams;
			return;
		}
	}
	
	Super::SetViewTarget(NewViewTarget, TransitionParams);
	
	if (IsLocalController())
	{
		if (AURGameHUD* GameHUD = GetHUD<AURGameHUD>())
		{
			GameHUD->SetViewTarget(GetViewTargetPawn());
		}
	}
}

void AURGamePlayerController::ServerStartSpectating_Implementation()
{
	StartSpectating();
}

void AURGamePlayerController::BeginDeathCamera(UCameraComponent* CurrentCamera)
{
	if (!IsLocalController() || !PlayerCameraManager || !DeathCameraClass)
	{
		return;
	}
	
	if (!IsValid(DeathCamera))
	{
		DeathCamera = GetWorld()->SpawnActor<ACameraActor>(DeathCameraClass);
		if (!IsValid(DeathCamera))
		{
			return;
		}
	}
	
	// 카메라 복사
	FMinimalViewInfo ViewInfo;
	if (CurrentCamera)
	{
		CurrentCamera->GetCameraView(0.f, ViewInfo);
	}
	else
	{
		ViewInfo = PlayerCameraManager->GetCameraCacheView();
	}
	
	DeathCamera->SetActorLocationAndRotation(ViewInfo.Location,ViewInfo.Rotation);
	UCameraComponent* CameraComp = DeathCamera->GetCameraComponent();
	CameraComp->SetFieldOfView(ViewInfo.FOV);
	CameraComp->SetProjectionMode(ViewInfo.ProjectionMode);
	CameraComp->SetConstraintAspectRatio(ViewInfo.bConstrainAspectRatio);
	CameraComp->SetAspectRatio(ViewInfo.AspectRatio);
	// CameraComp->PostProcessSettings = ViewInfo.PostProcessSettings;
	// CameraComp->PostProcessBlendWeight = ViewInfo.PostProcessBlendWeight;
	
	// DeathCamera 종료될 때 돌아올 수 있도록 기존 뷰타겟 저장
	PendingViewTarget = GetViewTarget();
	
	// bLockOutgoing을 true로 설정하여 블렌딩 시작 시 출발 화면을 고정해 화면 튐 방지
	SetViewTargetWithBlend(DeathCamera, 0.5f, VTBlend_Linear, 0.f, true);
	
	bIsUsingDeathCamera = true;
}

void AURGamePlayerController::EndDeathCamera()
{
	bIsUsingDeathCamera = false;
	
	// 저장해둔 대기 뷰타겟으로 설정
	if (PendingViewTarget.IsValid())
	{
		SetViewTarget(PendingViewTarget.Get(), PendingViewTargetTransitionParams);
	}
	PendingViewTarget.Reset();
}

void AURGamePlayerController::ClientOnAllPlayersDied_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}
	
	if (AURGameGameState* GameGS = GetWorld()->GetGameState<AURGameGameState>())
	{
		if (GameGS->IsCleared())
		{
			return;
		}
	}
	
	BeginDeathCamera(nullptr);
	UURWidgetBlueprintLibrary::OpenPopup(this, EPopupType::PlayerDied);
}

void AURGamePlayerController::ClientClearGame_Implementation()
{
	UURWidgetBlueprintLibrary::ClosePopupAll(this);
	UURWidgetBlueprintLibrary::OpenPopup(this, EPopupType::Clear);
}

APawn* AURGamePlayerController::GetViewTargetPawn() const
{
	AActor* ViewTarget = GetViewTarget();
	if (ViewTarget == nullptr)
	{
		return nullptr;
	}
	
	if (APawn* Pwn = Cast<APawn>(ViewTarget))
	{
		return Pwn;
	}
	
	if (APlayerController* PC = Cast<APlayerController>(ViewTarget))
	{
		return PC->GetPawn();
	}
	
	if (APlayerState* PS = Cast<APlayerState>(ViewTarget))
	{
		return PS->GetPawn();
	}
	
	return nullptr;
}

void AURGamePlayerController::BeginPlayingState()
{
	EndDeathCamera();
	bAutoManageActiveCameraTarget = true;
	
	Super::BeginPlayingState();
	
	if (IsLocalController())
	{
		UURWidgetBlueprintLibrary::ClosePopup(this, EPopupType::PlayerDied);
		if (AURGameHUD* GameHUD = GetHUD<AURGameHUD>())
		{
			GameHUD->SetIsSpectating(false);
		}
	}
}

void AURGamePlayerController::BeginSpectatingState()
{
	EndDeathCamera();
	// SetPawn(nullptr)이 되면서 AutoManageActiveCameraTarget이 호출되어 Controller 등으로 ViewTarget이 지정되는걸 방지한다. 
	bAutoManageActiveCameraTarget = false;
	
	Super::BeginSpectatingState();
	
	if (IsLocalController())
	{
		UURWidgetBlueprintLibrary::ClosePopup(this, EPopupType::PlayerDied);
		if (AURGameHUD* GameHUD = GetHUD<AURGameHUD>())
		{
			GameHUD->SetIsSpectating(true);
		}
	}
}

void AURGamePlayerController::StartSpectating()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (AURGamePlayerState* GamePS = GetPlayerState<AURGamePlayerState>())
	{
		if (!GamePS->IsDead())
		{
			return;
		}
	}
	
	if (UWorld* World = GetWorld())
	{
		if (AURGameGameState* GameGS = World->GetGameState<AURGameGameState>())
		{
			if (GameGS->IsAllPlayersDead())
			{
				return;
			}
		}
	}
	
	if (PlayerState)
	{
		PlayerState->SetIsSpectator(true);
	}
	
	ChangeState(NAME_Spectating);
	ClientGotoState(NAME_Spectating);

	ServerViewNextPlayer();
}
