// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "URCharacterInputComponent.generated.h"

namespace EAbilityInputBinds
{
	enum Type : int;
}

struct FInputActionValue;
class UURInputComponent;

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMovementInputChangedDelegate, const FVector2D&, OldInputValue, const FVector2D&, NewInputValue);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URGAME_API UURCharacterInputComponent : public UPawnComponent
{
	GENERATED_BODY()

public:	
	UURCharacterInputComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void SetupInputComponent(UURInputComponent* InInputComponent);
	
	void Cleanup();
	
	/*
	 * 자동 이동 취소를 델리게이트 사용에서 PathFollowingComp를 찾아 Abort하도록 변경함
	 * UpdateMovementInput 함수 내부 구현 참고
	 */
	// UPROPERTY(BlueprintAssignable, Category="UR")
	// FMovementInputChangedDelegate OnMovementInputChanged;
	
protected:
	void TryAddMappingContext(bool& IsSuccess);
	
	void OnInputLookMouse(const FInputActionValue& InputActionValue);
	
	void OnInputLookStick(const FInputActionValue& InputActionValue);
	
	void OnInputMove(const FInputActionValue& InputActionValue);
	
	void OnInputMoveEnd(const FInputActionValue& InputActionValue);
	
	void OnInputZoomWheel(const FInputActionValue& InputActionValue);
	
	/** ASC의 AbilityInputTagTriggered 호출 */
	void OnAbilityInputTriggered(const EAbilityInputBinds::Type InputBinding);
	
	/** ASC의 AbilityInputTagReleased 호출 */
	void OnAbilityInputReleased(const EAbilityInputBinds::Type InputBinding);
	
	void UpdateMovementInput(const FVector2D& NewInputValue);
	
	class UPathFollowingComponent* GetPathFollowingComponent() const;
	
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TObjectPtr<class UURInputMappingSet> CharacterInputSet;
	
	TWeakObjectPtr<UURInputComponent> BoundInputComponentPtr;
	TWeakObjectPtr<class AURGamePlayerController> BoundPlayerControllerPtr;
	
	// 최근 이동 입력 값. 이동 입력 변화 감지 위해 저장함
	FVector2D CurrentMovementInputValue = FVector2D::ZeroVector;
};
