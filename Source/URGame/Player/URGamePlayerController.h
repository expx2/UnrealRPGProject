// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AITypes.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "URGamePlayerController.generated.h"

/** 사용 가능 범위까지 자동 이동을 하며 사용 대기 중인 어빌리티 정보 */
USTRUCT(BlueprintType)
struct FURPendingAbility
{
	GENERATED_BODY()
	
	bool IsValid() const { return AbilityToActivate.IsValid() && MoveRequestID > 0; }
	
	UPROPERTY(BlueprintReadWrite, Category="UR")
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadWrite, Category="UR")
	TWeakObjectPtr<APawn> Pawn;
	
	UPROPERTY(BlueprintReadWrite, Category="UR")
	FGameplayAbilitySpecHandle AbilityToActivate;
	
	UPROPERTY(BlueprintReadWrite, Category="UR")
	TWeakObjectPtr<AActor> TargetActor;
	
	UPROPERTY(BlueprintReadWrite, Category="UR")
	int32 MoveRequestID = -1; // FAIRequestID
	/*
	 * Note
	 * FAIRequestID는 uint32를 값으로 가지지만 uint32는 Blueprint 사용 불가라 int32 사용함. 
	 * FAIRequestID::ToString에서 int32를 사용하므로 문제가 될 가능성은 적을 듯. 
	 */
};

USTRUCT()
struct FURAbilityFailedRecord
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;
	
	UPROPERTY()
	int16 PredictionKey = -1;
	
	UPROPERTY()
	double Time = 0.0;
};

/**
 * 
 */
UCLASS()
class URGAME_API AURGamePlayerController : public APlayerController,
                                           public IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()
	
public:
	AURGamePlayerController(const FObjectInitializer& ObjectInitializer);

	virtual void SetPawn(APawn* InPawn) override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;
	
	virtual void PawnLeavingGame() override;
	
	virtual void ReceiveAbilitySystemComponent_Implementation(class UAbilitySystemComponent* AbilitySystemComponent) override;
	
	void InitializeCharacterCode();
	
	UFUNCTION(Client,Reliable)
	void ClientRequestCharacterCode();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void SubmitCharacterCode(FName InCharacterCode);
	
	UFUNCTION(Client, Unreliable)
	void ClientDamageTakenFeedback(AActor* EffectInstigator, AActor* HitActor, float Magnitude, bool bIsCritical);

	class UURTargetingComponent* GetTargetingComponent() const { return TargetingComponent; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURTargetingComponent> TargetingComponent;
	
	bool bIsWaitingCharacterCode = false;
	
	// --------------------------------------
	//	Input
	// --------------------------------------
public:
	bool AddCharacterInputMappingContext(class UURCharacterInputComponent* InInputComponent, class UInputMappingContext* InMappingContext);
	
	void RemoveCharacterInputMappingContext(class UURCharacterInputComponent* InInputComponent, class UInputMappingContext* InMappingContext);
	
protected:
	TWeakObjectPtr<class UURCharacterInputComponent> SetCharacterInputComponent;
	TWeakObjectPtr<class UInputMappingContext> SetCharacterInputMappingContext;
	
	// --------------------------------------
	// 관전과 플레이어 사망 시 카메라 및 State 전환 관련
	// URPlayerController_Spectating.cpp에 작성한다.
	// --------------------------------------
public:
	virtual void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerStartSpectating();
	
	void BeginDeathCamera(UCameraComponent* CurrentCamera);
	
	void EndDeathCamera();
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientOnAllPlayersDied();
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientClearGame();
	
	APawn* GetViewTargetPawn() const;

protected:
	virtual void BeginPlayingState() override;
	
	virtual void BeginSpectatingState() override;
	
	void StartSpectating();
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TSubclassOf<class ACameraActor> DeathCameraClass;
	
	UPROPERTY(Transient)
	TObjectPtr<class ACameraActor> DeathCamera;
	
	bool bIsUsingDeathCamera = false;
	
	// 설정했지만 DeathCamera가 실행 중이라 리턴된 뷰타겟 저장하여 DeathCamera 종료 시 설정할 수 있도록 함 
	TWeakObjectPtr<AActor> PendingViewTarget;
	FViewTargetTransitionParams PendingViewTargetTransitionParams;
	
	// --------------------------------------
	// 어빌리티 실행 및 실패 관련
	// 플레이어의 액티브 스킬 어빌리티 사용은 타겟 설정과 사용 범위 밖에 있을 경우 자동 이동 기능을 위해 PlayerController를 거쳐서 사용한다.
	// URPlayerController_Ability.cpp에 작성한다.
	// --------------------------------------
	
public:
	/**
	 * 타겟을 지정하고 액티브 스킬 어빌리티 활성화를 시도한다, 사용 범위 밖에 있을 경우 자동 이동 후 활성화하도록 설정한다. 
	 * @return 어빌리티 활성화 여부를 리턴하고, 사용 범위 밖에 있어 자동 이동 후 활성화하도록 설정해도 true를 리턴한다.
	 */
	bool TryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate);
	
	bool IsTargetInRange(const class UURGameplayAbility_ActiveAbility* ActiveAbility, const AActor* Actor) const;
	
	UFUNCTION(Client, Reliable)
	void ClientAbilityFailed(struct FGameplayAbilitySpecHandle SpecHandle, int16 ActivationPredictionKey, const struct FGameplayTagContainer& FailureReason);
	
	void LocalAbilityFailed(struct FGameplayAbilitySpecHandle SpecHandle,  int16 ActivationPredictionKey, const struct FGameplayTagContainer& FailureReason);
	
protected:
	UAbilitySystemComponent* GetAbilitySystemComponent() const;
	
	/** 대상 액터를 포함한 이벤트 데이터를 통해 어빌리티를 활성화 시킨다. */
	bool TriggerAbility(FGameplayAbilitySpecHandle AbilityToActivate, AActor* TargetActor) const;
	
	/** PendingAbility를 설정 후 자동 이동을 활성화 */
	bool SetPendingAbilityAndMoveToTarget(FGameplayAbilitySpecHandle AbilityToActivate, AActor* TargetActor);
	
	/** PendingAbility를 설정하고 이동을 시작할 때, 1회성으로 서버에서 검증한다. */
	UFUNCTION(Server, Reliable)
	void ServerSetPendingAbility(int32 RequestID, const FGameplayAbilitySpecHandle AbilityToActivate, AActor* TargetActor);
	
	/** 서버에서의 유효성 검사를 실패하면 호출되어 현재 PendingAbility를 제거한다. */
	UFUNCTION(Client, Reliable)
	void ClientKillPendingAbility(const FGameplayAbilitySpecHandle AbilityToKill, const FGameplayTagContainer Reason);
	
	/** PendingAbility의 사용 거리를 검사하고 범위내에 있을 경우 활성화한다. */
	void CheckPendingAbilityToActivate();
	
	/** PendingAbility가 있을 경우 자동 이동을 종료하고 비운다. */
	void ClearPendingAbility();
	
	UFUNCTION(Server, Reliable)
	void ServerClearPendingAbility(int32 RequestID);
	
	/** MoveRequest 종료되었을 때 호출되어 PendingAbility 실행 시도 */
	void HandleMoveRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& PathFollowingResult);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURPathFollowingComponent> PathFollowingComponent;
	
	/** 자동 이동 완료 후 사용할 어빌리티 정보 */
	FURPendingAbility PendingAbility;
	
	/** 어빌리티 실행 실패 토스트 중복 표시 방지 위해 저장한 실패 정보 */
	TArray<struct FURAbilityFailedRecord> FailedAbilityRecords;
};
