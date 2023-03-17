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

	// manually set walk speed
	GetCharacterMovement()->MaxWalkSpeed = GetBaseMovementSpeed();
	
	SetReplicates(true);

	bReplicates = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AAlphaBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// print debug to screen
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Cyan, FString::Printf(TEXT("vel: %f"), GetVelocity().Size()));
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

		if (InputActions->InputJump)
		{
			PlayerEnhancedInputComponent->BindAction(InputActions->InputJump, ETriggerEvent::Triggered, this, &AAlphaBaseCharacter::Jump);
		}

		if (InputActions->InputSpecial1)
		{
			PlayerEnhancedInputComponent->BindAction(InputActions->InputSpecial1, ETriggerEvent::Triggered, this, &AAlphaBaseCharacter::SpecialA1);
		}

		if (InputActions->InputSpecial2)
		{
			PlayerEnhancedInputComponent->BindAction(InputActions->InputSpecial2, ETriggerEvent::Triggered, this, &AAlphaBaseCharacter::SpecialA2);
		}
	}
}

void AAlphaBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
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

// TODO: Make special ability calls a single universal func call
void AAlphaBaseCharacter::SpecialA1(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller nullptr"))
		return;
	}

	GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, FString::Printf(TEXT("special 1 triggered")));
	UE_LOG(LogTemp, Display, TEXT("AAlphaBaseCharacter::SpecialA1::init()"));

	if (bool InputValue = Value.Get<bool>())
	{
		// TODO: Trigger special ability 1
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, FString::Printf(TEXT("special 1 true")));
	}
}

void AAlphaBaseCharacter::SpecialA2(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller nullptr"))
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, FString::Printf(TEXT("special 2 triggered")));
	UE_LOG(LogTemp, Display, TEXT("AAlphaBaseCharacter::SpecialA2::init()"));

	if (bool InputValue = Value.Get<bool>())
	{
		// TODO: Trigger special ability 2
		GEngine->AddOnScreenDebugMessage(3, 5.0f, FColor::Green, FString::Printf(TEXT("special 2 true")));
	}
}