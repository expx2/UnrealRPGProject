# 시스템 아키텍처

객체별 책임, 데이터 흐름, 생명주기와 시스템 간 연결을 설명한다.

## 전체 구조

| 영역 | 주요 클래스 | 책임 |
|--|-|-|
| 게임 진행 | `AURGameGameMode`, `AURGameGameState` | 플레이어 사망·전멸, 리스폰, 보스 사망 및 클리어 상태 |
| 플레이어 | `AURGamePlayerController`, `UURCharacterInputComponent`, `UURTargetingComponent` | 입력 연결, 로컬 대상 선정, 스킬 사용 전 접근, 관전·카메라 |
| 캐릭터 | `AURASCCharacter`, `AURPlayerCharacter`, `AUREnemyCharacter` | ASC와 캐릭터 연결, 공통 전투 컴포넌트, 플레이어·AI별 처리 |
| 능력 및 상태 | `UURAbilitySystemComponent`, `UURGameplayAbility_ActiveAbility`, `AttributeSet` | 스킬 선택·실행, GE, 태그, 수치 상태 |
| 전투 | `UAbilityTask_AbilityActionTimeline`, Action 구조체·태스크, Combat/Collision Library | 시간에 따른 액션 실행, 타격 후보 탐색과 효과 적용 |
| UI | `UURUISubsystem`, `AURGameHUD`, 액터 표시·대미지 위젯 | 로컬 UI 계층과 플레이어에게 표시할 피드백 |

### 플레이어 캐릭터 선택과 생성

```mermaid
flowchart LR
  GI["GameInstnace<br/>캐릭터 ID 저장"] --> |레벨 이동| GM["GameMode::OnPostLogin"]
  GM --> S_PC["서버 PlayerController<br>"]
  S_PC --> |클라이언트 RPC| C_PC["클라이언트 PlayerController<br/>GameInstance에서 캐릭터 ID 가져옴"]
  C_PC --> |서버 RPC| S_PC2 ["서버 PlayerController<br/>검증 후 `RestatPlayer`"]
```

로비에서 선택한 캐릭터 ID는 레벨 이동 후에도 유지할 수 있도록 `GameInstance`에 저장한다.

`AURGameGameMode`는 `AGameMode::OnPostLogin`를 오버라이드 해 플레이어 로그인 후 
`AURGamePlayerController`에 캐릭터 ID를 요청한다.

`AURGamePlayerController`는 클라이언트 RPC로 캐릭터 코드를 요청하고 클라이언트의 컨트롤러는 `UURGameInstance`에 저장된 캐릭터 ID를 서버 RPC로 서버에 전달한다.

서버에서는 검증 후 `RestartPlayer`를 호출해 캐릭터를 생성한다. 
