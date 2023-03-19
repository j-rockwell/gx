#pragma once
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "UAlphaMovementConfig.h"
#include "AAlphaBaseCharacter.generated.h"

UCLASS(Config=Game)
class AAlphaBaseCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	class UCameraComponent* CameraComponent;
	
public:
	AAlphaBaseCharacter();

	UFUNCTION(Category = "Getters", BlueprintPure) FORCEINLINE UAlphaMovementConfig* GetMovementPtr() const
	{
		return MovementPtr;
	}

	/**
	 * @brief Returns the name of the character
	 * @return Character Name
	 */
	 FString GetName() { return Name; }

	/**
	 * Returns the eye height (in units) the camera should be moved on the y-axis
	 */
	float GetBaseEyeHeight() { return BaseEyeHeight; }

	/**
	 * Sets the eye height (in units) the camera should be moved on the y-axis
	 * @param NewBaseEyeHeight Distance in units
	 */
	void SetBaseEyeHeight(float NewBaseEyeHeight) { BaseEyeHeight = NewBaseEyeHeight; }

	/**
	 * Sets the name of the character
	 * @param NewName Character Name
	 */
	void SetName(FString NewName) { Name = NewName; }

	/**
	 * Returns the characters base health without any modifiers
	 */
	float GetBaseHealth() { return BaseHealth; }

	/**
	 * Sets the characters base health without any modifiers
	 * @param NewBaseHealth Health total (100.0 = 100 health)
	 */
	void SetBaseHealth(float NewBaseHealth) { BaseHealth = NewBaseHealth; }

	/**
	 * Returns the base health regeneration rate (per 1s)
	 */
	float GetBaseHealthRegeneration() { return BaseHealthRegeneration; }

	/**
	 * Sets the characters base health regeneration without any modifiers
	 * @param NewBaseHealthRegeneration New health regeneration (per 1s)
	 */
	void SetBaseHealthRegeneration(float NewBaseHealthRegeneration) { BaseHealthRegeneration = NewBaseHealthRegeneration; }

	/**
	 * Returns the base movement speed (unit/s) without any modifiers
	 */
	float GetBaseMovementSpeed() { return BaseMovementSpeed; }

	/**
	 * Sets the characters base movement speed (unit/s) without any modifiers
	 * @param NewBaseMovementSpeed New movement speed (unit/s)
	 */
	void SetBaseMovementSpeed(float NewBaseMovementSpeed) { BaseMovementSpeed = NewBaseMovementSpeed; }

	/**
	 * Returns true if the player is attempting to walk in a new direction
	 */
	UFUNCTION()
	bool DoesWantToWalk() const
	{
		 return bWantsToWalk;
	}

	/**
	 * Returns true if the player is walking
	 */
	UFUNCTION()
	bool IsWalking() const
	{
		 return bIsWalking;
	}
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input from Script")
	class UInputMappingContext* InputContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input from Script")
	class UAlphaInputConfig* InputActions;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;
	virtual void SpecialA1(const FInputActionValue& Value);
	virtual void SpecialA2(const FInputActionValue& Value);
	// void UltimateA(const FInputActionValue& Value);

private:
	UAlphaMovementConfig* MovementPtr;
	
	FString Name;
	float BaseEyeHeight;
	float BaseHealth;
	float BaseHealthRegeneration;
	float BaseMovementSpeed;
	bool bWantsToWalk;
	bool bIsWalking;
	bool bDeferJumpStop;
};
