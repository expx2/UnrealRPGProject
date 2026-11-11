// Written by EXPx2


#include "Character/URASCCharacter.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/URAbilitySystemComponent.h"

#include "URCombatComponent.h"
#include "URDamageTakenTimelineComponent.h"
#include "URDeathTimelineComponent.h"
#include "URGame.h"
#include "URHealthComponent.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbilitySet.h"
#include "AbilitySystem/GameplayEffect/GameplayEffect_Dead.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/URGameplayTags.h"
#include "GameplayTags/URGameplayTags_Ability.h"
#include "UI/ActorDisplay/URActorDisplayWidgetComponent.h"

AURASCCharacter::AURASCCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	HealthComponent = CreateDefaultSubobject<UURHealthComponent>(TEXT("Health Component"));
	HealthComponent->OnDeath.AddDynamic(this, &ThisClass::HandleDeath);
	
	CombatComponent = CreateDefaultSubobject<UURCombatComponent>(TEXT("Combat Component"));
	
	static const FName ActorDisplayAttachSocketName(TEXT("HoverAttachPoint"));
	ActorDisplayWidgetComponent = CreateDefaultSubobject<UURActorDisplayWidgetComponent>(TEXT("Actor Display Widget Component"));
	ActorDisplayWidgetComponent->SetupAttachment(GetMesh(), ActorDisplayAttachSocketName);
	ActorDisplayWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));
	
	DamageTakenTimelineComponent = CreateDefaultSubobject<UURDamageTakenTimelineComponent>(TEXT("Damage Taken Timeline Component"));
	DamageTakenTimelineComponent->SetMeshComponent(GetMesh());
	
	DeathTimelineComponent = CreateDefaultSubobject<UURDeathTimelineComponent>(TEXT("Death Timeline Component"));
	DeathTimelineComponent->SetMeshComponent(GetMesh());
}

void AURASCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAbilitySystemComponent();
	
	Super::EndPlay(EndPlayReason);
}

void AURASCCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	// 플레이어 캐릭터의 이름 색상 설정 위해 컨트롤러 복제 시 업데이트
	if (ActorDisplayWidgetComponent)
	{
		ActorDisplayWidgetComponent->RefreshNameColor();
	}
}

FRotator AURASCCharacter::GetBaseAimRotation() const
{
	if (CombatComponent)
	{
		// 현재 전투 중인 대상 있을 경우 대상의 AimTargetLocation을 바라보는 Rotator 리턴
		if (AActor* CombatTarget = CombatComponent->GetCombatTargetActor())
		{
			FVector AimLocation;
			UURGameBlueprintLibrary::GetAimTargetLocation(CombatTarget, AimLocation);
			const FVector AimDirection = AimLocation - GetPawnViewLocation();
			if (!AimDirection.IsNearlyZero())
			{
				return AimDirection.Rotation();
			}
		}
	}
	
	return Super::GetBaseAimRotation();
}

void AURASCCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	
	SetMovementModeTag(PrevMovementMode, false);
	SetMovementModeTag(GetCharacterMovement()->MovementMode, true);
}

void AURASCCharacter::ClearAbilitySystemComponent()
{
	if (!URASC)
	{
		return;
	}
	
	// 부여한 어빌리티 제거
	for (FGameplayAbilitySpecHandle& Handle : GrantedAbilitySpecHandles)
	{
		URASC->ClearAbility(Handle);
	}
	
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(HealthComponent);
	
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(CombatComponent);
	
	// 해당 캐릭터가 아직 ASC의 아바타 액터일 경우 초기화
	if (URASC->GetAvatarActor() == this)
	{
		URASC->ClearAbilityInput();

		FGameplayTagContainer ActiveAbilityTag(URGameplayTags::Ability_Active);
		URASC->CancelAbilities(&ActiveAbilityTag);
		
		if (TagRelationshipMapping)
		{
			URASC->SetTagRelationshipMapping(nullptr);
		}
		
		if (URASC->GetOwnerActor() != nullptr)
		{
			URASC->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			URASC->ClearActorInfo();
		}
	}
	URASC = nullptr;
}

