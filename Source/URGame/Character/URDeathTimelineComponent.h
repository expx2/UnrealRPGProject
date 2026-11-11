// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "URDeathTimelineComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URGAME_API UURDeathTimelineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UURDeathTimelineComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetMeshComponent(UMeshComponent* InMeshComponent);

	UFUNCTION(BlueprintCallable, Category="UR|Character|DamageTaken")
	void PlayFromStart();
	
protected:
	UFUNCTION()
	void HandleTimelineUpdate(float Value);
	
	UFUNCTION()
	void HandleTimelineFinished();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UCurveFloat> FadeCurve;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UParticleSystem> FinishParticle;
	
	UPROPERTY(BlueprintReadOnly, Category="UR")
	TObjectPtr<class UMeshComponent> MeshComp;
	
	FTimeline Timeline;
};
