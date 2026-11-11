// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Helpers/URValueByStateResolver.h"
#include "URCharacterMovementComponent.generated.h"

UENUM()
enum class EURRotationSource : uint8
{
	Default,
	Combat,
	MoveTo,
	Ability,
};

USTRUCT()
struct FURRotationContext
{
	GENERATED_BODY()
	
public:
	FURRotationContext() : bHasTargetActor(false)
	{
	}
	
	explicit FURRotationContext(AActor* Actor) : bHasTargetActor(true)
	{
		TargetActor = Actor;
	}

	explicit FURRotationContext(const FVector& Location) : bHasTargetActor(false)
	{
		TargetLocation = Location;
	}
	
	FVector GetLookAtLocation() const
	{
		return bHasTargetActor && TargetActor.IsValid() ? TargetActor->GetActorLocation() : TargetLocation;
	}
	
	bool bHasTargetActor;
	
	bool bUsingRotationSpeed = true;
	
	TWeakObjectPtr<AActor> TargetActor;
	
	FVector TargetLocation;
};

/**
 * 
 */
UCLASS()
class URGAME_API UURCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UURCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void PhysicsRotation(float DeltaTime) override;
	
	void LookAt(const FVector& Location, bool bUsingRotationSpeed, float DeltaSeconds = -1.f);
	
	TURValueByStateResolver<EURRotationSource, FURRotationContext> RotationResolver;
};
