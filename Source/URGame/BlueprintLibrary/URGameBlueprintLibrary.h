// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "URGameBlueprintLibrary.generated.h"

enum class EPopupType : uint8;
/**
 * 
 */
UCLASS()
class URGAME_API UURGameBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category="UR|Game")
	static FVector GetFeetLocation(const AActor* Actor);
	
	UFUNCTION(BlueprintPure, Category="UR|Game")
	static void GetAimTargetLocation(const AActor* Actor, FVector& ResultLocation);
	
	/**
	 * 컨트롤러에서 UPathFollowingComponent를 찾고 없을 경우 임시로 생성한다.
	 * @see AIBlueprintHelperLibrary::InitNavigationControl 
	 */
	UFUNCTION(BlueprintPure, Category="UR|Game")
	static class UPathFollowingComponent* GetPathFollowingComponent(AController* Controller, const bool bCreateIfDontHave);
	
	UFUNCTION(BlueprintPure, Category="UR|Game")
	static bool IsActorInRangeUsingPathFollowingComponent(const UPathFollowingComponent* PathFollowingComponent, const AActor* Actor, float Range);
	
	UFUNCTION(BlueprintPure, Category="UR|Game")
	static bool IsDamageableActor(const AActor* Actor);
	
	UFUNCTION(BlueprintCallable, Category="UR|Game")
	static bool TriggerAbilityWithTarget(class UAbilitySystemComponent* AbilitySystemComponent, struct FGameplayAbilitySpecHandle AbilityToTrigger, AActor* TargetActor);
};

namespace URUtilsLibrary
{
	template <typename T>
	TArray<T*> ToRawArray(const TArray<TWeakObjectPtr<T>>& In)
	{
		TArray<T*> Out;
		Out.Reserve(In.Num());

		for (const TWeakObjectPtr<T>& Weak : In)
		{
			if (T* Ptr = Weak.Get())
			{
				Out.Add(Ptr);
			}
		}
		return Out;
	}
}
