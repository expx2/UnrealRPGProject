# ProjectUR

언리얼 엔진 5.7로 개발한 멀티 플레이 RPG 게임입니다.

GAS(Gameplay Ability System)를 사용한 스킬 및 효과, 상태 관리 시스템을 가지고 있습니다.

데디케이티드 서버 환경에서의 전투 로직과 동기화, 지연 보정을 구현했습니다.

## 플레이 영상

[<img src="https://img.youtube.com/vi/U3yrNprBXRI/sddefault.jpg" width="480">](https://youtu.be/U3yrNprBXRI)

<i>
이미지를 클릭하면 유튜브로 연결됩니다.

서버와 클라이언트 패키징 후,
서버는 AWS에서 구동하고 두 PC에서 플레이했습니다.

</i>

#### 게임 흐름
1. 로비 화면 - 캐릭터 선택 후 게임 입장
2. 자코 전투
3. 보스 전투
4. 승리 후 로비로 복귀

## 상세 설명 문서

[`GameplayArchitecture.md`](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/GameplayArchitecture.md): 게임 플레이에서의 구조를 전반적으로 설명합니다.

[`Combat.md`](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/Combat.md): 전투 시스템에 대해 설명합니다.

[`ActiveAbility.md`](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/ActiveAbility.md): 데이터 기반의 액션 조합형 액티브 스킬에 대해 설명합니다.

## 주요 기능

### 전투 시스템

- 원, 부채꼴, 도넛, 도넛 부채꼴, 사각형 범위 충돌 판정
- 멀티 플레이 환경에서 서버 책임의 전투 판정과 결과 동기화, 클라이언트 피드백

[상세 설명 문서 - Combat.md](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/Combat.md)

### GAS (Gameplay Ability System)

- 데이터 기반의 액션 조합형 액티브 스킬 GameplayAbility [상세 설명 문서 - ActiveAbility.md](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/ActiveAbility.md)
- 대미지 및 상태 효과 GameplayEffect
- Attribute 캐릭터 스탯
- GameplayCue를 이용한 연출

### 캐릭터

- 플레이어 캐릭터

  - 입력 시스템과 PlayerState에서의 ASC 관리 [상세 설명 문서 - GameplayArchitecture.md](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/GameplayArchitecture.md)

- 적 캐릭터
  - [AI 행동 트리](https://github.com/user-attachments/assets/198ca6ce-403c-4615-b2a5-3fb3214c9fe3)
    - 순찰: 일정 거리를 순찰하며 선공 적일 경우 범위 내 적을 검사한다.
    - 전투
      - 추적: 보유한 액티브 어빌리티 중 사용 가능 거리가 가장 짧은 액티브 어빌리티의 사용 가능 거리까지 이동하며 사용 가능한 어빌리티가 있는지 검사한다.
      - 액티브 어빌리티 활성화: 우선순위대로 검사하여 사용 가능한 어빌리티를 활성화한다.
        > 어빌리티 활성화 순서를 설정하는 `UAIAbilityOrderSet`가 있을 경우 순서대로 사용한다.
    - 귀환: 본인이 초기 위치에서 일정 이상 거리를 벗어나거나 전투 중 대상이 없어지면 상태를 초기화하고 초기 위치로 이동한다.
  
  - 어그로 관리
  
    적 캐릭터는 [`URAggroComponent`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/Character/URAggroComponent.h)를 가져서 피격 시 공격자와 어그로 수치를 기록한다.
  
    AI 행동 트리에서 어그로 수치가 가장 높은 상대를 대상으로 액티브 스킬 활성화를 시도한다.

    > 향후 개선 과제  
    > 게임의 재미를 위해 다음 기능을 추가한다.
    > - 도발 스킬
    > - (보스) 일정 시간(행동)마다 랜덤하게 타겟 재지정 

- Template 사용 AnimBP

### 게임 플레이

- 타 플레이어 관전 및 전멸 시 리스폰 [상세 설명 문서 - GameplayArchitecture.md](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/GameplayArchitecture.md)

- 레벨 시퀀스

### UI (UMG)

- UI 관리 구조

  로컬 플레이어 서브 시스템 [`UURUISubsystem`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/URUISubsystem.h)를 사용해 팝업 위젯 생성/제거, 공통 UI 기능 등 UI 매니저로서의 역할을 담당한다.

  > UI 수가 많지 않고 팝업 연관 관계가 없어 최소한의 기능만 개발함.

  `RootWidget`을 두어 공통 UI(토스트 메세지 등)와 UI 레이어를 통합 관리한다.

  `AHUD`를 해당 레벨의 전용 UI 매니저 역할로 사용해 공통 UI와 해당 레벨 전용 UI의 책임을 분리했다. [`AURGameHUD`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/HUD/URGameHUD.h)

- 스킬 슬롯

  [`UURGameHUDAbilitySlotWidget`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/HUD/URGameHUDAbilitySlotWidget.h) 게임 레벨 HUD에 표시되는 스킬 슬롯

  [`어빌리티 선택`](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/ActiveAbility.md#어빌리티-선택)을 통해 현재 입력에 활성화 시도 할 어빌리티 정보 표시

  쿨다운 태그를 부여하는 GE 중 가장 늦게 종료되는 GE를 구해 남은 쿨다운 시간 표시

  > |GE|지속 시간|남은 시간|
  > |-|-|-|
  > |A|30|3|
  > |B|10|5|
  >
  > => B가 선택되어 50%의 쿨다운 표시

  연계에 필요한 태그를 부여한 GE 중 태그별로 가장 늦게 종료되는 GE를 구하고, 그 중에서 가장 먼저 종료되는 GE를 구해 연계 가능 시간 표시

  > | GE | 태그 | 지속 시간 | 남은 시간 |
  > |----|----|-------|-------|
  > | A  | X  | 40    | 10    |
  > | B  | X  | 20    | 5     |
  > | B  | Y  | 20    | 5     |
  > | C  | Y  | 30    | 20    |
  >
  > X 태그를 부여하는 GE 중 가장 늦게 종료 되는 A, Y 태그를 부여하는 GE 중 가장 늦게 종료되는 C  
  > 그 중 가장 먼저 종료되는 A  
  > => A가 선택되어 25%의 연계 가능 시간 표시
  
- WidgetComponent [`UURActorDisplayWidgetComponent`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/ActorDisplay/URActorDisplayWidgetComponent.h)
- 대미지 플로터와 풀링 [`UURFloatingDamageLayerWidget`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/FloatingDamage/URFloatingDamageLayerWidget.h)
- BP/C++ 구현


### MISC

- 우선순위 기반 상태 제어 템플릿 [`TURValueByStateResolver`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/Helpers/URValueByStateResolver.h)

  여러 소스에서 등록한 값을 관리하고 우선순위가 가장 높은 값을 반환하는 클래스로 템플릿을 사용해 각종 상태 제어에 사용 가능하다. 
  
  프로젝트에선 캐릭터의 회전 관리에 사용한다.

- Receiver Interface

  `IURActorReceiverInterface`와 `IURActorReceiverInterface`를 사용해 설정/초기화 함수 구현

  [`UURWidgetBlueprintLibrary`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/BlueprintLibrary/URWidgetBlueprintLibrary.h)에서 하위 위젯으로 전달하는 함수 제공하여 위젯 설정 간소화

## 포스트모템

언리얼 엔진의 GAS와 네트워크 기능을 활용하여, RPG 전투에서 서버 중심의 전투 판정을 구현하는 것을 목표로 프로젝트를 진행했습니다.

평소 MMORPG 게임을 즐겨하는데 조작성과 액션성이 강한 `아이온2`의 전투 시스템을 참고하여, 빠른 조작 반응성을 가진 전투 시스템을 구현했습니다.

결과적으로 서버와 클라이언트의 책임을 구분하고, 클라이언트 예측과 네트워크 지연 보정을 구현하며
클라이언트의 반응성과 서버 판정의 신뢰성 사이에서 균형을 잡기 위한 멀티 플레이 구조 설계 방법을 익혔습니다.

#### 만족스러운 점

이전에 담당한 프로젝트에서 데이터 기반의 스킬 시스템을 개발한 경험을 바탕으로,
언리얼의 GAS를 활용해 더욱 확장성과 재사용성이 높은 구조로 구성할 수 있었습니다.

그 결과 캐릭터를 추가하며 새로운 액티브 어빌리티 5종을 제작하면서 별도의 C++ 구현 없이 
기존 액션의 데이터 조합만으로 구현할 수 있었습니다.

#### 설계상 타협

현재 전투 판정은 권위있는 서버에서만 진행하지만, 
빠른 조작 반응성을 위해 자동 이동과 실행 등의 일부 동작을 클라이언트 중심으로 처리하고, 
서버에서 검증하는 방식을 택했습니다.

이 과정에서 어디까지 클라이언트를 신뢰하고 서버에서 담당해야 하는지 판단하는 것이 어려웠습니다.

현재는 PvE 중심의 프로젝트이므로 반응성과 구현 복잡도를 고려해 타협했지만, PvP 같은 경쟁 요소가 들어가면 클라이언트에 대한 신뢰를 더욱 엄격히 제한해야 한다고 생각합니다.

#### 아쉬운 점

전투와 멀티 플레이 게임 흐름에 집중하면서, 백엔드 서버 연동을 통한 아웃게임 콘텐츠까지 확장하지 못했습니다.

언리얼에서 제공하는 EQS, 모션 매칭 등의 시스템(플러그인)을 다양하게 경험하지 못했습니다.

#### 다시 개발한다면

처음부터 서버와 클라이언트의 신뢰 경계를 더 명확하게 정의하고, 검증 책임과 예외 상황을 구체적으로 설계할 것입니다.

또한 현재 액티브 어빌리티에선 클라이언트에서 보낸 시간을 기준으로 서버를 보정하는데,
시간을 전적으로 신뢰하지 않고 제한적으로 수용하기 때문에 네트워크가 불안정할 경우 
클라이언트와 서버의 실행 상태가 달라질 수 있습니다.  
이를 개선하기 위해 서버에서 보정된 결과를 기준으로 클라이언트를 보정하는 Reconciliation을 고려할 것입니다.

