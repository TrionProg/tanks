// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TankState.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum class ETankMovementState : uint8
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
};

UENUM(BlueprintType)
enum class ETankRotationInertia : uint8
{
	None 				UMETA(DisplayName = "No inertia"),
	RotateLeft			UMETA(DisplayName = "Visually Shake tank left with Acceleration, uses timer"),
	RotateRight			UMETA(DisplayName = "Visually Shake tank right with Acceleration, uses timer"),
};

UENUM(BlueprintType)
enum class ETankDamageLocation : uint8
{
	Caterpillar 				UMETA(DisplayName = "Caterpillar is damaged(speed is lower)"),
	Board 						UMETA(DisplayName = "Projectile hit into Board"),
	Forehead					UMETA(DisplayName = "Projectile hit into Forehead"),
	Stern						UMETA(DisplayName = "Projectile hit into Stern"),
	Turret						UMETA(DisplayName = "Projectile hit into Turret"),
	Gun							UMETA(DisplayName = "Projectile hit into Gun(tank can not temporary shoot)"),
};

/*
UCLASS()
class TANKS_API ATankState : public APlayerState {
	GENERATED_BODY()

public:
	
	UPROPERTY()
	ETankMovement Movement;

	UPROPERTY()
	uint8 RotateSteps;

	//TODO rotate to

	UPROPERTY()
	uint8 Health;
};
*/