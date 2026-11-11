# 전투 시스템

전투 시스템은 [`액티브 어빌리티 - 액션`](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/ActiveAbility.md#액션)에서 구성한 정보를 바탕으로
대상 검출 → 효과 적용 → 피해 계산 → 피격 처리의 흐름을 가진다.

범위 검사는 [`UURCollisionBlueprintLibrary`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/BlueprintLibrary/URCollisionBlueprintLibrary.h), 대상 선정 및 효과 적용은 [`UURCombatBlueprintLibrary`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/BlueprintLibrary/URCombatBlueprintLibrary.h)에 분리하여 재사용성과 유지보수성을 높였다.

실제 타격 판정과 피해 효과 적용은 항상 **`Authority`가 있는 서버**에서 진행하며, 
클라이언트는 복제된 체력과 `GameplayCue`, 클라이언트 RPC를 통해 결과를 표시한다.

지연 보정은 [`액티브 어빌리티 - 네트워크 지연과 프레임 지연 보정`](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/ActiveAbility.md#네트워크-지연과-프레임-지연-보정) 문서에서 다룬다

## Damageable Interface

[`IURDamageableInterface`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/Interface/URDamageableInterface.h) 인터페이스를 구현한 액터만을 전투 및 타격 대상으로 사용한다.

`AbilitySystemComponent`를 가지고 있는 액터에겐 `GameplayEffect`를 통해 타격을 진행하고, 
프랍과 같이 `AbilitySystemComponen`t가 없는 액터는 `IURDamageableInterface::NativeTakeDamage`를 구현해 피격 처리를 진행한다.

진영을 나타내는 `EURFaction`을 가지며, 
스킬 사용 시 대상 진영은 `UURCombatBlueprintLibrary::GetRelativeTargetFaction`을 통해 공격자 기준으로 해석한다.
> 현재 구현은 공격자가 `Enemy`일 때, `Ally`와 `Enemy`를 서로 바꿔, 적 캐릭터의 공격 스킬도 대상을 `Enemy`로 설정하도록 해 사용성을 높였다. 

## Hit Detection

언리얼에서 제공하는 충돌 쿼리와 함께 추가 조건 검사를 통해 원, 부채꼴, 도넛, 도넛 부채꼴, 사각형의 범위 검사를 제공한다.

범위 데이터는 `FURColliderShape`를 상속받으며, 사용처에서 `TInstancedStruct`로 모양과 값을 설정한다.

### 범위 검사 방법

먼저 OverlapMultiByChannel로 범위 주변의 후보를 얻고, 각 모양에 필요한 추가 조건을 검사한다.

충돌 쿼리와 마찬가지로 캐릭터의 원점 뿐만 아니라 캡슐이 범위에 걸쳐 있으면 포함하도록 하여 일관성을 유지한다. 

[`UURCollisionBlueprintLibrary::GetDamageableActors`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/BlueprintLibrary/URCollisionBlueprintLibrary.cpp#L152)

|모양| 추가 검사                             |
|--|-----------------------------------|
|부채꼴| 원점이 부채꼴 내부에 있거나, 각 변과 충돌할 만큼 가까운지 |
|도넛|도넛 내부에 온전히 들어가 있는지|
|도넛 부채꼴| 부채꼴과 도넛 검사 조합|
|원, 사각형 |없음|

### 실린더 형태의 후보 지정

OverlapMultiByChannel에서 사각형을 제외한 모양들은 1차적으로 Radius 내에 있는 캐릭터들을 후보로 얻는다.

여기서 Sphere로 검사하면 같은 높이에 있는 캐릭터라도 공격 중심에서 멀어지면 검출되지 않을 수 있다.

이를 해결하기 위해 허용 높이를 따로 지정하여, Capsule로 Overlap을 진행한 뒤, 추가 높이 검사를 통해 위아래 반구를 제외한 **실린더 형태의 범위로 검사**하도록 했다.

높이를 검사할 때도 대상 원점만 비교하지 않고 대상이 범위에 일부라도 걸치는지 확인한다.

<center>

<img src="https://github.com/user-attachments/assets/2ef39119-c808-4e4e-8c4c-99fa8549e036" height="250"/>

참고 이미지

</center>

## 타격 대상 수 제한과 우선순위

범위형 공격에는 최대 타격 대상 수를 지정할 수 있다.

검출된 대상이 최대 대상 수보다 많을 경우 우선순위를 두어 처리한다.

[`UURCombatBlueprintLibrary::ResolveTargets`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/BlueprintLibrary/URCombatBlueprintLibrary.cpp#L52)

| 우선순위 | 대상 | 의도                           |
|---|---|------------------------------|
| 1 | 스킬 사용 시 지정한 타겟 | 사용자가 지정한 대상을 우선 타격한다.        |
| 2 | 이전 타격 회차에서 처리한 대상 | 다단히트 중 타격 대상이 매번 바뀌지 않도록 한다. |
| 3 | 그 외 범위 내 대상 | 남은 타격 수를 채운다.                |

> 우선권이 있어도 현재 회차의 범위 검사에 검출되어야 타격한다.

## 타격 효과 적용

액티브 스킬의 액션에서 `FURHitEffect` 배열을 통해 대미지를 포함한 다른 부가 효과를 지정한다.

`SetByCaller`를 통해 필요한 값을 전달해 하나의 구조로 다양한 값을 지정 가능하게 한다.

<center>

<img src="https://github.com/user-attachments/assets/908f3cad-3dd4-4ee7-887f-3b9a5d266a97" height="250"/>

설정 예시

</center>

검출된 대상에 대한 효과 적용은 [`UURCombatBlueprintLibrary::Hit`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/BlueprintLibrary/URCombatBlueprintLibrary.cpp#L167)를 공통 진입점으로 사용한다.

### AbilitySystemComponent가 있는 대상

`FURHitEffect`에서 지정한 `SetByCaller`값을 담아 GE를 적용한다.

`FURHitGameplayEffectContext`로 기본 Context를 확장해 치명타 여부와 피격 GC 태그를 전달한다.

Hit에선 대상마다 원본 Context를 복제하여 크리티컬 여부와 같이 개별로 기록되어야 할 값이 공유되지 않도록 했다.

하나의 효과가 대상을 사망시키는 등 상태를 바꿀 수 있으므로, 각 효과를 적용하기 전 피해 수신 가능 여부를 다시 확인한다.

> 현재는 유효한 타격 대상일 경우 GE를 전달하고, 별도의 명중이나 회피 판정은 없다.

### AbilitySystemComponent가 없는 대상

대미지를 주는 `UGameplayEffect_Damage`가 있을 경우 대미지 계수만 추출해 `IURDamageableInterface::NativeTakeDamage`로 대미지를 전달한다.

## 피해 계산

대미지 전달은 `UGameplayEffect_Damage`를 통해 전달하며, `CalculationClass`에 [`UURDamageExecutionCalculation`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/AbilitySystem/GameplayEffect/Calculation/URDamageExecutionCalculation.h)을 지정해 피해량을 계산한다.

공격의 대미지 계수는 `SetByCaller.DamageMultiplier`로 전달한다. 따라서 공격은 계수와 효과 데이터만 설정하고, 공격력 및 치명타, 방어력 계산은 공통 계산식을 사용한다.

계산 결과는 체력에 직접 쓰지 않고, `UURHealthSet`의 메타 어트리뷰트 `DamageTaken`에 더해 체력 반영 단계로 전달한다.

### 계산에 사용하는 어트리뷰트

| 어트리뷰트              | 출처                        | 역할                  |
|-------------------------|---------------------------|---------------------|
| `AttackPower`           | 공격자의 `UURCombatSet` | 기본 공격력              |
| `AdditionalAttackPower` | 공격자의 `UURCombatSet` | 무작위로 더할 추가 공격력의 상한  |
| `CriticalRate`          | 공격자의 `UURCombatSet` | 치명타 확률              |
| `CriticalDamage`        | 공격자의 `UURCombatSet` | 치명타 발생 시 공격력에 곱할 배율 |
| `Defense`               | 대상의 `UURCombatSet` | 대미지에서 차감할 방어력       |
| `DamageMultiplier`      | GE의 `SetByCaller`         | 공격의 대미지 계수. 미지정 시 1 |

### 계산 식

(공격력 + Random(0, 추가 공격력)) * 치명타 배율 * 대미지 계수 - 대상의 방어력

> 치명타 여부는 `FURHitGameplayEffectContext`에 기록하여 이후, 피격 피드백에서 사용하도록 한다.

## 피격 처리

피격은 권한 있는 서버에서 진행되며 `GameplayCue`와 클라이언트 RPC를 통해 클라이언트에 전달된다. 별도의 로컬 예측을 사용하진 않는다.

### 체력 반영

Lyra 프로젝트를 참고하여 체력 수치를 관리하는 `UURHealthSet`과 피격과 체력 상태 변경에 따른 이벤트를 전달하는 `UURHealthComponent`를 분리했다.

GE를 적용한 이후 호출되는 `UURHealthSet::PostGameplayEffectExecute`에서 메타 어트리뷰트인 `DamageTaken` 값이 변경되었을 경우 체력을 조정하고 `DamageTaken` 값을 초기화한다. 
이는 권한 있는 서버에서 진행되며 클라이언트에서는 변경된 체력값을 복제되어 받는다.

### GameplayCue 피드백

피격 시 발생하는 VFX, SFX 등 처리를 위해 `FURHitGameplayEffectContext`에서 `HitImpactCueTag`를 가져와 GC를 실행한다.

### RPC 피드백

[`AURGamePlayerController::ClientDamageTakenFeedback`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/Player/URGamePlayerController.cpp#L195)

모든 플레이어가 아닌 로컬 플레이어가 타격/피격 했을 경우에만 발생하는 피드백을 처리하기 위해 GameplayCue가 아닌 별도의 클라이언트 RPC를 사용했다.

대미지 표시와, 이름표의 상태를 갱신한다.

> 다른 플레이어 혹은 파티원이 공격한 대미지도 표시하는 게임들이 있는데, 그럴 경우 GC를 사용하고 이후에 Instigator에 따라 행동을 수행하는 식으로 개발 가능할 것으로 예상함.

## 서버 판정 책임

타격 처리 공통 진입점인 [`UURCombatBlueprintLibrary::Hit`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/BlueprintLibrary/URCombatBlueprintLibrary.cpp#L167)에선 
Authority를 검사하여 권한 있는 서버에서만 진행한다.

`GameplayEffect`는 권한 있는 서버에서만 적용한다.

피드백 처리 시 권한 있는 서버에서만 `GameplayCue` 및 클라이언트 RPC를 전달한다.
