// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "URAggroComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URGAME_API UURAggroComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UURAggroComponent();

	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void Clear();
	
	void CleanInvalidPair();
	
	/** 
	 * @return 가장 어그로 수치가 높은 대상 리턴 
	 */
	UFUNCTION(BlueprintCallable, Category="UR|Aggro")
	AActor* GetTargetActor();

	/**
	 * @param bIncludeMost 가장 어그로 수치가 높은 대상도 포함할지 여부
	 * @return 어그로 수치가 있는 대상 중 랜덤하게 리턴
	 */
	UFUNCTION(BlueprintCallable, Category="UR|Aggro")
	AActor* GetRandomTargetActor(const bool bIncludeMost);
	
	UFUNCTION(BlueprintCallable, Category="UR|Aggro")
	void AddAggro(AActor* Instigator, const float AggroValue);
	
	void ResetAggro();

protected:
	UFUNCTION()
	void HandleDamageTaken(AActor* Instigator, AActor* Causer, float Magnitude);
	
	void LogAggroMap();
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Units="cm"))
	float MaxDistance = 5000.f;
	
	TMap<TWeakObjectPtr<AActor>, float> AggroMap;
};
