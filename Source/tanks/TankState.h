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