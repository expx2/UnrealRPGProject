# 플레이어

## 입력

입력은 Lyra 프로젝트를 참고해 작성했다.

[`UURCharacterInputComponent`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/Character/URCharacterInputComponent.h) 독립된 컴포넌트에서, InputComponent에 입력을 바인딩하고 그에 따른 행동 구현을 담당한다.

#### 어빌리티 바인딩 흐름
1. `EAbilityInputBinds`로 어빌리티 실행 입력을 정의한다. 
2. `UURCharacterInputComponent`에서 `UURInputMappingSet`을 통해 InputAction에 따른 `EAbilityInputBinds`를 바인딩 한다.
3. `UURGameplayAbilitySet`에서 `AbilitySystemComponent`에 어빌리티를 등록할 때 `GameplayAbilitySpec`의 `InputID`에 `EAbilityInputBinds`를 연결한다.

한 개의 `InputID`에 다수의 어빌리티가 등록될 수 있으며, 
입력 시 어빌리티 선택 과정을 통해 선택된 어빌리티를 활성화 시도한다.

## 어빌리티 선택

시스템을 참고한 `아이온2`에서는 한 개의 스킬 슬롯에 여러개의 스킬을 등록하여 순서대로 사용할 수 있다.

이를 구현하기 위해 하나의 입력에 여러 어빌리티의 등록을 허용하며, 입력 시 해당 입력에 등록된 어빌리티를 전부 활성화 시도하지 않고
`UURAbilitySystemComponent::ResolveCurrentAbilityForInput`를 통해 하나의 어빌리티를 선택하여 활성화 시도한다.

실제 사용 가능 조건과 별도로 선택될 수 있는지 검사하는 함수 `UURGameplayAbility_ActiveAbility::DoesAbilitySatisfyTagRequirementsToSelect`를 구현해
현재 **조건을 만족하는 어빌리티 중 우선순위가 가장 높은 하나**를 선택한다.

> 이는 스킬 슬롯([`UURGameHUDAbilitySlotWidget`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/HUD/URGameHUDAbilitySlotWidget.h))에서 현재 표시할 스킬을 선택하는데에도 사용된다.

실제 사용 가능 조건과 별도의 조건을 가지는 이유는 실제 사용이 불가하더라도 스킬 슬롯에 표시하고, 활성화 시도 시 이유에 따른 메세지를 표시하기 위함이다. (추가로 참고한 시스템을 재현하기 위함이다.)

> 선택 예시)
우선순위가 A > B일 때,
A의 사용 마나는 부족하고
B의 사용 마나는 충분하더라도 
선택되는 어빌리티는 A 어빌리티이다.

## AbilitySystemComponent 관리

플레이어의 `AbilitySystemComponent`는 `PlayerState(AURGamePlayerState)`에서 관리한다.

> 적 캐릭터는 캐릭터가 `AbilitySystemComponent`를 소유하여 캐릭터와 같은 수명을 가진다.

이를 통해 플레이어 캐릭터가 사망하여 파괴되어도, 사망 상태를 유지하고 리스폰 시에도 쿨다운 등의 상태를 유지할 수 있다.

부모 클래스의 어빌리티 시스템 설정 함수 `AURASCCharacter::SetAbilitySystemComponent`를 오버라이드하여, `PlayerState`에서 `AbilitySystemComponent`를 가져와 `ActorInfo`를 설정한다.
