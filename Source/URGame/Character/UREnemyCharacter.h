// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Character/URASCCharacter.h"
#include "Definitions/URFaction.h"
#include "Interface/URAICharacterInterface.h"
#include "UREnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API AUREnemyCharacter : public AURASCCharacter,
                                     public IURAICharacterInterface
{
	GENERATED_BODY()
	
public:
	AUREnemyCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void ResetForSpawn() override;
	
	// ~Begin IURTargetableInterface
	virtual EURFaction GetFaction_Implementation() const override { return EURFaction::Enemy; }
	// ~End IURTargetableInterface
	
	// ~Begin IURAICharacterInterface
	virtual float GetPatrolRadius_Implementation() const override { return PatrolRadius; }
	virtual bool GetIsAggressive_Implementation() const override { return bIsAggressive; }
	virtual float GetDetectRange_Implementation() const override { return DetectRange; }
	virtual float GetActiveRange_Implementation() const override { return ActiveRange; }
	virtual class UAIAbilityOrderSet* GetAbilityOrderSet_Implementation() const override { return AbilityOrderSet; }
	// ~End IURAICharacterInterface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURAggroComponent> AggroComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURHealthSet> HealthSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURCombatSet> CombatSet;
	
	
	/** 순찰 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UR|EnemyAI", meta=(Units="cm"))
	float PatrolRadius = 500.f;
	
	/** 선공 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UR|EnemyAI")
	bool bIsAggressive;
	
	/** 선공 몬스터의 감지 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UR|EnemyAI", meta=(Units="cm", EditCondition="bIsAggressive == true", EditConditionHides))
	float DetectRange = 500.f;
	
	/** 활동 가능 범위. 범위를 벗어나면 스폰 위치로 돌아간다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UR|EnemyAI", meta=(Units="cm", EditCondition="bIsAggressive == true", EditConditionHides))
	float ActiveRange = 3000.f;
	
	/** 어빌리티 사용 순서 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UR|EnemyAI")
	TObjectPtr<class UAIAbilityOrderSet> AbilityOrderSet;
};
