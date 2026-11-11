// Written by EXPx2


#include "UI/HUD/URGameHUDAbilitySlotWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/URPlayerResourceSet.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "Components/Image.h"
#include "GameplayTags/URGameplayTags_Ability.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/Common/URAbilitySlotWidget.h"

void UURGameHUDAbilitySlotWidget::ReceiveAbilitySystemComponent_Implementation(UAbilitySystemComponent* AbilitySystemComponent)
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	UURAbilitySystemComponent* URASC = Cast<UURAbilitySystemComponent>(AbilitySystemComponent);
	if (!ensure(URASC))
	{
		return;
	}
	
	AbilityComponent = URASC;

	RegisterDelegates();

	ResolveAbility();
}

void UURGameHUDAbilitySlotWidget::ClearAbilitySystemComponent_Implementation()
{
	UnRegisterDelegates();
	
	SetEmpty();
	
	AbilityComponent.Reset();
}

void UURGameHUDAbilitySlotWidget::SetAbility(const FGameplayAbilitySpecHandle& AbilitySpecHandle)
{
	if (!AbilitySpecHandle.IsValid())
	{
		SetEmpty();
		return;
	}
	
	const UURAbilitySystemComponent* ASC = AbilityComponent.Get();
	if (!ensure(ASC))
	{
		SetEmpty();
		return;
	}
	
	const FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (!AbilitySpec)
	{
		SetEmpty();
		return;
	}
	
	UGameplayAbility* Ability = AbilitySpec->Ability;
	if (!Ability)
	{
		SetEmpty();
		return;
	}
	
	UGameplayAbility* InstancedAbility = AbilitySpec->GetPrimaryInstance();
	UGameplayAbility* AbilitySource = InstancedAbility ? InstancedAbility : Ability;
	UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(AbilitySource);
	if (!IsValid(ActiveAbility))
	{
		SetEmpty();
		return;
	}
	
	ShowingAbility = ActiveAbility;
	AbilitySlotWidget->SetAbility(ActiveAbility);
	
	// 쿨다운 표시
	ResolveCooldown();
	
	// 사용 가능 시간 표시
	ResolveChainTime();
	// if (ActiveAbility->IsChainAbility())
	// {
	// 	// 연계 스킬일 경우 사용 가능 시간 표시
	// 	ResolveChainTime();
	// }

	// 활성 표시
	UpdateIsAbilityEnabled();
}

void UURGameHUDAbilitySlotWidget::NativeDestruct()
{
	IURAbilitySystemComponentReceiverInterface::Execute_ClearAbilitySystemComponent(this);
	
	Super::NativeDestruct();
}

void UURGameHUDAbilitySlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bHasCooldownGE)
	{
		RefreshCooldown();
	}
	if (bHasChainGE)
	{
		RefreshChainTime();
	}
}

void UURGameHUDAbilitySlotWidget::RegisterDelegates()
{
	UURAbilitySystemComponent* ASC = AbilityComponent.Get();
	if (!ensure(ASC))
	{
		return;
	}
	
	// [표시 어빌리티 변경]
	// 1. 태그 변경
	// Note: 커맨드에 해당하는 어빌리티만 가져와서 연관된 태그들에만 Register하는 식으로 처리할 수도 있음
	TagCountChangedDelegateHandle = ASC->RegisterGameplayTagEvent(URGameplayTags::TAG_Ability_Condition, EGameplayTagEventType::AnyCountChange).AddUObject(this, &UURGameHUDAbilitySlotWidget::HandleGameplayTagChanged);
	
	// 2. 어빌리티 추가/제거
	AbilityListChangedDelegateHandle = ASC->OnAbilityListChanged.AddUObject(this, &ThisClass::HandleAbilityListChanged);
	
	// [활성 표시 변경]
	// 1. 마나 변경
	ManaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UURPlayerResourceSet::GetManaAttribute()).AddUObject(this, &UURGameHUDAbilitySlotWidget::HandleManaChanged);
	
	/*
	 * [Deprecated]
	// 2. 어빌리티 시작 및 종료
	// 활성 상태 표시 위해 Spec등 정보가 필요하면 OnAbilityEnded 사용
	ASC->AbilityActivatedCallbacks.AddUObject(this, &UURHUDAbilitySlotWidget::HandleAbilityActivated);
	ASC->AbilityEndedCallbacks.AddUObject(this, &UURHUDAbilitySlotWidget::HandleAbilityEnded);
	// 3. GE 시작 및 종료
	EffectAddedDelegateHandle = ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UURHUDAbilitySlotWidget::OnActiveGameplayEffectAdded);
	EffectRemovedDelegateHandle = ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UURHUDAbilitySlotWidget::OnActiveGameplayEffectRemoved);
	Note: GE 지속 시간 변경 기능이 추가되면 GE 지속 시간 변경 시 처리 필요
	*/
}

