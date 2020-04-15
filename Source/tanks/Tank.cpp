// Fill out your copyright notice in the Description page of Project Settings.


#include "Tank.h"
#include "Runtime/Engine/Classes/Components/InputComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "TankMovementComponent.h"
#include "TankState.h"
#include "Projectile.h"
#include "TankPlayerController.h"
#include "TankPlayerState.h"
#include "Specter.h"

#include "Math/UnrealMathUtility.h"


const float ZOOM_STEP = 100;
const float MIN_ZOOM_DIST = 400;
const float MAX_ZOOM_DIST = 2000;
const float DEFAULT_ZOOM_DIST = 2000;

const float EPS = 0.00001;

//UE events and methods

// Sets default values
ATank::ATank()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	auto box_collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Root"));
	box_collision->SetBoxExtent(FVector(220, 415, 150), false);
	collision = (UShapeComponent*)box_collision;
	RootComponent = collision;

	body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	gun_muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));

	RootComponent = collision;
	spring_arm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	camera = CreateDefaultSubobject<UCameraComponent>("Camera");

	movement_component = CreateDefaultSubobject<UTankMovementComponent>(TEXT("TankMovementComponent"));

	body->SetupAttachment(RootComponent);
	gun_muzzle->SetupAttachment(RootComponent);
	spring_arm->SetupAttachment(RootComponent);
	camera->SetupAttachment(spring_arm, USpringArmComponent::SocketName); //TODO , USpringArmComponent::SocketName ??

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankBodyModel(TEXT("StaticMesh'/Game/Models/panzer.panzer'"));
	if (TankBodyModel.Succeeded()) {
		UE_LOG(LogTemp, Warning, TEXT("LoadTankModel"));
		body->SetStaticMesh(TankBodyModel.Object);
	}

	body->SetWorldLocation(FVector(0, -35, -150.0));
	body->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	gun_muzzle->SetRelativeLocation(FVector(0, 500, 140.0));
	gun_muzzle->SetRelativeRotation(FRotator(0, 90, 0));

	spring_arm->bAbsoluteRotation = true;
	spring_arm->bDoCollisionTest = false; //Let objects to hide the pawn
	spring_arm->SetWorldRotation(FRotator(-60.0, -90.0, 0.0));
	spring_arm->TargetArmLength = DEFAULT_ZOOM_DIST;

	collision->SetWorldRotation(FRotator(0, YAW_OFFSET, 0.0));
	collision->SetCollisionProfileName(TEXT("Pawn"));
	collision->SetGenerateOverlapEvents(true);
	//collision->SetWorldLocation(FVector(0, 0, 150));

	movement_component->SetUpdatedComponent(collision);
	movement_component->SetIsReplicated(true);

	//Hitboxes
	left_caterpillar_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("left_caterpillar_hitbox"));
	left_caterpillar_hitbox->SetupAttachment(body);
	left_caterpillar_hitbox->SetWorldScale3D(FVector(1.1, 10.75, 2.75));
	left_caterpillar_hitbox->SetRelativeLocation(FVector(-185, 0, 90));
	left_caterpillar_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	right_caterpillar_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("right_caterpillar_hitbox"));
	right_caterpillar_hitbox->SetupAttachment(body);
	right_caterpillar_hitbox->SetWorldScale3D(FVector(1.1, 10.75, 2.75));
	right_caterpillar_hitbox->SetRelativeLocation(FVector(185, 0, 90));
	right_caterpillar_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	left_board_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("left_board_hitbox"));
	left_board_hitbox->SetupAttachment(body);
	left_board_hitbox->SetWorldScale3D(FVector(0.2, 10, 2.2));
	left_board_hitbox->SetRelativeLocation(FVector(-143, -25, 156));
	left_board_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	right_board_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("right_board_hitbox"));
	right_board_hitbox->SetupAttachment(body);
	right_board_hitbox->SetWorldScale3D(FVector(0.2, 10, 2.2));
	right_board_hitbox->SetRelativeLocation(FVector(145, -25, 156));
	right_board_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	forehead_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("forehead_hitbox"));
	forehead_hitbox->SetupAttachment(body);
	forehead_hitbox->SetWorldScale3D(FVector(4.5, 1.75, 1.75));
	forehead_hitbox->SetRelativeRotation(FRotator(0, 0, -45));
	forehead_hitbox->SetRelativeLocation(FVector(0, 295, 147));
	forehead_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	stern_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("stern_hitbox"));
	stern_hitbox->SetupAttachment(body);
	stern_hitbox->SetWorldScale3D(FVector(4.5, 0.5, 2.2));
	stern_hitbox->SetRelativeLocation(FVector(0, -330, 156));
	stern_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	turret_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("turret_hitbox"));
	turret_hitbox->SetupAttachment(body);
	turret_hitbox->SetWorldScale3D(FVector(4, 4.2, 1.5));
	turret_hitbox->SetRelativeLocation(FVector(0, 50, 270));
	turret_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	gun_hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("gun_hitbox"));
	gun_hitbox->SetupAttachment(body);
	gun_hitbox->SetWorldScale3D(FVector(0.8, 4.3, 0.8));
	gun_hitbox->SetRelativeLocation(FVector(0, 320, 260));
	gun_hitbox->SetCollisionProfileName(TEXT("TankHitbox"));

	SetReplicatingMovement(true);
	//ReplicatedMovement = true;

	//SetIsReplicated(true);

	//---Movement---
	prev_float_value = 0;

	//---Shooting---
	ShootInterval = 1;

	//---Health---
	StartHealth = 100;

	//ShootProjectile = AProjectile::GetClass();

	this->OnDestroyed.AddDynamic(this, &ATank::Destroyed);
}

// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();

	//---Movement---
	prev_float_value = 0;

	//---Shooting---
	ShootInterval = 1;

	//---Health---
	Health = StartHealth;
	

	//movement_component->SetSpeed(300);
	//movement_component->SetSpinSpeed(40);
	
}

// Called to bind functionality to input
void ATank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent = PlayerInputComponent;

	InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ATank::input_zoom_in);//TODO may be and released
	InputComponent->BindAction("ZoomOut", IE_Pressed, this, &ATank::input_zoom_out);

	InputComponent->BindAction("RotateLeft", IE_Pressed, this, &ATank::input_rotate_left);
	InputComponent->BindAction("RotateRight", IE_Pressed, this, &ATank::input_rotate_right);
	InputComponent->BindAction("Shoot", IE_Pressed, this, &ATank::input_shoot);

	InputComponent->BindAxis("MoveForward", this, &ATank::input_move_forward).bExecuteWhenPaused = true;

	//InputComponent->BindAction("Pause", IE_Released, this, &ATank::on_key_space_release).bExecuteWhenPaused = true;
	//InputComponent->BindAction("Restart", IE_Released, this, &ATank::on_key_r_release).bExecuteWhenPaused = true;

}

// Called every frame
void ATank::Tick(float dt)
{
	Super::Tick(dt);

#if WITH_SERVER_CODE

	/*
	//Ётот код почему-то работает.
	if (Role != ROLE_Authority)
	{
		auto pos = GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("ATank %f %f %f"), pos.X, pos.Y, pos.Z);
	}
	*/

	/*
	//Ётот код почему-то работает.
	if (!HasAuthority())
	{
		auto pos = GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("ATank %f %f %f"), pos.X, pos.Y, pos.Z);
	}
	*/

	/*
	switch (GetNetMode()) {
	case NM_Standalone: UE_LOG(LogTemp, Warning, TEXT("Standalone")); break;
	case NM_DedicatedServer: UE_LOG(LogTemp, Warning, TEXT("DedicatedServer")); break;
	case NM_ListenServer: UE_LOG(LogTemp, Warning, TEXT("ListenServer")); break;
	case NM_Client: UE_LOG(LogTemp, Warning, TEXT("Client")); break;
	case NM_MAX: UE_LOG(LogTemp, Warning, TEXT("MAX")); break;
	}
	*/

	/*
	//Ќе работает
	if (GetNetMode() == NM_DedicatedServer) {
		//movement_component->SetMovement(ETankMovement::MoveForward);
		auto pos = GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("ATank %f %f %f"), pos.X, pos.Y, pos.Z);
		//auto pos = GetActorLocation();
		//UE_LOG(LogTemp, Warning, TEXT("ATank %f %f %f"), pos.X, pos.Y, pos.Z);
	}
	*/


#endif
	/*
	//Ќе работает
	if (GetNetMode() == NM_DedicatedServer) {
		auto pos = GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("ATank %f %f %f"), pos.X, pos.Y, pos.Z);
	}
	*/
}

void ATank::Destroyed(AActor* DestroyedActor) {
	UE_LOG(LogTemp, Warning, TEXT("ATank Destroyed"));
}

