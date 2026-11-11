// Written by EXPx2

#include "Player/URGamePlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "URGame.h"
#include "URGamePlayerState.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "Camera/CameraActor.h"
#include "Character/URCharacterInputComponent.h"
#include "Character/URPlayerCharacter.h"
#include "Character/URTargetingComponent.h"
#include "Components/URPathFollowingComponent.h"
#include "DataAssets/PlayerCharactersSet.h"
#include "GameInstances/URGameInstance.h"
#include "GameModes/URGameGameMode.h"
#include "GameStates/URGameGameState.h"
#include "Navigation/PathFollowingComponent.h"
#include "UI/URUISubsystem.h"
#include "UI/ActorDisplay/URActorDisplayWidget.h"
#include "UI/ActorDisplay/URActorDisplayWidgetComponent.h"
#include "UI/HUD/URGameHUD.h"

AURGamePlayerController::AURGamePlayerController(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PathFollowingComponent = CreateDefaultSubobject<UURPathFollowingComponent>(TEXT("UR Path Following Component"));
	if (PathFollowingComponent)
	{
		PathFollowingComponent->OnRequestFinished.AddUObject(this, &ThisClass::HandleMoveRequestFinished);
	}
	
	// TargetingComponent
	TargetingComponent = CreateDefaultSubobject<UURTargetingComponent>(TEXT("Targeting Component"));
	
	static ConstructorHelpers::FClassFinder<ACameraActor> DeathCameraRef(TEXT("/Game/_ProjectUR/Misc/BP_DeathCamera"));
	DeathCameraClass = DeathCameraRef.Class;
}

void AURGamePlayerController::SetPawn(APawn* InPawn)
{
	// 이동 초기화
	if (PathFollowingComponent)
	{
		ClearPendingAbility();
		PathFollowingComponent->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished);
		PathFollowingComponent->Cleanup();
	}
	
	Super::SetPawn(InPawn);
	
	if (PathFollowingComponent)
	{
		PathFollowingComponent->Initialize();
	}
	
	if (IsLocalController())
	{
		if (AURGameHUD* GameHUD = GetHUD<AURGameHUD>())
		{
			IURActorReceiverInterface::Execute_ReceiveActor(GameHUD, GetPawn());
		}
	}
}

void AURGamePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	CheckPendingAbilityToActivate();
}

void AURGamePlayerController::SetCinematicMode(
	bool bInCinematicMode, 
	bool bHidePlayer, 
	bool bAffectsHUD,
	bool bAffectsMovement, 
	bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning);
	
	if (bAffectsHUD)
	{
		if (AURGameHUD* GameHUD = GetHUD<AURGameHUD>())
		{
			GameHUD->UpdateVisibility();
		}
		
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
			{
				if (AHUD* HUD = GetHUD())
				{
					UISystem->UpdateCinematicMode(bInCinematicMode);
				}
			}
		}
	}
}

void AURGamePlayerController::PawnLeavingGame()
{
	// 연결 끊을 때도 플레이어 캐릭터 사망 처리해서 관전이나 리스폰 처리할 수 있도록 함
	if (HasAuthority())
	{
		if (AURGamePlayerState* GamePS = GetPlayerState<AURGamePlayerState>())
		{
			if (!GamePS->IsDead())
			{
				// 이미 사망 호출했으면 호출하지 않음
				if (AURPlayerCharacter* PC = GetPawn<AURPlayerCharacter>())
				{
					if (AURGameGameMode* GameGM = GetWorld()->GetAuthGameMode<AURGameGameMode>())
					{
						GameGM->OnPlayerDied(PC);
					}
				}
			}
		}
	}
	
	Super::PawnLeavingGame();
}

void AURGamePlayerController::ReceiveAbilitySystemComponent_Implementation(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (IsLocalController())
	{
		if (AHUD* HUD = GetHUD())
		{
			if (HUD->GetClass()->ImplementsInterface(UURAbilitySystemComponentReceiverInterface::StaticClass()))
			{
				IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(HUD, AbilitySystemComponent);
			}
		}
	}
}