void UURGameHUDAbilitySlotWidget::UnRegisterDelegates()
{
	if (UURAbilitySystemComponent* ASC = AbilityComponent.Get())
	{
		if (TagCountChangedDelegateHandle.IsValid())
		{
			ASC->UnregisterGameplayTagEvent(TagCountChangedDelegateHandle, URGameplayTags::TAG_Ability_Condition, EGameplayTagEventType::AnyCountChange);
		}
		if (AbilityListChangedDelegateHandle.IsValid())
		{
			ASC->OnAbilityListChanged.Remove(AbilityListChangedDelegateHandle);
		}
		if (ManaChangedDelegateHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UURPlayerResourceSet::GetManaAttribute()).Remove(ManaChangedDelegateHandle);
		}

	}
	TagCountChangedDelegateHandle.Reset();
	AbilityListChangedDelegateHandle.Reset();
	ManaChangedDelegateHandle.Reset();
}


void UURGameHUDAbilitySlotWidget::SetEmpty()
{
	ShowingAbility = nullptr;
	AbilitySlotWidget->SetAbility(nullptr);
	
	ClearCooldown();
	ClearChainTime();
	UpdateIsAbilityEnabled();
}

void UURGameHUDAbilitySlotWidget::ResolveAbility()
{
	if (UURAbilitySystemComponent* ASC = AbilityComponent.Get())
	{
		// 죽어서 캐릭터 제거되어 어빌리티 제거될 때 업데이트 되는 것 방지
		if (!ASC->IsDead())
		{
			SetAbility(ASC->ResolveCurrentAbilityForInput(Command));
		}
	}
}