UPawnMovementComponent* ATank::GetMovementComponent() const {
	//UE_LOG(LogTemp, Warning, TEXT("ATank GetMovementComponent"));
	return (UPawnMovementComponent*)movement_component;
}

//---Util---

OptionPtr<UWorld> ATank::get_world() {
	return OptionPtr<UWorld>::new_unchecked(GetWorld());
}

int32 ATank::GetPlayerId() {
	if (auto TankController = GetController()) {
		if (auto TankPlayerController = Cast<ATankPlayerController>(TankController)) {
			//if (ATankPlayerState* TankPlayerState = TankController->GetPlayerState()) { ќн не может вывести тип “.. какой же придурок
			if (auto TankPlayerState = TankController->GetPlayerState<ATankPlayerState>()) {
				return TankPlayerState->GetPlayerId();
			}
		}
	}

	return BOT_PLAYER_ID;
}

float calc_angle_between_vectors_2d(FVector a, FVector b) {
	return calc_angle_between_vectors_2d_rad(a, b) * (180.0f / (float)PI);
}

float calc_angle_between_vectors_2d_rad(FVector a, FVector b) {
	auto scalar_mul = a.X * b.X + a.Y * b.Y;
	auto pseudoscalar_mul = a.X * b.Y - a.Y * b.X;

	float angle = 0.0;

	if (scalar_mul < -1.0 + EPS) { //надо лететь точно назад, формирует 8ку когда векторы <--.--> как бонус
		angle = PI;
	}
	else if (scalar_mul < 1.0 - EPS) { //можем вычислить угол
		if (pseudoscalar_mul > 0.0) {
			angle = FMath::Acos(scalar_mul);
		}
		else {
			angle = -FMath::Acos(scalar_mul);
		}
	}

	return angle;
}

//---Movement---

void ATank::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) {
	UE_LOG(LogTemp, Warning, TEXT("ATank AddMovementInput"));
}

void ATank::input_move_forward(float value) {
	if (value != prev_float_value) {
		if (value > 0) {
			movement_component->SendMovementCommand(ETankMovementCommand::MoveForward);
		} else if (value < 0) {
			movement_component->SendMovementCommand(ETankMovementCommand::MoveBackward);
		} else {
			movement_component->SendMovementCommand(ETankMovementCommand::Stand);
		}

		prev_float_value = value;
	}
}

void ATank::input_rotate_left() {
	UE_LOG(LogTemp, Warning, TEXT("Input RotateLeft--"));
	movement_component->SendMovementCommand(ETankMovementCommand::RotateLeft);
}

void ATank::input_rotate_right() {
	movement_component->SendMovementCommand(ETankMovementCommand::RotateRight);
}

//---Zoom---

void ATank::input_zoom_in() {
	if (spring_arm->TargetArmLength - ZOOM_STEP > MIN_ZOOM_DIST) {
		spring_arm->TargetArmLength -= ZOOM_STEP;
	}
	else {
		spring_arm->TargetArmLength = MIN_ZOOM_DIST;
	}
}

void ATank::input_zoom_out() {
	if (spring_arm->TargetArmLength + ZOOM_STEP < MAX_ZOOM_DIST) {
		spring_arm->TargetArmLength += ZOOM_STEP;
	}
	else {
		spring_arm->TargetArmLength = MAX_ZOOM_DIST;
	}
}

//---Network---

void ATank::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate health.
	DOREPLIFETIME(ATank, Health);
}

//---Movement---

//Calls on clients
void ATank::OnMovementStateChanged(ETankMovementState MovementState) {
	UE_LOG(LogTemp, Warning, TEXT("OnMovementStateChanged"));
}

//Calls on clients
void ATank::OnMovementInertiaEnabled(ETankMovementInertia MovementInertia) {
	UE_LOG(LogTemp, Warning, TEXT("OnMovementInertiaEnabled"));
}

//Calls on clients
void ATank::OnRotationInertiaEnabled(ETankRotationInertia RotationInteria) {
	UE_LOG(LogTemp, Warning, TEXT("OnRotationInertiaEnabled"));
}

FVector ATank::GetForwardVector() {
	// find out which way is forward
	const FRotator Rotation = GetActorRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	return Direction;
}

FVector ATank::GetRightVector() {
	// find out which way is right
	const FRotator Rotation = GetActorRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get right vector 
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	return Direction;
}

