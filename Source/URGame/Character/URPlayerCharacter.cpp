// Written by EXPx2


#include "Character/URPlayerCharacter.h"

#include "AbilitySystem/URAbilitySystemComponent.h"

#include "URCharacterInputComponent.h"
#include "URGame.h"
#include "BlueprintLibrary/URWidgetBlueprintLibrary.h"
#include "Input/URInputComponent.h"
#include "Character/URCameraBoomComponent.h"
#include "Definitions/URFaction.h"
#include "GameModes/URGameGameMode.h"
#include "Player/URGamePlayerController.h"
#include "Player/URGamePlayerState.h"
#include "UI/ActorDisplay/URActorDisplayWidget.h"
#include "UI/ActorDisplay/URActorDisplayWidgetComponent.h"
#include "UI/HUD/URGameHUD.h"

AURPlayerCharacter::AURPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// URASCCharacter
	URASC = nullptr;
	
	// InputComponent
	CharacterInputComponent = CreateDefaultSubobject<UURCharacterInputComponent>(TEXT("Character Input Component"));

	// Camera
	CameraBoom = CreateDefaultSubobject<UURCameraBoomComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	
	// Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AURPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UURInputComponent* URInputComponent = Cast<UURInputComponent>(PlayerInputComponent);
	
	check(URInputComponent);
	
	CharacterInputComponent->SetupInputComponent(URInputComponent);
}

void AURPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	SetAbilitySystemComponent();
}

void AURPlayerCharacter::UnPossessed()
{
	CharacterInputComponent->Cleanup();
	
	Super::UnPossessed();
}

void AURPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	
	if (ActorDisplayWidgetComponent)
	{
		ActorDisplayWidgetComponent->RefreshNameColor();
	}

	if (IsLocallyControlled())
	{
		if (AURGamePlayerController* PC = GetController<AURGamePlayerController>())
		{
			PC->EndDeathCamera();
		}
	}
}

void AURPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	SetAbilitySystemComponent();
}

void AURPlayerCharacter::ResetForSpawn()
{
	Super::ResetForSpawn();

	if (!HasAuthority())
	{
		return;
	}
	
	if (!URASC)
	{
		return;
	}
	
	if (AURGamePlayerState* GamePS = GetPlayerState<AURGamePlayerState>())
	{
		GamePS->ResetForSpawn();
	}
}

EURFaction AURPlayerCharacter::GetFaction_Implementation() const
{
	return EURFaction::Ally;
}

void AURPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// if (APlayerController* PC = GetController<APlayerController>())
	// {
	// 	PC->ConsoleCommand(TEXT("ShowDebug AbilitySystem"));
	// }
	
	// 플레이어는 체력 항상 표시
	if (ActorDisplayWidgetComponent)
	{
		if (UURActorDisplayWidget* DisplayWidget = Cast<UURActorDisplayWidget>(ActorDisplayWidgetComponent->GetWidget()))
		{
			DisplayWidget->SetIsHit(true);
			ActorDisplayWidgetComponent->RefreshVisibility();
		}
	}
}

void AURPlayerCharacter::SetAbilitySystemComponent()
{
	AURGamePlayerState* GamePS = GetPlayerState<AURGamePlayerState>();
	if (!GamePS)
	{
		return;
	}
	
	UURAbilitySystemComponent* ASC = GamePS->GetURAbilitySystemComponent();
	check(ASC);
	
	if (URASC == ASC)
	{
		// 이미 설정한 ASC
		ASC->RefreshAbilityActorInfo();
		return;
	}
	
	if (URASC)
	{
		// 이전 ASC 정리
		ClearAbilitySystemComponent();
		URASC = nullptr;
	}
	
	AActor* ExistingAvatar = ASC->GetAvatarActor();
	if (ExistingAvatar && ExistingAvatar != this)
	{
		// 기존 ASC의 아바타 액터가 있을 경우 초기화
		if (AURASCCharacter* ASCCharacter = Cast<AURASCCharacter>(ExistingAvatar))
		{
			ASCCharacter->ClearAbilitySystemComponent();
		}
	}

	URASC = ASC;
	ASC->InitAbilityActorInfo(GamePS, this);
	
	GamePS->InitAbilitySystemComponent();
	InitAbilitySystemComponent();
	
	ResetForSpawn();
	
	GamePS->NotifyPlayerAbilitySystemReady();
}

void AURPlayerCharacter::HandleDeath(AActor* Actor)
{
	Super::HandleDeath(Actor);
	
	if (IsLocallyControlled())
	{
		UURWidgetBlueprintLibrary::OpenPopup(Cast<APlayerController>(GetController()), EPopupType::PlayerDied);
		if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(GetController()))
		{
			GamePC->BeginDeathCamera(FollowCamera);
		}
	}
	
	if (HasAuthority())
	{
		if (AURGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AURGameGameMode>())
		{
			GameMode->OnPlayerDied(this);
		}
	}
}
