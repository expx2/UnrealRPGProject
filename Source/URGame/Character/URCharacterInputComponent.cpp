// Written by EXPx2


#include "Character/URCharacterInputComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "URGame.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "GameplayTags/URGameplayTags.h"
#include "Input/URInputComponent.h"
#include "Character/URCameraBoomComponent.h"
#include "Input/URInputMappingSet.h"
#include "Navigation/PathFollowingComponent.h"
#include "Player/URGamePlayerController.h"

class UPathFollowingComponent;
// Sets default values for this component's properties
UURCharacterInputComponent::UURCharacterInputComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UURInputMappingSet> CharacterInputSetRef(TEXT("/Game/_ProjectUR/Inputs/DA_InputMappingSet.DA_InputMappingSet"));
	CharacterInputSet = CharacterInputSetRef.Object;
}

void UURCharacterInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Cleanup();
	
	Super::EndPlay(EndPlayReason);
}

void UURCharacterInputComponent::SetupInputComponent(UURInputComponent* InInputComponent)
{
	if (!ensureMsgf(IsValid(CharacterInputSet), TEXT("%s has no CharacterInputSet"), *GetNameSafe(GetOwner())))
	{
		return;
	}
	
	if (InInputComponent == BoundInputComponentPtr.Get())
	{
		// 이미 설정한 컴포넌트
		return;
	}
	
	Cleanup();
	
	InInputComponent = GetValid(InInputComponent);
	if (!InInputComponent)
	{
		// null로 설정
		return;
	}
	
	// InInputComponent는 Valid함

	bool IsMappingContextAdded = false;
	TryAddMappingContext(IsMappingContextAdded);

	if (!IsMappingContextAdded)
	{
		return;
	}

	for (FNativeInputMapping& InputMapping : CharacterInputSet->NativeInputMappings)
	{
		if (!IsValid(InputMapping.InputAction))
		{
			continue;
		}

		switch (InputMapping.Command)
		{
		case ENativeInputBinds::MouseLook:
			InInputComponent->BindNativeAction(InputMapping.InputAction, ETriggerEvent::Triggered, this, &ThisClass::OnInputLookMouse);
			continue;
		case ENativeInputBinds::StickLook:
			InInputComponent->BindNativeAction(InputMapping.InputAction, ETriggerEvent::Triggered, this, &ThisClass::OnInputLookStick);
			continue;
		case ENativeInputBinds::Move:
			InInputComponent->BindNativeAction(InputMapping.InputAction, ETriggerEvent::Triggered, this, &ThisClass::OnInputMove);
			InInputComponent->BindNativeAction(InputMapping.InputAction, ETriggerEvent::Completed, this, &ThisClass::OnInputMoveEnd);
			continue;
		case ENativeInputBinds::WheelZoom:
			InInputComponent->BindNativeAction(InputMapping.InputAction, ETriggerEvent::Triggered, this, &ThisClass::OnInputZoomWheel);
			continue;
		default:
			continue;
		}
	}
	
	for (FAbilityInputMapping& InputMapping : CharacterInputSet->AbilityInputMappings)
	{
		if (!IsValid(InputMapping.InputAction))
		{
			continue;
		}

		InInputComponent->BindAbilityAction(InputMapping.InputAction, this, &ThisClass::OnAbilityInputTriggered, &ThisClass::OnAbilityInputReleased, InputMapping.Command);
	}
	
	BoundInputComponentPtr = InInputComponent;
}

void UURCharacterInputComponent::Cleanup()
{
	if (BoundInputComponentPtr.IsValid())
	{
		BoundInputComponentPtr->ResetBindings();
	}
	BoundInputComponentPtr.Reset();
	
	if (BoundPlayerControllerPtr.IsValid() && CharacterInputSet)
	{
		BoundPlayerControllerPtr->RemoveCharacterInputMappingContext(this, CharacterInputSet->InputMappingContext);
	}
	BoundPlayerControllerPtr.Reset();
}


void UURCharacterInputComponent::TryAddMappingContext(bool& IsSuccess)
{
	if (CharacterInputSet && CharacterInputSet->InputMappingContext)
	{
		if (const APawn* Pawn = GetPawn<APawn>())
		{
			if (AURGamePlayerController* PlayerController = Cast<AURGamePlayerController>(Pawn->GetController()))
			{
				IsSuccess = PlayerController->AddCharacterInputMappingContext(this, CharacterInputSet->InputMappingContext);
				if (IsSuccess)
				{
					BoundPlayerControllerPtr = PlayerController;
				}
				return;
			}
		}
	}
	
	IsSuccess = false;
}

