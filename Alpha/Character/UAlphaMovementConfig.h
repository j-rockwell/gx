#pragma once
#include "GameFramework/CharacterMovementComponent.h"
#include "UAlphaMovementConfig.generated.h"

UCLASS()
class UAlphaMovementConfig : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UAlphaMovementConfig();

	// init
	virtual void InitializeComponent() override;
	virtual void OnRegister() override;

	// movement overrides
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	virtual bool ShouldLimitAirControl(float DeltaTime, const FVector& FallAcceleration) const override;
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;
	
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	void UpdateSurfaceFriction(bool bIsSliding = false);

	// jump overrides
	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves) override;
	
	virtual void TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const override;
	virtual float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact) override;
	virtual FVector ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const override;
	virtual FVector HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const override;
	virtual bool ShouldCatchAir(const FFindFloorResult& OldFloor, const FFindFloorResult& NewFloor) override;
	virtual bool IsWithinEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const override;
	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const override;
	virtual bool ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit) const override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	
	void TraceCharacterFloor(FHitResult& OutHit);

	float GetCameraRoll();
	virtual float GetMaxSpeed() const override;
	
	FORCEINLINE FVector GetAcceleration() const
	{
		return Acceleration;
	}

	bool IsBrakingFrameTolerated() const
	{
		return bBrakingFrameTolerated;
	}
	
protected:
	class AAlphaBaseCharacter* AlphaCharacter;
	
	/**
	 * Multiplier for acceleration when on the ground
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float GroundAccelerationModifier;

	/**
	 * Multiplier for acceleration while in the air
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float AirAccelerationModifier;

	/**
	 * Vector differential magnitude cap when in the air
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling")
	float AirSpeedCap;

	/**
	 * The minimum step height from moving fast
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float MinStepHeight;

	/**
	 * Default sprint speed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float BaseMovementSpeed;

	/**
	 * Walking movement speed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float WalkMovementSpeed;

	/**
	 * Minimum speed to scale up from slope movement
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Surfing")
	float MinSlopeSpeedModifier;

	/**
	 * Maximum speed to scale up from slope movement
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Surfing")
	float MaxSlopeSpeedModifier;

	/**
	 * Max angle to roll for camera adjustment
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Camera")
	float CamRollAngle;

	/**
	 * Speed to roll the camera
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Camera")
	float CamRollSpeed;

	/**
	 * Speed of rolling the camera
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Camera")
	float CamBounceModifier;

	/**
	 * Max speed allowed on any given axis
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float AxisSpeedLimit = 700.0f;

	/**
	 * Threshold relating to speed ratio and friction which causes us to catch air
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float SlideLimit = 0.5f;
	
	/**
	 * FLAG
	 * If the player has been on the ground for at least one frame and braking can be applied
	 */
	bool bBrakingFrameTolerated;

private:
	float DefaultStepHeight;
	float DefaultWalkableFloorZ;
	float SurfaceFriction;
};
