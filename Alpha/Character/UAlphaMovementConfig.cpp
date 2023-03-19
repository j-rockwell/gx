#include "UAlphaMovementConfig.h"
#include "AAlphaBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Math/UnitConversion.h"

// magic numbers
constexpr float DesiredGravity = -1143.0f;
constexpr float JumpVelocity = 266.7f;
const float MAX_STEP_SIDE_Z = 0.08f;
const float VERTICAL_SLOPE_NORMAL_Z = 0.001f;

/**
 * Calculates the friction from hitting a physical object
 */
float GetFrictionFromHit(const FHitResult& Hit)
{
	float SurfaceFriction = 1.0f;

	if (Hit.PhysMaterial.IsValid())
		SurfaceFriction = FMath::Min(1.0f, Hit.PhysMaterial->Friction * 1.25f);

	return SurfaceFriction;
}

UAlphaMovementConfig::UAlphaMovementConfig()
{
	AirControl = 1.0f;
	AirControlBoostMultiplier = 0.0f;
	AirControlBoostVelocityThreshold = 0.0f;
	MaxAcceleration = 857.25f;
	WalkMovementSpeed = 285.75f;
	BaseMovementSpeed = 609.6f;
	MaxWalkSpeed = BaseMovementSpeed;
	GroundAccelerationModifier = 10.0f;
	AirAccelerationModifier = 10.0f;
	AirSpeedCap = 57.15f;
	GroundFriction = 4.0f;
	BrakingFriction = 4.0f;
	SurfaceFriction = 1.0f;
	bUseSeparateBrakingFriction = false;
	BrakingFrictionFactor = 1.0f;
	BrakingSubStepTime = 0.015f;
	MaxSimulationTimeStep = 0.5f;
	MaxSimulationIterations = 1;
	FallingLateralFriction = 0.0f;
	BrakingDecelerationFalling = 0.0f;
	BrakingDecelerationFlying = 190.5f;
	BrakingDecelerationSwimming = 190.5f;
	BrakingDecelerationWalking = 190.5f;
	MaxStepHeight = 34.29f;
	DefaultStepHeight = MaxStepHeight;
	MinStepHeight = 10.0f;
	JumpZVelocity = 304.8f;
	JumpOffJumpZFactor = 0.0f;
	MinSlopeSpeedModifier = BaseMovementSpeed * 1.7f;
	MaxSlopeSpeedModifier = BaseMovementSpeed * 2.5f;
	bBrakingFrameTolerated = true;
	SetWalkableFloorZ(0.7f);
	DefaultWalkableFloorZ = GetWalkableFloorZ();
	AxisSpeedLimit = 6667.5f;
	StandingDownwardForceScale = 1.0f;
	InitialPushForceFactor = 100.0f;
	PushForceFactor = 500.0f;
	RepulsionForce = 0.0f;
	MaxTouchForce = 0.0f;
	TouchForceFactor = 0.0f;
	bPushForceUsingZOffset = false;
	PushForcePointZOffsetFactor = -0.66f;
	bScalePushForceToVelocity = true;
	bPushForceScaledToMass = false;
	bTouchForceScaledToMass = false;
	Mass = 85.0f;
	bUseControllerDesiredRotation = false;
	bUseFlatBaseForFloorChecks = true;
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanJump = true;
	NavAgentProps.bCanFly = true;
	GravityScale = DesiredGravity / UPhysicsSettings::Get()->DefaultGravityZ;
	bMaintainHorizontalGroundVelocity = true;
}

void UAlphaMovementConfig::InitializeComponent()
{
	Super::InitializeComponent();
	AlphaCharacter = Cast<AAlphaBaseCharacter>(GetOwner());
}

void UAlphaMovementConfig::OnRegister()
{
	Super::OnRegister();
}

void UAlphaMovementConfig::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UpdatedComponent->IsSimulatingPhysics())
		return;

	// roll cam
	if (CamRollAngle != 0 && CamRollSpeed != 0 && AlphaCharacter->GetController())
	{
		FRotator ControlRot = AlphaCharacter->GetController()->GetControlRotation();
		ControlRot.Roll = GetCameraRoll();
		AlphaCharacter->GetController()->SetControlRotation(ControlRot);
	}

	bBrakingFrameTolerated = IsMovingOnGround();
}

