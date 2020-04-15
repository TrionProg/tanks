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

	//virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;
public:
	// Sets default values for this pawn's properties
	ATank();

//===Util===
private:
	OptionPtr<UWorld> get_world();
	int32 GetPlayerId();

//===Zoom===
private:
	void input_zoom_in();
	void input_zoom_out();

//===Network===
protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//===Movement===
private:
	float prev_float_value;

	void input_move_forward(float value);
	void input_rotate_left();
	void input_rotate_right();

protected:
	//UFUNCTION(BlueprintCallable, Category = "Pawn|Input", meta = (Keywords = "AddInput"))
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

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
	
//===Shooting===
protected:
	UPROPERTY(EditDefaultsOnly, Category = Shooting)
	TSubclassOf<class AProjectile> ShootProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float ShootInterval;

	//TODO ammo, interval
private:
	//Works on client
	void input_shoot();

	//Player shoots, runs on server
	UFUNCTION(Server, Reliable)
	void OnShoot();

	//Works on server for all(players and bots)
	void OnShootOnServer();

	//Works on clients
	UFUNCTION(NetMulticast, Reliable)
	void OnShootMulticast(int32 ShootInstigator);

	//Works on clients and spawns visual projectile
	void OnShootOnClient();

//===Health===
protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StartHealth;

	UPROPERTY(EditDefaultsOnly, Category = Damage)
	TSubclassOf<class AFlyingText> DamageFlyingText;

	UFUNCTION()
	void OnRep_CurrentHealth();

public:
	//Only on server
	UFUNCTION(BlueprintCallable)
	bool ApplyDamage(float Damage, ETankDamageLocation DamageLocation);

	ETankDamageLocation CalcDamageLocation(UPrimitiveComponent* TankComp);

private:
	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	UFUNCTION(NetMulticast, Unreliable)
	void OnDamageMulticast(float damage);

	//Works on clients
	void OnDamageOnClient(float damage);

	//Works on server
	void OnDeathOnServer();

	//Works on clients
	UFUNCTION(NetMulticast, Reliable)
	void OnDeathMulticast();

	//Works on clients
	void OnDeathOnClient();

	//Works on client who is player
	void OnPlayerDeath();

public:
	float GetHealth();

};

//===Util===
float calc_angle_between_vectors_2d(FVector a, FVector b);
float calc_angle_between_vectors_2d_rad(FVector a, FVector b);