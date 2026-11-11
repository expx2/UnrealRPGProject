// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "URDamageTakenTimelineComponent.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURDamageTakenTimelineComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UURDamageTakenTimelineComponent();
	
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void SetMeshComponent(UMeshComponent* InMeshComponent);

	UFUNCTION(BlueprintCallable, Category="UR|Character|DamageTaken")
	void PlayFromStart();
	
protected:
	UFUNCTION()
	void HandleTimelineUpdate(float Value);
	
	UFUNCTION()
	void HandleTimelineFinished();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UCurveFloat> FlashCurve;
	
	UPROPERTY(BlueprintReadOnly, Category="UR")
	TObjectPtr<class UMeshComponent> MeshComp;
	
	FTimeline Timeline;
};