bool UAlphaMovementConfig::DoJump(bool bReplayingMoves)
{
	if (CharacterOwner && CharacterOwner->CanJump() && (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f))
	{
		if (Velocity.Z <= 0.0f)
			Velocity.Z = JumpZVelocity;
		else
			Velocity.Z = JumpZVelocity;

		SetMovementMode(MOVE_Falling);
		return true;
	}

	return false;
}

void UAlphaMovementConfig::TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const
{
	Super::TwoWallAdjust(Delta, Hit, OldHitNormal);
}

float UAlphaMovementConfig::SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact)
{
	return Super::SlideAlongSurface(Delta, Time, Normal, Hit, bHandleImpact);
}

FVector UAlphaMovementConfig::ComputeSlideVector(const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const
{
	return Super::ComputeSlideVector(Delta, Time, Normal, Hit);
}

FVector UAlphaMovementConfig::HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const
{
	const float WallAngle = FMath::Abs(Hit.ImpactNormal.Z);
	FVector ImpactNormal;

	// cap normal if too extreme
	if (WallAngle <= VERTICAL_SLOPE_NORMAL_Z || WallAngle == 1.0f)
		ImpactNormal = Normal;
	else
		ImpactNormal = Hit.ImpactNormal;

	if (bConstrainToPlane)
		ImpactNormal = ConstrainNormalToPlane(ImpactNormal);

	const float BounceCoeff = 1.0f + CamBounceModifier * (1.0f - SurfaceFriction);
	return (Delta - BounceCoeff * Delta.ProjectOnToNormal(ImpactNormal)) * Time;
}

bool UAlphaMovementConfig::ShouldCatchAir(const FFindFloorResult& OldFloor, const FFindFloorResult& NewFloor)
{
	const float OldSurfaceFriction = GetFrictionFromHit(OldFloor.HitResult);
	const float SpeedMod = MaxSlopeSpeedModifier / Velocity.Size2D();
	const float Diff = NewFloor.HitResult.ImpactNormal.Z - OldFloor.HitResult.ImpactNormal.Z;
	const float Slope = Velocity | OldFloor.HitResult.ImpactNormal;
	const float StrafeMovement = FMath::Abs(GetLastInputVector() | GetOwner()->GetActorRightVector());
	
	const bool bSliding = OldSurfaceFriction * SpeedMod < 0.5f;
	const bool bGainingRamp = Diff >= 0.0f;
	const bool bWasGoingUpRamp = Slope < 0.0f;
	const bool bStrafingOffRamp = StrafeMovement > 0.0f;
	const bool bMovingForCatchAir = bWasGoingUpRamp || bStrafingOffRamp;

	if (bSliding && bGainingRamp && bMovingForCatchAir)
		return true;

	return Super::ShouldCatchAir(OldFloor, NewFloor);
}

bool UAlphaMovementConfig::IsWithinEdgeTolerance(const FVector& CapsuleLocation, const FVector& TestImpactPoint, const float CapsuleRadius) const
{
	return Super::IsWithinEdgeTolerance(CapsuleLocation, TestImpactPoint, CapsuleRadius);
}

bool UAlphaMovementConfig::ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit) const
{
	return !bUseFlatBaseForFloorChecks && Super::ShouldCheckForValidLandingSpot(DeltaTime, Delta, Hit);
}

bool UAlphaMovementConfig::IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const
{
	if (!Hit.bBlockingHit)
		return false;

	if (!Hit.bStartPenetrating)
	{
		if (!IsWalkable(Hit))
			return false;

		float PawnRadius;
		float PawnHalfHeight;
		float LowerHemisphereZ;

		CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

		if (bUseFlatBaseForFloorChecks)
		{
			LowerHemisphereZ = Hit.Location.Z - PawnHalfHeight + MAX_FLOOR_DIST;
			
			if ((Hit.ImpactNormal.Z < GetWalkableFloorZ() || Hit.ImpactNormal.Z == 1.0f) && Hit.ImpactPoint.Z > LowerHemisphereZ)
				return false;
		}
		else
		{
			LowerHemisphereZ = Hit.Location.Z - PawnHalfHeight + PawnRadius;

			if (Hit.ImpactPoint.Z >= LowerHemisphereZ)
				return false;
		}

		if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, PawnRadius))
			return false;
	}
	else if (Hit.Normal.Z < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	FFindFloorResult FloorResult;
	FindFloor(CapsuleLocation, FloorResult, false, &Hit);

	if (!FloorResult.IsWalkableFloor())
		return false;

	if (Hit.Normal.Z < 1.0f && (Velocity | Hit.Normal) < 0.0f)
	{
		FVector DeflectVector = Velocity;
		DeflectVector.Z += 0.5f * GetGravityZ() * GetWorld()->GetDeltaSeconds();
		DeflectVector = ComputeSlideVector(DeflectVector, 1.0f, Hit.Normal, Hit);

		if (DeflectVector.Z > JumpVelocity)
			return false;
	}

	return true;
}

