// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OptionPtr.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite) //EditAnywhere, Category = Projectile
	float shot_interval;
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
public:
	// Sets default values for this pawn's properties
	ATank();
//My methods
private:
	OptionPtr<UWorld> get_world();

	void on_move_forward(float value);
	void on_rotate_left();
	void on_rotate_right();

	void on_shoot();

	void on_zoom_in();
	void on_zoom_out();

	//void shoot();
/*
public:
	// Sets default values for this pawn's properties
	ATank();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
*/

};
