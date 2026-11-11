// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Combat/URHitEffect.h"
#include "Definitions/URFaction.h"
#include "GameFramework/Actor.h"
#include "StructUtils/InstancedStruct.h"
#include "HitTraceActor.generated.h"

UCLASS()
class URGAME_API AHitTraceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AHitTraceActor();
	
	virtual void Tick(float DeltaSeconds) override;

	void InitHitTraceActor(class UAbilitySystemComponent* InASC, AActor* InTargetActor, EURFaction InTargetFactions, const TArray<FURHitEffect>& InHitEffects, FGameplayTag InHitImpactTag, const float ElapsedTime = 0.0f);

protected:
	virtual void BeginPlay() override;
	
	void Trace();
	
	void Hit(TArray<AActor*> TargetActors);

	UFUNCTION()
	void HandleAvatarActorDeath(AActor* Actor);
	
	/** 액터 수명으로 타격 완료와 상관 없이 제거된다. 0이면 수명 제한 없음. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Units="s", ClampMin="0", UIMin="0"))
	float LifeTime = 5.0f;
	
	/** 타격 시작 딜레이 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Units="s", ClampMin="0", UIMin="0"))
	float PreDelay = 0.2f;
	
	/** 범위 정보 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ExcludeBaseStruct))
	TInstancedStruct<struct FURColliderShape> Shape;
	
	/** 최대 타격 가능한 대상 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	int MaxTargetCount = 1; 
	
	/** 타격 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ClampMin=1, UIMin=1))
	int HitCount = 1;

	/** 타격 인터벌 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(EditCondition="HitCount > 1", EditConditionHides, Units="s", ClampMin=0.02, UIMin=0.02))
	float HitInterval = 0.1f;
	
	TWeakObjectPtr<class UAbilitySystemComponent> AbilityComponent;
	TWeakObjectPtr<AActor> TargetActor;
	EURFaction TargetFactions = EURFaction::None;
	TArray<FURHitEffect> HitEffects;
	FGameplayTag HitImpactTag;

	double NextActionTime = 0.0;
	int TraceCount = 0;

	// 다수의 타격 횟수를 가질 때, 이전 타격에서의 대상 액터들을 저장해 다음 타격에 우선권을 부여함
	TArray<TWeakObjectPtr<AActor>> LastHitActors;
	
	FDelegateHandle DeathDelegateHandle;
};
