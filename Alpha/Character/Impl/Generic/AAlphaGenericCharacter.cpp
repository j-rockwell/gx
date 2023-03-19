#include "AAlphaGenericCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAlphaGenericCharacter::AAlphaGenericCharacter()
{
	SetName("Generic Character");
	SetBaseHealth(200.0f);
	SetBaseHealthRegeneration(15.0f);
	SetBaseEyeHeight(64.0f);
	SetBaseMovementSpeed(1000.0f);

	// remove this after debugging
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// manually set walk speed
	GetCharacterMovement()->MaxWalkSpeed = GetBaseMovementSpeed();
	
	SetReplicates(true);

	bReplicates = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
	PrimaryActorTick.bCanEverTick = true;
	
	UE_LOG(LogTemp, Display, TEXT("AAlphaGenericCharacter init()"));
}

void AAlphaGenericCharacter::SpecialA1(const FInputActionValue& Value)
{
	Super::SpecialA1(Value);
	GEngine->AddOnScreenDebugMessage(4, 5.0f, FColor::Orange, FString::Printf(TEXT("special 1 true (from child)")));
}

void AAlphaGenericCharacter::SpecialA2(const FInputActionValue& Value)
{
	Super::SpecialA2(Value);
	GEngine->AddOnScreenDebugMessage(5, 5.0f, FColor::Orange, FString::Printf(TEXT("special 2 true (from child)")));
}

