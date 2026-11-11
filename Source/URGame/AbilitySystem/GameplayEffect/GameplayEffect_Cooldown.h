// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffect_Cooldown.generated.h"

/** 어빌리티별 SetByCaller 시간과 동적 부여 태그를 사용하는 공통 쿨다운 GE */
UCLASS()
class URGAME_API UGameplayEffect_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()
	
public:
	UGameplayEffect_Cooldown();
};