void UAlphaMovementConfig::TraceCharacterFloor(FHitResult& OutHit)
{
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CharacterFloorTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	
	InitCollisionParams(CapsuleParams, ResponseParam);
	
	CapsuleParams.bTraceComplex = true;
	CapsuleParams.bReturnPhysicalMaterial = true;

	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_None);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
	FVector StandingLocation = PawnLocation;
	
	StandingLocation.Z -= MAX_FLOOR_DIST * 10.0f;
	
	GetWorld()->SweepSingleByChannel(
		OutHit,
		PawnLocation,
		StandingLocation,
		FQuat::Identity,
		CollisionChannel,
		StandingCapsuleShape,
		CapsuleParams,
		ResponseParam
	);
}

void UAlphaMovementConfig::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	bool bJumped = false;

	if (PreviousMovementMode == MOVE_Walking && MovementMode == MOVE_Falling)
		bJumped = true;

	FHitResult Hit;
	TraceCharacterFloor(Hit);

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

float UAlphaMovementConfig::GetCameraRoll()
{
	if (CamRollSpeed == 0.0f || CamRollAngle == 0.0f)
		return 0.0f;

	float Side = Velocity | FRotationMatrix(GetCharacterOwner()->GetControlRotation()).GetScaledAxis(EAxis::Y);
	const float Sign = FMath::Sign(Side);

	Side = FMath::Abs(Side);

	if (Side < CamRollSpeed)
		Side = Side * CamRollAngle / CamRollSpeed;
	else
		Side = CamRollAngle;

	return Side * Sign;
}

void UAlphaMovementConfig::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	if (Velocity.IsNearlyZero(0.1f) || !HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME)
		return;

	const float Speed = Velocity.Size2D();
	const float FrictionFactor = FMath::Max(0.0f, BrakingFrictionFactor);

	Friction = FMath::Max(0.0f, Friction * FrictionFactor);
	{
		BrakingDeceleration = FMath::Max(BrakingDeceleration, Speed);
	}
	BrakingDeceleration = FMath::Max(0.0f, BrakingDeceleration);

	const bool bZeroFriction = FMath::IsNearlyZero(Friction);
	const bool bZeroBraking = BrakingDeceleration == 0.0f;

	if (bZeroFriction || bZeroBraking)
		return;

	const FVector OldVelocity = Velocity;
	const FVector ReverseAcceleration = -Velocity.GetSafeNormal();
	const float MaxStepTime = FMath::Clamp(BrakingSubStepTime, 1.0f / 75.0f, 1.0f / 20.0f);
	float RemainingTime = DeltaTime;

	while(RemainingTime >= MIN_TICK_TIME)
	{
		const float Delta = (RemainingTime > MaxStepTime ? FMath::Min(MaxStepTime, RemainingTime * 0.5f) : RemainingTime);
		RemainingTime -= Delta;

		Velocity += (Friction * BrakingDeceleration * ReverseAcceleration) * Delta;

		if ((Velocity | OldVelocity) <= 0.0f)
		{
			Velocity = FVector::ZeroVector;
			return;
		}
	}

	if (Velocity.IsNearlyZero(KINDA_SMALL_NUMBER))
		Velocity = FVector::ZeroVector;
}

bool UAlphaMovementConfig::ShouldLimitAirControl(float DeltaTime, const FVector& FallAcceleration) const
{
	return false;
}

FVector UAlphaMovementConfig::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	FVector FallVelocity = Super::NewFallVelocity(InitialVelocity, Gravity, DeltaTime);
	FallVelocity.Z = FMath::Clamp(FallVelocity.Z, -AxisSpeedLimit, AxisSpeedLimit);
	return FallVelocity;
}

void UAlphaMovementConfig::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	Velocity.Z = FMath::Clamp(Velocity.Z, -AxisSpeedLimit, AxisSpeedLimit);
	// TODO: UpdateCrouching
}