void AURGamePlayerController::InitializeCharacterCode()
{
	if (bIsWaitingCharacterCode)
	{
		return;
	}
	
	bIsWaitingCharacterCode = true;
	ClientRequestCharacterCode();
}

void AURGamePlayerController::ClientRequestCharacterCode_Implementation()
{
	// 서버 RPC로 선택한 캐릭터 코드 전달
	if (UURGameInstance* URGI = GetGameInstance<UURGameInstance>())
	{
		SubmitCharacterCode(URGI->LocalSelectedCharacterCode);
	}
}

void AURGamePlayerController::SubmitCharacterCode_Implementation(const FName InCharacterCode)
{
	if (!bIsWaitingCharacterCode)
	{
		return;
	}

	AURGamePlayerState* PS = GetPlayerState<AURGamePlayerState>();
	AURGameGameMode* GM = GetWorld()->GetAuthGameMode<AURGameGameMode>();
	AURGameGameState* GS = GetWorld()->GetGameState<AURGameGameState>();

	if (!PS || !GM || !GS || GS->IsCleared() || GetPawn())
	{
		return;
	}
	
	PS->PlayingCharacterCode = InCharacterCode;
	GM->RestartPlayer(this);
}

bool AURGamePlayerController::SubmitCharacterCode_Validate(const FName InCharacterCode)
{
	if (UURGameInstance* URGI = GetGameInstance<UURGameInstance>())
	{
		if (UClass* CharacterClass = URGI->GetCharacterClass(InCharacterCode))
		{
			return true;
		}
	}
	
	return false;
}

void AURGamePlayerController::ClientDamageTakenFeedback_Implementation(
	AActor* EffectInstigator,
	AActor* HitActor,
	const float Magnitude,
	bool bIsCritical)
{
	if (!IsLocalController() || 
		!IsValid(HitActor))
	{
		return;
	}
	
	if (const UURActorDisplayWidgetComponent* DisplayWidgetComponent = HitActor->FindComponentByClass<UURActorDisplayWidgetComponent>())
	{
		if (UURActorDisplayWidget* DisplayWidget = Cast<UURActorDisplayWidget>(DisplayWidgetComponent->GetWidget()))
		{
			DisplayWidget->SetIsHit(true);
		}
	}
	
	if (AURGameHUD* GameHUD = GetHUD<AURGameHUD>())
	{
		GameHUD->ShowDamage(HitActor, Magnitude, bIsCritical);
	}
}

void AURGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		FInputModeGameOnly InputModeData;
		SetInputMode(InputModeData);
		
		if (AURGamePlayerState* GamePS = GetPlayerState<AURGamePlayerState>())
		{
			IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(this, GamePS->GetAbilitySystemComponent());
		}
	}
}

bool AURGamePlayerController::AddCharacterInputMappingContext(
	UURCharacterInputComponent* InInputComponent,
	UInputMappingContext* InMappingContext)
{
	if (!IsValid(InInputComponent) || 
		!IsValid(InMappingContext))
	{
		return false;
	}
	
	if (SetCharacterInputComponent == InInputComponent &&
		SetCharacterInputMappingContext == InMappingContext)
	{
		// 이미 설정한 것과 같은 내용
		return true;
	}
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			if (const UInputMappingContext* OldIMC = SetCharacterInputMappingContext.Get())
			{
				// 기존 매핑 제거
				InputSystem->RemoveMappingContext(OldIMC);
			}
			
			InputSystem->AddMappingContext(InMappingContext, 0);
			SetCharacterInputComponent = InInputComponent;
			SetCharacterInputMappingContext = InMappingContext;
			
			return true;
		}
	}
	
	return false;
}

void AURGamePlayerController::RemoveCharacterInputMappingContext(
	UURCharacterInputComponent* InInputComponent,
	UInputMappingContext* InMappingContext)
{
	if (!InMappingContext ||
		SetCharacterInputComponent != InInputComponent ||
		SetCharacterInputMappingContext != InMappingContext)
	{
		// 설정한 IMC가 아님
		return;
	}
	
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			InputSystem->RemoveMappingContext(InMappingContext);
		}
	}
	
	SetCharacterInputComponent.Reset();
	SetCharacterInputMappingContext.Reset();
}
