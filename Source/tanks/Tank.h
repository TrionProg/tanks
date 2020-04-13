// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OptionPtr.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "TankState.h"

#include "Tank.generated.h"

const int32 BOT_PLAYER_ID = -1;

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

	UPROPERTY(EditAnyWhere)
	UShapeComponent* left_caterpillar_hitbox;
	UPROPERTY(EditAnyWhere)
	UShapeComponent* right_caterpillar_hitbox;
	UPROPERTY(EditAnyWhere)
	UShapeComponent* left_board_hitbox;
	UPROPERTY(EditAnyWhere)
	UShapeComponent* right_board_hitbox;
	UPROPERTY(EditAnyWhere)
	UShapeComponent* forehead_hitbox;
	UPROPERTY(EditAnyWhere)
	UShapeComponent* stern_hitbox;
	UPROPERTY(EditAnyWhere)
	UShapeComponent* turret_hitbox;
	UPROPERTY(EditAnyWhere)
	UShapeComponent* gun_hitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTankMovementComponent* movement_component;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float shot_interval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float start_health;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AProjectile> ShootProjectile;

	float prev_float_value;
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

	//Works on client
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

	//Player shoots, runs on server
	UFUNCTION(Server, Reliable)
	void OnShoot();

	//Works on server for all(players and bots)
	void OnShootOnServer();

	//Works on clients
	UFUNCTION(NetMulticast, Reliable)
	void OnShootMulticast(int32 ShootInstigator);
	//void OnShootMulticast(int32 instigator);

	int32 GetPlayerId();

	//void shoot();
public:
	FVector GetForwardVector();
	FVector GetRightVector();

	//Calls on clients
	//UFUNCTION(BlueprintImplementableEvent)
	virtual void OnMovementStateChanged(ETankMovementState MovementState);

	//Calls on clients
	//UFUNCTION(BlueprintImplementableEvent)
	virtual void OnMovementInertiaEnabled(ETankMovementInertia MovementInertia);

	//Calls on clients
	virtual void OnRotationInertiaEnabled(ETankRotationInertia RotationInteria);

	ETankDamageLocation CalcDamageLocation(UPrimitiveComponent* TankComp);

	//Only on server
	UFUNCTION(BlueprintCallable)
	bool ApplyDamage(float Damage, ETankDamageLocation DamageLocation);
};

float calc_angle_between_vectors_2d(FVector a, FVector b);
float calc_angle_between_vectors_2d_rad(FVector a, FVector b);