// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OptionPtr.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "TankState.h"

#include "Tank.generated.h"

UCLASS()
class TANKS_API ATank : public APawn
{
	GENERATED_BODY()
//UE variables
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* body;
	//class USkeletalMeshComponent* Mesh1P;

	USceneComponent* gun_muzzle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* spring_arm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* camera;

	//UPROPERTY(EditDefaultsOnly, Category = Projectile)
	//TSubclassOf<class AProjectile> ShootProjectile;

	UPROPERTY(EditAnyWhere)
	UShapeComponent* collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTankMovementComponent* movement_component;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float shot_interval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float start_health;
//UE events and methods
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void Destroyed(AActor* DestroyedActor);

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	//UFUNCTION(BlueprintCallable, Category = "Pawn|Input", meta = (Keywords = "AddInput"))
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

	//virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;
public:
	// Sets default values for this pawn's properties
	ATank();
//My methods
private:
	OptionPtr<UWorld> get_world();

	void input_move_forward(float value);
	void input_rotate_left();
	void input_rotate_right();

	void input_shoot();

	void input_zoom_in();
	void input_zoom_out();

//Network
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	//void shoot();
public:
	FVector GetForwardVector();
	FVector GetRightVector();

};

float calc_angle_between_vectors_2d(FVector a, FVector b);
float calc_angle_between_vectors_2d_rad(FVector a, FVector b);