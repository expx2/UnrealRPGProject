// Written by EXPx2


#include "Character/URCharacter.h"
#include "Character/URCharacterMovementComponent.h"

// Sets default values
AURCharacter::AURCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UURCharacterMovementComponent>(CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->bReceivesDecals = false;
}

// Called when the game starts or when spawned
void AURCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AURCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AURCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

