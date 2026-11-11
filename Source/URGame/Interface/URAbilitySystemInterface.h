// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "URAbilitySystemInterface.generated.h"

class UURAbilitySystemComponent;

/** UAbilitySystemInterface을 상속 받아 기본 ASC대신, URAbilitySystemComponent를 리턴하는 함수 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UURAbilitySystemInterface : public UAbilitySystemInterface
{
	GENERATED_BODY()
};

class URGAME_API IURAbilitySystemInterface : public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UURAbilitySystemComponent* GetURAbilitySystemComponent() const = 0;
};
