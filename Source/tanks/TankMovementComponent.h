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

USTRUCT()
struct FPlayerSynchronizationData/*: public FFastArraySerializerItem*/ {
	//GENERATED_BODY()
	//GENERATED_USTRUCT_BODY()
	GENERATED_BODY()
//public:
	UPROPERTY(Transient)
	FVector Position;
	UPROPERTY(Transient)
	float Yaw;
	UPROPERTY(Transient)
	ETankMovementState MovementState;
	UPROPERTY(Transient)
	ETankMovementInertia MovementInertia;
	UPROPERTY(Transient)
	float CurrentSpeed;
	UPROPERTY(Transient)
	float CurrentSpinSpeed;
	UPROPERTY(Transient)
	float RotateOn;

	//bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

const float SYNCHRONIZE_WITH_SERVER_DELAY = 3;
const float SYNCHRONIZE_WITH_SERVER_MAX_POS_ERROR = 30;
const float SYNCHRONIZE_WITH_SERVER_MAX_ROT_ERROR = 15;
const float YAW_OFFSET = 180;

//TODO Касаемо инерции, то вперёд-назад может существовать параллельно с влево-вправо. Отмена инерции вызывает баги.
//TODO Можно принимать инпут от игрока сразу при его действиях. Например, в SendCommands, там же он и шлёт серверу сообщения
//TODO Аналагично и на стороне сервера. Так мы сможем вытащить отправку сообщений из тиков и сдвинуть время назад, 
//и мб отправка сообщений занимает время тика, ломая тем самым чёткий интервал

UCLASS()
class TANKS_API UTankMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	//GENERATED_UCLASS_BODY()
protected:
	bool IsMovementCommand;
	ETankMovementCommand MovementCommand;

	UPROPERTY(ReplicatedUsing = OnRep_MovementState)
	ETankMovementState Movement_State;
	ETankMovementInertia MovementInertia;
	UPROPERTY(Replicated)
	ETankRotationInertia RotationInertia;
	float RotationInertiaTimer;

	float Yaw;
	float CurrentSpeed;
	float CurrentSpinSpeed;
	float SpinShakeSpeed;
	float RotateOn;

	//TODO по-хорошему этот счётчик должен быть только на стороне сервера
	float SynchronizationWithServerTimer;

	//UPROPERTY()
	//UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)

	//UPROPERTY()
	//uint8 RotateSteps;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float Speed;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float SpinSpeed;

	UFUNCTION()
	void OnRep_MovementState();

	//void OnMovementState();

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
	void ProcessCommandsOfUser();
	void ProcessCommandsOnServer();

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

	UFUNCTION(Server, Reliable)
	void OnMovementStateChanged(ETankMovementState NewMovementState);

	UFUNCTION(Client, Unreliable) //Not releable -- we just help to client
	void SynchronizePlayerWithClient(FPlayerSynchronizationData SyncData);

	//void SynchronizePlayerWithClient(FVector Pos);

	void SynchronizePlayerWithClientTick(float dt);

	void CorrectMovement(FVector NewPosition, ETankMovementState NewMovementState, ETankMovementInertia NewMovementInertia, float NewCurrentSpeed);

	void CorrectRotation(float NewYaw, float NewCurrentSpinSpeed, float NewRotateOn);

	//Works on client for other players and bots
	void SimulateMovement();
	/*
	 * Setting affecting extra force applied when changing direction, making turns have less drift and become more responsive.
	 * Velocity magnitude is not allowed to increase, that only happens due to normal acceleration. It may decrease with large direction changes.
	 * Larger values apply extra force to reach the target direction more quickly, while a zero value disables any extra turn force.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FloatingPawnMovement, meta = (ClampMin = "0", UIMin = "0"))
		float TurningBoost;

	*/

private:
	void EnableInertia(ETankMovementState PrevMovementState);
	void DisableMovementInertia(bool ClearAcceleration);
	void DisableRotationInertia();
	void SetMovementState(ETankMovementState NewMovementState);
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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