void UURGameHUDAbilitySlotWidget::UpdateIsAbilityEnabled() const
{
	if (!ShowingAbility.IsValid() || 
		!AbilityComponent.IsValid())
	{
		ImgDisabled->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	const bool bIsAbilityEnabled = ShowingAbility->CanDisplayAsEnabled(*AbilityComponent);
	ImgDisabled->SetVisibility(bIsAbilityEnabled ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}


void UURGameHUDAbilitySlotWidget::HandleGameplayTagChanged(FGameplayTag GameplayTag, int Count)
{
	ResolveAbility();
}

void UURGameHUDAbilitySlotWidget::HandleAbilityListChanged()
{
	ResolveAbility();
}

void UURGameHUDAbilitySlotWidget::HandleManaChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	UpdateIsAbilityEnabled();
}

// =====================================================================================================================
// Cooldown
// =====================================================================================================================

void UURGameHUDAbilitySlotWidget::ResolveCooldown()
{
	ClearCooldown();
	
	if (!ShowingAbility.IsValid() || 
		!AbilityComponent.IsValid())
	{
		return;
	}
	
	const FGameplayTagContainer* CooldownTags = ShowingAbility->GetCooldownTags();
	if (!CooldownTags || CooldownTags->IsEmpty())
	{
		return;
	}
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
	float EndTime = 0.f;
	float Duration = 0.f;
	// 가장 늦게 끝나는 GE의 EndTime과 Duration을 설정함
	const bool bFoundEffect = AbilityComponent->GetActiveEffectsEndTimeAndDuration(Query, EndTime, Duration);
	if (bFoundEffect)
	{
		const float Current = GetWorld()->GetTimeSeconds();
		if (Duration > SMALL_NUMBER &&
			EndTime > Current)
		{
			bHasCooldownGE = true;
			CooldownEndTime = EndTime;
			CooldownDuration = Duration;

			RefreshCooldown();
		}
	}
}

void UURGameHUDAbilitySlotWidget::ClearCooldown()
{
	bHasCooldownGE = false;
	CooldownEndTime = -1.f;
	CooldownDuration = -1.f;
	SetCooldown(0.f);
}

void UURGameHUDAbilitySlotWidget::RefreshCooldown()
{
	if (!bHasCooldownGE)
	{
		ClearCooldown();
		return;
	}
	
	if (CooldownDuration <= SMALL_NUMBER)
	{
		ClearCooldown();
		return;
	}
	
	const float Current = GetWorld()->GetTimeSeconds();
	if (Current >= CooldownEndTime)
	{
		// 만료
		ClearCooldown();
		return;
	}
	
	const float Remaining = CooldownEndTime - Current;
	
	SetCooldown(Remaining / CooldownDuration);
}

void UURGameHUDAbilitySlotWidget::SetCooldown_Implementation(const float Percentage) const
{
	if (UMaterialInstanceDynamic* CooldownMaterial = ImgCooldown->GetDynamicMaterial())
	{
		static const FName FillParameterName("FillAmount");
		CooldownMaterial->SetScalarParameterValue(FillParameterName, FMath::Clamp(Percentage, 0.f, 1.f));
	}
}

// =====================================================================================================================
// Chain
// =====================================================================================================================

void UURGameHUDAbilitySlotWidget::ResolveChainTime()
{
	ClearChainTime();
	
	if (!ShowingAbility.IsValid() || 
		!AbilityComponent.IsValid())
	{
		return;
	}

	float ShortestEndTime = BIG_NUMBER;
	float ShortestDuration = 0.f;
	
	// 태그가 여러개일 경우 각 태그를 부여한 GE 중 가장 늦게 끝나는 GE를 구하고 
	// 그 중에서 가장 먼저 끝나는 GE의 종료 시간과 지속 시간을 선택한다.
	for (const FGameplayTag& RequiredTag : ShowingAbility->GetActivationRequiredTags())
	{
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(FGameplayTagContainer(RequiredTag));
		float EndTime = 0.f;
		float Duration = 0.f;
		
		// 해당 Tag 부여한 GE 중 가장 늦게 끝나는 GE의 EndTime과 Duration
		const bool bFoundEffect = AbilityComponent->GetActiveEffectsEndTimeAndDuration(Query, EndTime, Duration);
		if (bFoundEffect)
		{
			// Infinite는 제외
			if (EndTime >= SMALL_NUMBER && 
				EndTime < ShortestEndTime)
			{
				// 현재 가장 빨리 사라지는 태그를 가진 GE
				ShortestEndTime = EndTime;
				ShortestDuration = Duration;
			}
		}
	}
	
	// ~Infinite GE 적용 시 ShortestEndTime, ShortestDuration은 음수다. Infinite라면 사용 가능 시간을 표시할 필요도 없음.~
	// Note: => 조건이 여러개일 수 있기 때문에 필요하다.
	if (ShortestDuration > SMALL_NUMBER)
	{
		bHasChainGE = true;
		ChainEndTime = ShortestEndTime;
		ChainDuration = ShortestDuration;
		
		RefreshChainTime();
	}
	
	// 적용중인 RequiredTags를 부여한 GE 없음
}

void UURGameHUDAbilitySlotWidget::ClearChainTime()
{
	bHasChainGE = false;
	ChainEndTime = -1.f;
	ChainDuration = -1.f;
	SetChainTime(0.f);
}

void UURGameHUDAbilitySlotWidget::RefreshChainTime()
{
	if (!bHasChainGE)
	{
		ClearChainTime();
		return;
	}
	if (ChainDuration <= SMALL_NUMBER)
	{
		ClearChainTime();
		return;
	}
	
	const float Current = GetWorld()->GetTimeSeconds();
	if (Current >= ChainEndTime)
	{
		ClearChainTime();
		return;
	}
	
	const float Remaining = ChainEndTime - Current;
	
	SetChainTime(Remaining / ChainDuration);
}

void UURGameHUDAbilitySlotWidget::SetChainTime_Implementation(const float Percentage) const
{
	if (UMaterialInstanceDynamic* AvailableMaterial = ImgAvailableFill->GetDynamicMaterial())
	{
		static const FName FillParameterName("FillAmount");
		AvailableMaterial->SetScalarParameterValue(FillParameterName, FMath::Clamp(Percentage, 0.f, 1.f));
	}
}
