// Written by EXPx2

/**
 * GameplayAbilitySet 참고하여 작성함
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Input/URInputMappingSet.h"
#include "URGameplayAbilitySet.generated.h"

class UGameplayAbility;
struct FGameplayAbilitySpecHandle;

namespace EAbilityInputBinds
{
	enum Type : int;
}

/**
 * 어빌리티 바인딩 정보
 */
USTRUCT()
struct FAbilityBindInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="UR")
	TEnumAsByte<EAbilityInputBinds::Type> Command = EAbilityInputBinds::Type::Ability1;

	UPROPERTY(EditAnywhere, Category="UR")
	TSubclassOf<UGameplayAbility> GameplayAbilityClass;
};


/**
 * 입력 바인딩과 어빌리티를 매핑하고 부여하는 데이터셋 
 */
UCLASS()
class URGAME_API UURGameplayAbilitySet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UURGameplayAbilitySet();
	
	UPROPERTY(EditAnywhere, Category = AbilitySet)
	TArray<FAbilityBindInfo> Abilities;

	void GiveAbilities(class UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAbilitySpecHandle>& OutGrantedSpecHandles);
};