//---Shooting---

//Works on client
void ATank::input_shoot() {
	if (auto controller = GetController()) {
		if (controller->IsLocalPlayerController()) {
			//TODO check if tank can to shoot

			OnShoot();
			OnShootOnClient();
		}
	}
}

//Player shoots, runs on server
void ATank::OnShoot_Implementation() {
	//TODO check if wtank can to shoot

	OnShootOnServer();
}

//Works on server for all(players and bots)
void ATank::OnShootOnServer() {
	if (auto world = get_world().match()) {
		const auto spawn_rotation = gun_muzzle->GetComponentRotation();
		const auto spawn_location = gun_muzzle->GetComponentLocation();

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		world->SpawnActor<AProjectile>(ShootProjectile, spawn_location, spawn_rotation, ActorSpawnParams);
	}

	OnShootMulticast(GetPlayerId());
}

void ATank::OnShootMulticast_Implementation(int32 ShootInstigator) {
	if (ShootInstigator != BOT_PLAYER_ID) {
		if (ShootInstigator == GetPlayerId()) {
			return;
		}
	}

	OnShootOnClient();
}

void ATank::OnShootOnClient() {
	if (auto world = get_world().match()) {
		const auto spawn_rotation = gun_muzzle->GetComponentRotation();
		const auto spawn_location = gun_muzzle->GetComponentLocation();

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		world->SpawnActor<AProjectile>(ShootProjectile, spawn_location, spawn_rotation, ActorSpawnParams);
	}
}

//---Health---

void ATank::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void ATank::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), Health);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (Health <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (Role == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), Health);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}


ETankDamageLocation ATank::CalcDamageLocation(UPrimitiveComponent* TankComp) {
	auto DamageLocation = ETankDamageLocation::Board;

	if (TankComp == left_caterpillar_hitbox || TankComp == right_caterpillar_hitbox) {
		DamageLocation = ETankDamageLocation::Caterpillar;
	}
	else if (TankComp == left_board_hitbox || TankComp == right_board_hitbox) {
		DamageLocation = ETankDamageLocation::Board;
	}
	else if (TankComp == forehead_hitbox) {
		DamageLocation = ETankDamageLocation::Forehead;
	}
	else if (TankComp == stern_hitbox) {
		DamageLocation = ETankDamageLocation::Stern;
	}
	else if (TankComp == turret_hitbox) {
		DamageLocation = ETankDamageLocation::Turret;
	}
	else if (TankComp == gun_hitbox) {
		DamageLocation = ETankDamageLocation::Gun;
	}

	return DamageLocation;
}

bool ATank::ApplyDamage(float Damage, ETankDamageLocation DamageLocation) {
	UE_LOG(LogTemp, Warning, TEXT("Damage.."));
	Health -= Damage;

	if (Health < 0) {
		OnDeathOnServer();
		OnDeathMulticast();

		return true;
	}

	return false;
}

void ATank::OnDeathOnServer() {
	if (auto TankController = GetController()) {
		if (auto TankPlayerController = Cast<ATankPlayerController>(TankController)) {
			//Spawn specter
			if (auto world = get_world().match()) {
				auto location = spring_arm->GetComponentLocation();

				auto rotation = FRotator(0.0, 90.0, 0.0);
				auto spawn_info = FActorSpawnParameters();
				spawn_info.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				ASpecter* specter = (ASpecter*)world->SpawnActor(ASpecter::StaticClass(), &location, &rotation, spawn_info);
				specter->SetZoom(spring_arm->TargetArmLength);

				TankPlayerController->Possess(specter);
			}

			TankPlayerController->OnDeathOnServer();
		}
	}

	//TODO Possess player to be spectator, set respawn counter to PlayerState and Controler
	Destroy();
}

void ATank::OnDeathMulticast_Implementation() {
	if (auto TankController = GetController()) {
		if (auto TankPlayerController = Cast<ATankPlayerController>(TankController)) {
			if (TankPlayerController->IsLocalPlayerController()) {
				OnPlayerDeath();
			}
		}
	}
	OnDeathOnClient();
}

void ATank::OnDeathOnClient() {
	//Explode..
	//Destroy();
}

void ATank::OnPlayerDeath() {
	UE_LOG(LogTemp, Warning, TEXT("You are dead"));
	//Unpossess, etc..
	//Destroy();
}

float ATank::GetHealth() {
	return Health;
}