void UAlphaMovementConfig::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	Velocity.Z = FMath::Clamp(Velocity.Z, -AxisSpeedLimit, AxisSpeedLimit);
	UpdateSurfaceFriction();
	// TODO: UpdateCrouching(DeltaSeconds, true);
}

void UAlphaMovementConfig::UpdateSurfaceFriction(bool bIsSliding)
{
	if (!IsFalling() && CurrentFloor.IsWalkableFloor())
	{
		FHitResult Hit;
		TraceCharacterFloor(Hit);
		SurfaceFriction = GetFrictionFromHit(Hit);
	}
	else
	{
		const bool bPlayerControlsMovedVert = Velocity.Z > JumpVelocity || Velocity.Z <= 0.0f;
		if (bPlayerControlsMovedVert)
			SurfaceFriction = 1.0f;
		else if (bIsSliding)
			SurfaceFriction = 0.25f;
	}
}

void UAlphaMovementConfig::PhysFalling(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
		return;

	FVector FallAcceleration = GetFallingLateralAcceleration(deltaTime);
	FallAcceleration.Z = 0.f;

	const bool bHasLimitedAirControl = ShouldLimitAirControl(deltaTime, FallAcceleration);
	float RemainingTime = deltaTime;

	while ((RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		float Tick = GetSimulationTimeStep(RemainingTime, Iterations);
		RemainingTime -= Tick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FVector OldVelocityWithRootMotion = Velocity;
		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		
		bJustTeleported = false;
		RestorePreAdditiveRootMotionVelocity();

		const FVector OldVelocity = Velocity;
		const float MaxDeceleration = GetMaxBrakingDeceleration();

		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			{
				TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
				Velocity.Z = 0.f;
				CalcVelocity(Tick, FallingLateralFriction, false, MaxDeceleration);
				Velocity.Z = OldVelocity.Z;
			}
		}

		const FVector Gravity(0.f, 0.f, GetGravityZ());
		float GravityTime = Tick;
		bool bEndingJumpForce = false;

		if (CharacterOwner->JumpForceTimeRemaining > 0.0f)
		{
			const float JumpForceTime = FMath::Min(CharacterOwner->JumpForceTimeRemaining, Tick);

			GravityTime = bApplyGravityWhileJumping ? Tick : FMath::Max(0.0f, Tick - JumpForceTime);
			CharacterOwner->JumpForceTimeRemaining -= JumpForceTime;

			if (CharacterOwner->JumpForceTimeRemaining <= 0.0f)
			{
				CharacterOwner->ResetJumpState();
				bEndingJumpForce = true;
			}
		}

		Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);

		if (OldVelocity.Z > 0.f && Velocity.Z <= 0.f && NumJumpApexAttempts < MaxJumpApexAttemptsPerSimulation)
		{
			const FVector DerivedAcceleration = (Velocity - OldVelocity) / Tick;

			if (!FMath::IsNearlyZero(DerivedAcceleration.Z))
			{
				const float TimeToApex = -OldVelocity.Z / DerivedAcceleration.Z;
				const float ApexTimeMin = 0.0001f;

				if (TimeToApex >= ApexTimeMin && TimeToApex < Tick)
				{
					const FVector ApexVelocity = OldVelocity + DerivedAcceleration * TimeToApex;

					Velocity = ApexVelocity;
					Velocity.Z = 0.f;

					RemainingTime += (Tick - TimeToApex);
					Tick = TimeToApex;
					
					Iterations--;
					NumJumpApexAttempts++;
				}
			}
		}

		ApplyRootMotionToVelocity(Tick);

		if (bNotifyApex && (Velocity.Z < 0.f))
		{
			bNotifyApex = false;
			NotifyJumpApex();
		}

		FVector AdjustedVelocity = 0.5f * (OldVelocityWithRootMotion + Velocity) * Tick;

		if (bEndingJumpForce && !bApplyGravityWhileJumping)
		{
			const float NonGravityTime = FMath::Max(0.f, Tick - GravityTime);
			AdjustedVelocity = (OldVelocityWithRootMotion * NonGravityTime) + (0.5f * (OldVelocityWithRootMotion + Velocity) * GravityTime);
		}

		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(AdjustedVelocity, PawnRotation, true, Hit);

		if (!HasValidData())
			return;

		float LastMoveTimeSlice = Tick;
		float SubTimeTickRemaining = Tick * (1.f - Hit.Time);

		if (Hit.bBlockingHit)
		{
			if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
			{
				RemainingTime += SubTimeTickRemaining;
				ProcessLanded(Hit, RemainingTime, Iterations);
				return;
			}
			else
			{
				if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(Tick, AdjustedVelocity, Hit))
				{
					const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
					FFindFloorResult FloorResult;

					FindFloor(PawnLocation, FloorResult, false);

					if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
					{
						RemainingTime += SubTimeTickRemaining;
						ProcessLanded(FloorResult.HitResult, RemainingTime, Iterations);
						return;
					}
				}

				HandleImpact(Hit, LastMoveTimeSlice, AdjustedVelocity);

				if (!HasValidData() || !IsFalling())
					return;

				FVector VelocityNoAirControl = OldVelocity;
				FVector AirControlAcceleration = Acceleration;

				if (bHasLimitedAirControl)
				{
					{
						TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
						TGuardValue<FVector> RestoreVelocity(Velocity, OldVelocity);

						Velocity.Z = 0.f;
						CalcVelocity(Tick, FallingLateralFriction, false, MaxDeceleration);
						VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, Velocity.Z);
						VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, GravityTime);
					}

					const bool bCheckLandingSpot = false;
					const FVector AirControlDeltaVelocity = LimitAirControl(LastMoveTimeSlice, AirControlAcceleration, Hit, bCheckLandingSpot) * LastMoveTimeSlice;
					AirControlAcceleration = (Velocity - VelocityNoAirControl) / Tick;
					AdjustedVelocity = (VelocityNoAirControl + AirControlDeltaVelocity) * LastMoveTimeSlice;
				}

				const FVector OldHitNormal = Hit.Normal;
				const FVector OldHitImpactNormal = Hit.ImpactNormal;
				FVector Delta = ComputeSlideVector(AdjustedVelocity, 1.f - Hit.Time, OldHitNormal, Hit);
				FVector DeltaStep = ComputeSlideVector(Velocity * Tick, 1.f - Hit.Time, OldHitNormal, Hit);

				if (SubTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
				{
					const FVector NewVelocity = (DeltaStep / SubTimeTickRemaining);
					Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? FVector(Velocity.X, Velocity.Y, Velocity.Z) : NewVelocity;
				}

				if (SubTimeTickRemaining > KINDA_SMALL_NUMBER && (Delta | AdjustedVelocity) > 0.f)
				{
					SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

					if (Hit.bBlockingHit)
					{
						LastMoveTimeSlice = SubTimeTickRemaining;
						SubTimeTickRemaining = SubTimeTickRemaining * (1.f - Hit.Time);

						if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
						{
							RemainingTime += SubTimeTickRemaining;
							ProcessLanded(Hit, RemainingTime, Iterations);
							return;
						}

						HandleImpact(Hit, LastMoveTimeSlice, Delta);

						if (!HasValidData() || !IsFalling())
							return;

						if (bHasLimitedAirControl && Hit.Normal.Z > VERTICAL_SLOPE_NORMAL_Z)
						{
							const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
							Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
						}

						FVector PreTwoWallDelta = Delta;
						TwoWallAdjust(Delta, Hit, OldHitNormal);

						if (bHasLimitedAirControl)
						{
							const bool bCheckLandingSpot = false;
							const FVector AirControlDeltaVelocity = LimitAirControl(SubTimeTickRemaining, AirControlAcceleration, Hit, bCheckLandingSpot) * SubTimeTickRemaining;

							if (FVector::DotProduct(AirControlDeltaVelocity, OldHitNormal) > 0.f)
								Delta += (AirControlDeltaVelocity * SubTimeTickRemaining);
						}

						if (SubTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
						{
							const FVector NewVelocity = (Delta / SubTimeTickRemaining);
							Velocity = HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocityWithIgnoreZAccumulate() ? FVector(Velocity.X, Velocity.Y, Velocity.Z) : NewVelocity;
						}

						// mega complicated bool which means the player is stuck between two surfaces that they can not walk on (i.e, a "ditch")
						bool bInDitch = ((OldHitImpactNormal.Z > 0.f) && (Hit.ImpactNormal.Z > 0.f) && (FMath::Abs(Delta.Z) <= KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f));
						SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

						if (Hit.Time == 0.f)
						{
							FVector SideDelta = (OldHitNormal + Hit.ImpactNormal).GetSafeNormal2D();

							if (SideDelta.IsNearlyZero())
								SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();

							SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit);
						}

						if (bInDitch || IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit) || Hit.Time == 0.f)
						{
							RemainingTime = 0.f;
							ProcessLanded(Hit, RemainingTime, Iterations);
							return;
						}

						if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && OldHitImpactNormal.Z >= GetWalkableFloorZ())
						{
							const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
							const float ZDistance = FMath::Abs(PawnLocation.Z - OldLocation.Z);
							const float MoveDist2DSQ = (PawnLocation - OldLocation).SizeSquared2D();

							if (ZDistance <= 0.2f * Tick && MoveDist2DSQ <= 4.f * Tick)
							{
								Velocity.X += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
								Velocity.Y += 0.25f * GetMaxSpeed() * (RandomStream.FRand() - 0.5f);
								Velocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
								Delta = Velocity * Tick;
								SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
							}
						}
					}
				}
			}
		}

		if (Velocity.SizeSquared2D() <= KINDA_SMALL_NUMBER * 10.f)
		{
			Velocity.X = 0.f;
			Velocity.Y = 0.f;
		}
	}
}

