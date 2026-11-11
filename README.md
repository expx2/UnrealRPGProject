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

## 주요 기능

### 전투 시스템

- 원, 부채꼴, 도넛, 도넛 부채꼴, 사각형 모양의 충돌 감지
- 멀티 플레이 환경에서 서버 책임의 전투 로직과 동기화, 클라이언트 피드백

[상세 문서](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/Combat.md)

### GAS (Gameplay Ability System)

- 데이터 기반의 액션 조합형 액티브 스킬(GameplayAbility) [상세 문서](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/ActiveAbility.md)
- 대미지 및 상태 효과 GameplayEffect
- Attribute를 이용한 캐릭터 스탯
- GameplayCue를 이용한 연출

### 캐릭터

- 플레이어 캐릭터: Enhanced Input, PlayerState에서의 AbilitySystemComponent 관리 [상세 문서](https://github.com/expx2/UnrealRPGProject/blob/main/Docs/Player.md)
- 적 캐릭터: [AI 행동 트리](https://github.com/user-attachments/assets/198ca6ce-403c-4615-b2a5-3fb3214c9fe3), 어그로 관리
- Template 사용 AnimBP

### 게임 플레이

- 타 플레이어 관전 및 전멸 시 리스폰 [`AURGameGameMode`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/GameModes/URGameGameMode.h) [`AURGameGameState`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/GameStates/URGameGameState.h)
- 레벨 시퀀스

### UMG

- UI 관리 LocalPlayerSubsystem [`UURUISubsystem`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/URUISubsystem.h)
- WidgetComponent [`UURActorDisplayWidgetComponent`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/ActorDisplay/URActorDisplayWidgetComponent.h)
- 대미지 플로터 풀링 [`UURFloatingDamageLayerWidget`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/UI/FloatingDamage/URFloatingDamageLayerWidget.h)
- BP/C++ 구현

### MISC

- 우선순위 기반 상태 제어 템플릿 [`URValueByStateResolver`](https://github.com/expx2/UnrealRPGProject/blob/main/Source/URGame/Helpers/URValueByStateResolver.h)

## 포스트모템

언리얼 엔진과 네트워크를 사용하는 게임 개발 경험이 부족해 시작했습니다.

전투를 진행함에 있어 서버와 클라이언트에서의 책임 구분과 네트워크 지연 보정 그리고 RPC를 이용한 통신을 구현해보며,
정확성과 반응성 사이에서 균형을 잡고 일관된 플레이 경험을 만드는 멀티플레이 구조 설계 방법을 조금이나마 익혔습니다.

또한 이전 프로젝트에서 담당했던 데이터 기반 전투(스킬) 시스템을 GAS를 이용해 더 견고하게 구현할 수 있었습니다.

이 경험이 장르와 엔진에 국한되지 않고, 여러 환경에서 게임을 만드는데 좋은 거름이 될 것이라 생각합니다.

만족한 점
- 액티브 스킬(어빌리티)의 데이터 기반 시스템이 잘 구현되어, 캐릭터를 추가하며 스킬을 추가하는데 추가 구현 없이 빠르게 제작할 수 있었습니다.

부족한 점
- 서버/클라 환경에서의 콘텐츠 개발 경험은 쌓지 못했습니다.
- 언리얼에서 제공하는 다른 시스템을 많이 사용해보지 못했습니다.
