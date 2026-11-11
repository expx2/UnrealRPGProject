// Written by EXPx2


#include "Input/URInputMappingSet.h"

#include "UObject/ConstructorHelpers.h"

UURInputMappingSet::UURInputMappingSet()
{
	// AbilityInputMappings 자동 설정
	const UEnum* AbilityInputBindsEnum = StaticEnum<EAbilityInputBinds::Type>();
	check(AbilityInputBindsEnum);

	// 마지막 항목은 UHT가 자동으로 추가한 _MAX 값이므로 제외한다.
	const int32 AbilityInputBindCount = AbilityInputBindsEnum->NumEnums() - 1;
	AbilityInputMappings.Reserve(AbilityInputBindCount);

	// None 제외
	for (int32 EnumIndex = 1; EnumIndex < AbilityInputBindCount; ++EnumIndex)
	{
		const int64 EnumValue = AbilityInputBindsEnum->GetValueByIndex(EnumIndex);
		const FString EnumName = AbilityInputBindsEnum->GetNameStringByIndex(EnumIndex);
		const FString InputActionPath = FString::Printf(
			TEXT("/Game/_ProjectUR/Inputs/InputActions/IA_%s.IA_%s"),
			*EnumName,
			*EnumName);

		const ConstructorHelpers::FObjectFinder<UInputAction> InputActionRef(*InputActionPath);
		if (InputActionRef.Succeeded())
		{
			FAbilityInputMapping& InputMapping = AbilityInputMappings.AddDefaulted_GetRef();
			InputMapping.Command = static_cast<EAbilityInputBinds::Type>(EnumValue);
			InputMapping.InputAction = InputActionRef.Object;
		}
	}
}
