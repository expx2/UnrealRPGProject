// Written by EXPx2


#include "AI/URAIController.h"

#include "URGame.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Character/URHealthComponent.h"
#include "Components/URPathFollowingComponent.h"

AURAIController::AURAIController(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer.SetDefaultSubobjectClass<UURPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BlackboardRef(TEXT("/Game/_ProjectUR/AI/BB_Zako.BB_Zako"));
	BlackboardData = BlackboardRef.Object;
	
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BehaviourTreeRef(TEXT("/Game/_ProjectUR/AI/BT_Zako.BT_Zako"));
	BehaviorTree = BehaviourTreeRef.Object;
}

void AURAIController::RunAI()
{
	UR_LOG(LogUR, Log, TEXT("RunAI %s"), *GetPawn()->GetName())
	UBlackboardComponent* BlackboardPtr = Blackboard.Get();
	if (UseBlackboard(BlackboardData, BlackboardPtr))
	{
		SetBlackBoardValues();
		
		const bool bResult = RunBehaviorTree(BehaviorTree);
		ensure(bResult);
	}
}

void AURAIController::StopAI()
{
	if (UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BehaviorTreeComponent->StopTree();
	}
	
}

void AURAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (UURHealthComponent* HealthComponent = InPawn->FindComponentByClass<UURHealthComponent>())
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &ThisClass::HandleDeath);
	}
	RunAI();
}

void AURAIController::HandleDeath(AActor* Actor)
{
	if (Actor == GetPawn())
	{
		StopAI();
	}
}

void AURAIController::SetBlackBoardValues_Implementation()
{
	
}
