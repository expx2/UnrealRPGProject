// Written by EXPx2

/**
 * GameplayAbilitySet 참고하여 작성함
 */

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "URInputMappingSet.generated.h"

/**
 * 기본 조작 입력에 대한 바인딩
 */
UENUM(BlueprintType)
namespace ENativeInputBinds
{
	enum Type : int
	{
		None		UMETA(Hidden),
		MouseLook	UMETA(DisplayName="Look(Mouse)"),
		StickLook	UMETA(DisplayName="Look(Stick)"),
		Move		UMETA(DisplayName="Move"),
		WheelZoom	UMETA(DisplayName="Zoom(Wheel)"),
	};
}

/**
 *	기본 조작 바인딩 정보
 */
USTRUCT()
struct FNativeInputMapping
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="UR")
	TEnumAsByte<ENativeInputBinds::Type> Command = ENativeInputBinds::Type::None;

	UPROPERTY(EditAnywhere, Category="UR")
	TObjectPtr<UInputAction> InputAction;
};

/**
 * 어빌리티 실행 입력에 대한 바인딩
 */
UENUM(BlueprintType)
namespace EAbilityInputBinds
{
	enum Type : int
	{
		None					UMETA(DisplayName="None"),
		Ability1				UMETA(DisplayName = "Ability1 (LMB)"),
		Ability2				UMETA(DisplayName = "Ability2 (Q)"),
		Ability3				UMETA(DisplayName = "Ability3 (E)"),
		Ability4				UMETA(DisplayName = "Ability4 (R)"),
		Ability5				UMETA(DisplayName = "Ability5 (X)"),
		Ability6				UMETA(DisplayName = "Ability6 (Space)"),
		Ability7				UMETA(DisplayName = "Ability7 (Tab)"),
		Ability8				UMETA(DisplayName = "Ability8 (RMB)"),
	};
}

/**
 * 어빌리티 바인딩 정보
 */
USTRUCT()
struct FAbilityInputMapping
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="UR")
	TEnumAsByte<EAbilityInputBinds::Type> Command = EAbilityInputBinds::Type::Ability1;

	UPROPERTY(EditAnywhere, Category="UR")
	TObjectPtr<UInputAction> InputAction;
};


/**
 * 입력 바인딩과 InputAction을 매핑하는 데이터셋
 */
UCLASS()
class URGAME_API UURInputMappingSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UURInputMappingSet();
	
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TObjectPtr<class UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TArray<FNativeInputMapping> NativeInputMappings;
	
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TArray<FAbilityInputMapping> AbilityInputMappings;
};
