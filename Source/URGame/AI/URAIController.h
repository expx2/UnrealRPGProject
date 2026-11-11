// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "URAIController.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API AURAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AURAIController(const FObjectInitializer& ObjectInitializer);
	
	void RunAI();
	
	void StopAI();
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	void HandleDeath(AActor* Actor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UR|AI")
	void SetBlackBoardValues();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UBlackboardData> BlackboardData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UBehaviorTree> BehaviorTree;
};
