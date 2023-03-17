#include "AAlphaBaseCharacter.h"
#include "AAlphaBaseCharacter.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "UAlphaInputConfig.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAlphaBaseCharacter::AAlphaBaseCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	
	SetReplicates(true);

	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AAlphaBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AAlphaBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	
	Subsystem->AddMappingContext(InputContext, 0);

	if (InputActions == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("InputActions nullptr"));
		return;
	}
	
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputActions->InputMove)
		{
			PlayerEnhancedInputComponent->BindAction(InputActions->InputMove, ETriggerEvent::Triggered, this, &AAlphaBaseCharacter::Move);
		}

		if (InputActions->InputLook)
		{
			PlayerEnhancedInputComponent->BindAction(InputActions->InputLook, ETriggerEvent::Triggered, this, &AAlphaBaseCharacter::Look);
		}
	}
}

void AAlphaBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// debug out
	check(GEngine != nullptr);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("AAlphaBaseCharacter::BeginPlay()"));
}

void AAlphaBaseCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D MoveValue = Value.Get<FVector2D>();
	const FRotator MoveRot(0, Controller->GetControlRotation().Yaw, 0);

	// forward/back
	if (MoveValue.Y != 0.0f)
	{
		const FVector Direction = MoveRot.RotateVector(FVector::ForwardVector);
		AddMovementInput(Direction, MoveValue.Y);
	}

	// left/right
	if (MoveValue.X != 0.0f)
	{
		const FVector Direction = MoveRot.RotateVector(FVector::RightVector);
		AddMovementInput(Direction, MoveValue.X);
	}
}

void AAlphaBaseCharacter::Look(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller nullptr"))
		return;
	}

	const FVector2D LookValue = Value.Get<FVector2D>();

	// look x-axis
	if (LookValue.X != 0.f)
	{
		AddControllerYawInput(LookValue.X);
	}

	// look y-axis
	if (LookValue.Y != 0.f)
	{
		AddControllerPitchInput(LookValue.Y);
	}
}