void AURASCCharacter::ResetForSpawn()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (URASC && URASC->GetAvatarActor() == this)
	{
		URASC->ClearAbilityInput();
		
		// 어빌리티 종료
		FGameplayTagContainer ActiveAbilityTag(URGameplayTags::Ability_Active);
		URASC->CancelAbilities(&ActiveAbilityTag);
		
		// GE제거
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchNoOwningTags(FGameplayTagContainer(URGameplayTags::TAG_Ability_Condition_Cooldown));
		URASC->RemoveActiveEffects(Query);
	}
	
	// 체력 초기화
	if (HealthComponent)
	{
		HealthComponent-> SetHealthToMax();
	}
	
	// 전투 상태 초기화
	if (CombatComponent)
	{
		CombatComponent->Clear();
	}
	
	// ActorDisplayWidget 상태 초기화
	if (ActorDisplayWidgetComponent)
	{
		ActorDisplayWidgetComponent->ResetIsHit();
	}
}


void AURASCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SetAbilitySystemComponent();
}

void AURASCCharacter::SetAbilitySystemComponent()
{
	if (URASC)
	{
		URASC->InitAbilityActorInfo(this, this);
		InitAbilitySystemComponent();
		ResetForSpawn();
	}
}

void AURASCCharacter::InitAbilitySystemComponent()
{
	check(URASC);
	
	if (!ensure(URASC->GetAvatarActor() == this))
	{
		return;
	}
	
	// Character의 스탯 초기화
	if (HasAuthority())
	{
		// CharacterCode 그룹에 정의되지 않은 스탯 Default값으로 설정위해 Default 우선 설정 후 CharacterCode 그룹 설정
		UAbilitySystemGlobals::Get().GetAttributeSetInitter()->InitAttributeSetDefaults(URASC, "Default", 1, true);
		UAbilitySystemGlobals::Get().GetAttributeSetInitter()->InitAttributeSetDefaults(URASC, CharacterCode, 1, true);
	}
	
	if (TagRelationshipMapping)
	{
		URASC->SetTagRelationshipMapping(TagRelationshipMapping);
	}
	if (AbilitySet)
	{
		AbilitySet->GiveAbilities(URASC, GrantedAbilitySpecHandles);
	}
	
	// 컴포넌트에 ASC 전달
	IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(HealthComponent, URASC);
	IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(CombatComponent, URASC);
	
	K2_OnAbilitySystemInitialized(URASC);
}

void AURASCCharacter::SetMovementModeTag(const EMovementMode MovementMode, const bool bTagEnabled) const
{
	if (URASC)
	{
		const FGameplayTag* Tag = URGameplayTags::MovementModeTagMap.Find(MovementMode);
		if (Tag && Tag->IsValid())
		{
			URASC->SetLooseGameplayTagCount(*Tag, bTagEnabled ? 1 : 0);
		}
		else
		{
			// Tag from MovementMode is invalid
		}
	}
}

void AURASCCharacter::GameplayCue_DamageTaken_Implementation(
	const EGameplayCueEvent::Type CueEvent,
	const FGameplayCueParameters& CueParameters)
{
	DamageTakenTimelineComponent->PlayFromStart();
}

void AURASCCharacter::HandleDeath(AActor* Actor)
{
	if (HasAuthority())
	{
		SetLifeSpan(2.0f);
	}
	
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);
	}
	
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	
	if (UCharacterMovementComponent* MoveComponent = CastChecked<UCharacterMovementComponent>(GetCharacterMovement()))
	{
		MoveComponent->DisableMovement();
		MoveComponent->StopMovementImmediately();
	}
	
	// 상태 초기화
	if (URASC && URASC->GetAvatarActor() == this)
	{
		URASC->ClearAbilityInput();
		
		// 어빌리티 종료
		FGameplayTagContainer ActiveAbilityTag(URGameplayTags::Ability_Active);
		URASC->CancelAbilities(&ActiveAbilityTag);
		
		// GE제거
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchNoOwningTags(FGameplayTagContainer(URGameplayTags::TAG_Ability_Condition_Cooldown));
		URASC->RemoveActiveEffects(Query);
		
		// Dead GE 적용
		URASC->ApplyGameplayEffectToSelf(GetDefault<UGameplayEffect_Dead>(), 1.0f, URASC->MakeEffectContext());
	}
	
	if (CombatComponent)
	{
		CombatComponent->Clear();
	}
	
	if (ActorDisplayWidgetComponent)
	{
		ActorDisplayWidgetComponent->SetVisibility(false, true);
	}
	
	DeathTimelineComponent->PlayFromStart();
	
	if (DeathAnimMontage)
	{
		PlayAnimMontage(DeathAnimMontage);
	}
}