void UAlphaMovementConfig::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// UE4-COPY: void UCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)

	// Do not update velocity when using root motion or when SimulatedProxy and not simulating root motion - SimulatedProxy are repped their Velocity
	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME || (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}

	Friction = FMath::Max(0.0f, Friction);
	const float MaxAccel = GetMaxAcceleration();
	float MaxSpeed = GetMaxSpeed();

	// Player doesn't path follow
#if 0
	// Check if path following requested movement
	bool bZeroRequestedAcceleration = true;
	FVector RequestedAcceleration = FVector::ZeroVector;
	float RequestedSpeed = 0.0f;
	if (ApplyRequestedMove(DeltaTime, MaxAccel, MaxSpeed, Friction, BrakingDeceleration, RequestedAcceleration, RequestedSpeed))
	{
		RequestedAcceleration = RequestedAcceleration.GetClampedToMaxSize(MaxAccel);
		bZeroRequestedAcceleration = false;
	}
#endif

	if (bForceMaxAccel)
	{
		// Force acceleration at full speed.
		// In consideration order for direction: Acceleration, then Velocity, then Pawn's rotation.
		if (Acceleration.SizeSquared() > SMALL_NUMBER)
		{
			Acceleration = Acceleration.GetSafeNormal() * MaxAccel;
		}
		else
		{
			Acceleration = MaxAccel * (Velocity.SizeSquared() < SMALL_NUMBER ? UpdatedComponent->GetForwardVector() : Velocity.GetSafeNormal());
		}

		AnalogInputModifier = 1.0f;
	}

	MaxSpeed = FMath::Max(MaxSpeed * AnalogInputModifier, GetMinAnalogSpeed());

	// Apply braking or deceleration
	const bool bZeroAcceleration = Acceleration.IsNearlyZero();
	const bool bIsGroundMove = IsMovingOnGround() && bBrakingFrameTolerated;

	// Apply friction
	if (bIsGroundMove)
	{
		const bool bVelocityOverMax = IsExceedingMaxSpeed(MaxSpeed);
		const FVector OldVelocity = Velocity;

		const float ActualBrakingFriction = (bUseSeparateBrakingFriction ? BrakingFriction : Friction) * SurfaceFriction;
		ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, BrakingDeceleration);

		// Don't allow braking to lower us below max speed if we started above it.
		if (bVelocityOverMax && Velocity.SizeSquared() < FMath::Square(MaxSpeed) && FVector::DotProduct(Acceleration, OldVelocity) > 0.0f)
		{
			Velocity = OldVelocity.GetSafeNormal() * MaxSpeed;
		}
	}

	// Apply fluid friction
	if (bFluid)
	{
		Velocity = Velocity * (1.0f - FMath::Min(Friction * DeltaTime, 1.0f));
	}

	// Limit before
	Velocity.X = FMath::Clamp(Velocity.X, -AxisSpeedLimit, AxisSpeedLimit);
	Velocity.Y = FMath::Clamp(Velocity.Y, -AxisSpeedLimit, AxisSpeedLimit);

	// no clip
	if (bCheatFlying)
	{
		if (bZeroAcceleration)
		{
			Velocity = FVector(0.0f);
		}
		else
		{
			auto LookVec = CharacterOwner->GetControlRotation().Vector();
			auto LookVec2D = CharacterOwner->GetActorForwardVector();
			LookVec2D.Z = 0.0f;
			auto PerpendicularAccel = (LookVec2D | Acceleration) * LookVec2D;
			auto TangentialAccel = Acceleration - PerpendicularAccel;
			auto UnitAcceleration = Acceleration;
			auto Dir = UnitAcceleration.CosineAngle2D(LookVec);
			auto NoClipAccelClamp = AlphaCharacter->IsWalking() ? MaxAcceleration : 2.0f * MaxAcceleration; // inverted until we add crouch
			Velocity = (Dir * LookVec * PerpendicularAccel.Size2D() + TangentialAccel).GetClampedToSize(NoClipAccelClamp, NoClipAccelClamp);
		}
	}
	// walk move
	else
	{
		// Apply input acceleration
		if (!bZeroAcceleration)
		{
			// Clamp acceleration to max speed
			Acceleration = Acceleration.GetClampedToMaxSize2D(MaxSpeed);
			// Find veer
			const FVector AccelDir = Acceleration.GetSafeNormal2D();
			const float Veer = Velocity.X * AccelDir.X + Velocity.Y * AccelDir.Y;
			// Get add speed with air speed cap
			const float AddSpeed = (bIsGroundMove ? Acceleration : Acceleration.GetClampedToMaxSize2D(AirSpeedCap)).Size2D() - Veer;
			if (AddSpeed > 0.0f)
			{
				// Apply acceleration
				const float AccelerationMultiplier = bIsGroundMove ? GroundAccelerationModifier : AirAccelerationModifier;
				FVector CurrentAcceleration = Acceleration * AccelerationMultiplier * SurfaceFriction * DeltaTime;
				CurrentAcceleration = CurrentAcceleration.GetClampedToMaxSize2D(AddSpeed);
				Velocity += CurrentAcceleration;
			}
		}
	}

	// Limit after
	Velocity.X = FMath::Clamp(Velocity.X, -AxisSpeedLimit, AxisSpeedLimit);
	Velocity.Y = FMath::Clamp(Velocity.Y, -AxisSpeedLimit, AxisSpeedLimit);

	const float SpeedSq = Velocity.SizeSquared2D();

	// Dynamic step height code for allowing sliding on a slope when at a high speed
	if (SpeedSq <= MaxWalkSpeedCrouched * MaxWalkSpeedCrouched)
	{
		// If we're crouching or not sliding, just use max
		MaxStepHeight = DefaultStepHeight;
		SetWalkableFloorZ(DefaultWalkableFloorZ);
	}
	else
	{
		// Scale step/ramp height down the faster we go
		float Speed = FMath::Sqrt(SpeedSq);
		float SpeedScale = (Speed - MinSlopeSpeedModifier) / (MaxSlopeSpeedModifier - MinSlopeSpeedModifier);
		float SpeedMultiplier = FMath::Clamp(SpeedScale, 0.0f, 1.0f);
		SpeedMultiplier *= SpeedMultiplier;
		if (!IsFalling())
		{
			// If we're on ground, factor in friction.
			SpeedMultiplier = FMath::Max((1.0f - SurfaceFriction) * SpeedMultiplier, 0.0f);
		}
		MaxStepHeight = FMath::Lerp(DefaultStepHeight, MinStepHeight, SpeedMultiplier);
		SetWalkableFloorZ(FMath::Lerp(DefaultWalkableFloorZ, 0.9848f, SpeedMultiplier));
	}
}

bool UAlphaMovementConfig::CanAttemptJump() const
{
	bool bCanAttemptJump = IsJumpAllowed();

	if (IsMovingOnGround())
	{
		const float FloorZ = FVector(0.0f, 0.0f, 1.0f) | CurrentFloor.HitResult.ImpactNormal;
		const float WalkableFloor = GetWalkableFloorZ();
		bCanAttemptJump &= (FloorZ >= WalkableFloor) || FMath::IsNearlyEqual(FloorZ, WalkableFloor);
	}

	return bCanAttemptJump;
}

float UAlphaMovementConfig::GetMaxSpeed() const
{
	float Speed;

	if (AlphaCharacter->IsWalking() || AlphaCharacter->DoesWantToWalk())
		Speed = WalkMovementSpeed;
	else
		Speed = BaseMovementSpeed;

	return Speed;
}






















