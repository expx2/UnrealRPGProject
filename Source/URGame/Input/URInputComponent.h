// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "URInputComponent.generated.h"

namespace EAbilityInputBinds
{
	enum Type : int;
}

struct FNativeInputMapping;
/**
 * 
 */
UCLASS()
class URGAME_API UURInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	template<class UserClass, typename FuncType>
	void BindNativeAction(const UInputAction* InputAction, const ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func);
	
	template<class UserClass, typename TriggeredFuncType, typename ReleasedFuncType>
	void BindAbilityAction(const UInputAction* InputAction, UserClass* Object, TriggeredFuncType TriggeredFunc, ReleasedFuncType ReleasedFunc, const EAbilityInputBinds::Type InputBinding);

	void ResetBindings();

protected:
	TArray<uint32> BindHandles;
};

template <class UserClass, typename FuncType>
void UURInputComponent::BindNativeAction(
	const UInputAction* InputAction, 
	const ETriggerEvent TriggerEvent,
	UserClass* Object, 
	FuncType Func)
{
	check(InputAction);
	
	BindHandles.Add(BindAction(InputAction, TriggerEvent, Object, Func).GetHandle());
}

template<class UserClass, typename TriggeredFuncType, typename ReleasedFuncType>
void UURInputComponent::BindAbilityAction(
	const UInputAction* InputAction, 
	UserClass* Object, 
	TriggeredFuncType TriggeredFunc, 
	ReleasedFuncType ReleasedFunc,
	const EAbilityInputBinds::Type InputBinding)
{
	check(InputAction);
	
	if (TriggeredFunc)
	{
		BindHandles.Add(BindAction(InputAction, ETriggerEvent::Triggered, Object, TriggeredFunc, InputBinding).GetHandle());
	}
	if (ReleasedFunc)
	{
		BindHandles.Add(BindAction(InputAction, ETriggerEvent::Canceled, Object, ReleasedFunc, InputBinding).GetHandle());
		BindHandles.Add(BindAction(InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, InputBinding).GetHandle());
	}
};