void UURCharacterInputComponent::OnInputLookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	
	if (Value.X != 0.f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}
	if (Value.Y != 0.f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UURCharacterInputComponent::OnInputLookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		constexpr float YawRate = 30.f;
		Pawn->AddControllerYawInput(Value.X * YawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		constexpr float PitchRate = 30.f;
		Pawn->AddControllerPitchInput(Value.Y * PitchRate * World->GetDeltaSeconds());
	}
}

void UURCharacterInputComponent::OnInputMove(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}

	if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn))
	{
		if (ASC->HasMatchingGameplayTag(URGameplayTags::Player_State_InputBlocked_Movement))
		{
			// 이동 입력 제한
			return;
		}
	}
	
	const AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	UpdateMovementInput(Value);
	
	UPathFollowingComponent* PathFollowingComp = GetPathFollowingComponent();
	if (PathFollowingComp && PathFollowingComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		// 자동 이동 중엔 입력으로 인한 이동 제한
		// 입력 변경 시 자동 이동 해제됨
		return;
	}
	
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (!FMath::IsNearlyZero(Value.X))
	{
		Pawn->AddMovementInput(MovementRotation.RotateVector(FVector::RightVector), Value.X);
	}

	if (!FMath::IsNearlyZero(Value.Y))
	{
		Pawn->AddMovementInput(MovementRotation.RotateVector(FVector::ForwardVector), Value.Y);
	}
}

void UURCharacterInputComponent::OnInputMoveEnd(const FInputActionValue& InputActionValue)
{
	UpdateMovementInput(FVector2D::ZeroVector);
}

void UURCharacterInputComponent::OnInputZoomWheel(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !Pawn->IsLocallyControlled())
	{
		return;
	}
	
	UURCameraBoomComponent* CameraBoom = Pawn->FindComponentByClass<UURCameraBoomComponent>();
	if (!CameraBoom)
	{
		return;
	}
	
	const float Value = InputActionValue.Get<float>();
	CameraBoom->Zoom(Value);
}

void UURCharacterInputComponent::OnAbilityInputTriggered(const EAbilityInputBinds::Type InputBinding)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (UURAbilitySystemComponent* ASC = Cast<UURAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn)))
		{
			ASC->AbilityInputTagTriggered(InputBinding);
		}
	}
}

void UURCharacterInputComponent::OnAbilityInputReleased(const EAbilityInputBinds::Type InputBinding)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (UURAbilitySystemComponent* ASC = Cast<UURAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn)))
		{
			ASC->AbilityInputTagReleased(InputBinding);
		}
	}
}

void UURCharacterInputComponent::UpdateMovementInput(const FVector2D& NewInputValue)
{
	if (NewInputValue.Equals(CurrentMovementInputValue, SMALL_NUMBER))
	{
		// 같은 값
		return;
	}
	
	// Note: 패드 지원 시 허용치 설정 필요 
	const FVector2D LastMovementInputValue = CurrentMovementInputValue;
	CurrentMovementInputValue = NewInputValue;
	
	/*
	 * Note: 이동 입력 시 자동 이동을 취소하는 방식
	 * PathFollowingComp를 찾아 Abort하면 모든 자동 이동에 추가 작업 없이 대응할 수 있지만,
	 * 동시에 입력으로 인한 취소를 사용하지 않는 자동 이동이 사용 불가능함
	 */
	if (NewInputValue.IsNearlyZero())
	{
		return;
	}
	UPathFollowingComponent* PathFollowingComp = GetPathFollowingComponent();
	if (PathFollowingComp && PathFollowingComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		PathFollowingComp->AbortMove(*this, FPathFollowingResultFlags::UserAbort);
	}
	// OnMovementInputChanged.Broadcast(LastMovementInputValue, NewInputValue); 
}

UPathFollowingComponent* UURCharacterInputComponent::GetPathFollowingComponent() const
{
	AController* Controller = GetController<AController>();
	AAIController* AsAIController = Cast<AAIController>(Controller);
	UPathFollowingComponent* PathFollowingComp = nullptr;
	
	if (AsAIController)
	{
		PathFollowingComp = AsAIController->GetPathFollowingComponent();
	}
	else
	{
		PathFollowingComp = Controller->FindComponentByClass<UPathFollowingComponent>();
	}
	return PathFollowingComp;
}
