// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/URFaction.h"
#include "URTargetingComponent.generated.h"

enum class EURFaction : uint8;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTargetChangedDelegate, UURTargetingComponent*, TargetingComponent, AActor*, OldTargetActor, AActor*, NewTargetActor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URGAME_API UURTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UURTargetingComponent();
	
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category="UR|Targeting")
	bool SetMainTarget(AActor* NewTargetActor, bool bFromCandidates = false);
	
	UFUNCTION(BlueprintCallable, Category="UR|Targeting")
	void ResolveMainTarget(const TArray<AActor*>& Candidates);
	
	UFUNCTION(BlueprintPure, Category="UR|Targeting")
	AActor* GetMainTargetActor() const { return MainTargetActorPtr.Get(); }
	
	UFUNCTION(BlueprintPure, Category="UR|Targeting")
	AActor* GetAimTargetActor() const { return AimTargetActorPtr.Get(); }
	
	UPROPERTY(BlueprintAssignable, Category="UR|Targeting")
	FTargetChangedDelegate OnMainTargetChanged;
	
	UPROPERTY(BlueprintAssignable, Category="UR|Targeting")
	FTargetChangedDelegate OnAimTargetChanged;

protected:
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	void ValidateMainTargetActor();
	
	void UpdateAimTarget();
	
	void SetAimTarget(AActor* NewTargetActor);
	
	/** 액터가 타겟으로 설정될 수 있는지 검사하여 리턴한다. 유효한 액터인지와 타겟이 될 수 있는지, 거리 밖에 있는지 검사한다. */
	bool CanSetTarget(const AActor* InActor, const EURFaction TargetFactions = EURFaction::Ally | EURFaction::Neutral | EURFaction::Enemy) const;
	
	UFUNCTION()
	void HandleMainTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	
	UFUNCTION()
	void HandleAimTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	
	
	/** 메인 타겟과 에임 타겟 최대 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UR", meta=(UIMin=1, ClampMin=1, Units="cm"))
	float MaxDistance = 4000.0f;
	
	/** 에임 타겟 설정 시 카메라 정면 스윕하는 원의 반지름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UR|Aim", meta=(ClampMin=1, UIMin=1))
	float AimLineCheckRadius = 200.0f;
	
	// /** 에임 타겟 설정 시 화면 방향과의 거리 점수 비중 */
	// UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UR|Aim", meta=(ClampMin=0, UIMin=0))
	// float AimLineWeight = 1.0f;
	
	// /** 조준 대상 설정 시 플레이어 캐릭터와의 거리 점수 비중 */
	// UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UR|Aim", meta=(ClampMin=0, UIMin=0))
	// float AimDistanceWeight = 1.0f;

	
	UPROPERTY(BlueprintReadOnly, Category="UR|Targeting")
	TWeakObjectPtr<APlayerController> OwnerPlayerControllerPtr;
	
	UPROPERTY(BlueprintReadOnly, Category="UR|Targeting")
	TWeakObjectPtr<AActor> MainTargetActorPtr;
	
	/** 최근 메인 타겟 리스트. 포함되어 있을 경우 메인 타겟 대상에서 후순위로 처리한다. */
	TArray<TWeakObjectPtr<AActor>> RecentMainTargetActors;

	UPROPERTY(BlueprintReadOnly, Category="UR|Targeting")
	TWeakObjectPtr<AActor> AimTargetActorPtr;
};
