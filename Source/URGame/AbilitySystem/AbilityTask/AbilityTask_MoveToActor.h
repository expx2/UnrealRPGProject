// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AITypes.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_MoveToActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMoveToTargetDelegate);

/**
 * 액터에게 자동 이동한다
 */
UCLASS()
class URGAME_API UAbilityTask_MoveToActor : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_MoveToActor* MoveToActor(UGameplayAbility* OwningAbility, AActor* TargetActor);
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	/** 성공적으로 도착하면 호출 */
	UPROPERTY(BlueprintAssignable)
	FMoveToTargetDelegate OnReached;

	/** 이동 실패하고 종료되면 호출 */
	UPROPERTY(BlueprintAssignable)
	FMoveToTargetDelegate OnFailed;
	
	/** 타겟이 파괴되거나 유저 입력으로 이동이 중지되면 호출 */
	UPROPERTY(BlueprintAssignable)
	FMoveToTargetDelegate OnCancelled;

protected:
	virtual void Activate() override;

	/**
	 * RequestID 보장 위해 UAIBlueprintHelperLibrary의 함수 복사해서 사용함.
	 * @see UAIBlueprintHelperLibrary::SimpleMoveToActor 
	 */
	void SimpleMoveToActor(AController* Controller, const AActor* Goal, FAIRequestID& OutRequestID);
	
	void HandleMoveRequestFinished(FAIRequestID RequestID, const struct FPathFollowingResult& Result);

	UFUNCTION()
	void HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	
	/** 이동 중 플레이어의 이동 입력이 변경되면 태스크 종료 */
	UFUNCTION()
	void HandlePlayerMovementInputChanged(const FVector2D& OldInputValue, const FVector2D& NewInputValue);
	
	void FinishTask(const bool bIsReached, const bool bIsCancelled);

	TWeakObjectPtr<AActor> TargetActorPtr;
	
	TWeakObjectPtr<class UPathFollowingComponent> PathFollowingComponentPtr;
	
	FAIRequestID MoveRequestId;
	
	FDelegateHandle MoveRequestFinishedDelegateHandle;
	
	// UURCharacterInputComponent::UpdateMovementInput 함수 내부 구현 참고
	// TWeakObjectPtr<class UURCharacterInputComponent> CharacterInputComponentPtr; 
	
	/** 중복 종료 막기 위한 값 */ 
	bool bIsFinished = false; 
};
