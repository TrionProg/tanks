// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "TankState.h"

#include "TankMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ETankMovementCommand : uint8
{
	Stand 				UMETA(DisplayName = "Tank stand at place"),
	MoveForward 		UMETA(DisplayName = "Tank moves forward"),
	MoveBackward		UMETA(DisplayName = "Tank moves backward"),
	RotateLeft			UMETA(DisplayName = "Tank turning left"),
	RotateRight			UMETA(DisplayName = "Tank turning right"),
};

UENUM(BlueprintType)
enum class ETankMovementInertia : uint8
{
	None 				UMETA(DisplayName = "No inertia"),
	MoveForward 		UMETA(DisplayName = "Tank moves forward with Acceleration"),
	MoveBackward		UMETA(DisplayName = "Tank moves backward with Acceleration"),
	RotateLeft			UMETA(DisplayName = "Visually Shake tank left with Acceleration, uses timer"),
	RotateRight			UMETA(DisplayName = "Visually Shake tank right with Acceleration, uses timer"),
};

//TODO ������� �������, �� �����-����� ����� ������������ ����������� � �����-������. ������ ������� �������� ����.

UCLASS()
class TANKS_API UTankMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	//GENERATED_UCLASS_BODY()
protected:
	bool IsMovementCommand;
	ETankMovementCommand MovementCommand;

	ETankMovementState MovementState;
	ETankMovementInertia MovementInertia;
	float MovementInertiaTimer;

	float Yaw;
	float CurrentSpeed;
	float CurrentSpinSpeed;
	float SpinShakeSpeed;
	float RotateOn;

	//UPROPERTY()
	//UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)

	//UPROPERTY()
	//uint8 RotateSteps;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float Speed;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float SpinSpeed;

	//TODO rotate to
public:	
	// Sets default values for this component's properties
	//UTankMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual bool ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotation) override;

public:	
	UTankMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SendMovementCommand(ETankMovementCommand NewMovementCommand);

	//void SetMovementState(ETankMovementState new_movement_state);
	void SetSpeed(float new_speed);
	void SetSpinSpeed(float new_speed);

	/*
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	virtual void RequestPathMove(const FVector& MoveInput) override;
	virtual bool CanStartPathFollowing() const override;
	virtual bool CanStopPathFollowing() const override;
	virtual float GetPathFollowingBrakingDistance(float MaxSpeed) const override;
	*/

	//Begin UMovementComponent Interface
	virtual float GetMaxSpeed() const override { return MaxSpeedForward; }

	/** Maximum velocity magnitude allowed for the controlled Pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
	float MaxSpeedForward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
	float MaxSpinSpeed;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
	float AccelerationForward;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
	float DecelerationForward;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
	float AccelerationSpin;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement)
	float DecelerationSpin;

	/*
	 * Setting affecting extra force applied when changing direction, making turns have less drift and become more responsive.
	 * Velocity magnitude is not allowed to increase, that only happens due to normal acceleration. It may decrease with large direction changes.
	 * Larger values apply extra force to reach the target direction more quickly, while a zero value disables any extra turn force.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement, meta = (ClampMin = "0", UIMin = "0"))
		float TurningBoost;

	*/

private:
	void EnableMovementInertia(ETankMovementState PrevMovementState);
	void DisableMovementInertia(bool ClearAcceleration);
	void SetMovementState(ETankMovementState NewMovementState);
protected:
	/** Update Velocity based on input. Also applies gravity. */
	//virtual void ApplyControlInputToVelocity(float DeltaTime);

	/** Prevent Pawn from leaving the world bounds (if that restriction is enabled in WorldSettings) */
	virtual bool LimitWorldBounds();

	/** Set to true when a position correction is applied. Used to avoid recalculating velocity when this occurs. */
	UPROPERTY(Transient)
		uint32 bPositionCorrected : 1;
	/*
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentHealth();

	
	void OnHealthUpdate();
	*/
};
