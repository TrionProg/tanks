// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Specter.generated.h"

UCLASS()
class TANKS_API ASpecter : public APawn
{
	GENERATED_BODY()
//UE variables
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* spring_arm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementForce;

//UE events and methods
protected:
	UFUNCTION()
	void Destroyed(AActor* DestroyedActor);

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
public:
	// Sets default values for this pawn's properties
	ASpecter();

// My variables
protected:
	float should_move_left;
	float should_move_up;
//My methods and events
private:
	void input_zoom_in();
	void input_zoom_out();

	void input_move_forward(float value);
	void input_move_right(float value);

public:
	void SetZoom(float Distance);
};
