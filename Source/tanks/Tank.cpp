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

	spring_arm->bAbsoluteRotation = true;
	spring_arm->bDoCollisionTest = false; //Let objects to hide the pawn
	spring_arm->SetWorldRotation(FRotator(-60.0, -90.0, 0.0));
	spring_arm->TargetArmLength = DEFAULT_ZOOM_DIST;

	collision->SetWorldRotation(FRotator(0, 180, 0.0));
	collision->SetCollisionProfileName(TEXT("Pawn"));
	collision->SetGenerateOverlapEvents(true);
	//collision->SetWorldLocation(FVector(0, 0, 150));

	movement_component->SetUpdatedComponent(collision);
	movement_component->SetIsReplicated(true);

	SetReplicatingMovement(true);
	//ReplicatedMovement = true;

	//SetIsReplicated(true);

	shot_interval = 1;
	start_health = 3;

	this->OnDestroyed.AddDynamic(this, &ATank::Destroyed);
}

// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();

	Health = start_health;

	movement_component->SetSpeed(300);
	movement_component->SetSpinSpeed(40);
	
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
	//Этот код почему-то работает.
	if (Role != ROLE_Authority)
	{
		auto pos = GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("ATank %f %f %f"), pos.X, pos.Y, pos.Z);
	}
	*/

	/*
	//Этот код почему-то работает.
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
	//Не работает
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
	//Не работает
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

void ATank::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) {
	UE_LOG(LogTemp, Warning, TEXT("ATank AddMovementInput"));
}

//My methods

OptionPtr<UWorld> ATank::get_world() {
	return OptionPtr<UWorld>::new_unchecked(GetWorld());
}

void ATank::input_move_forward(float value) {
	if (value != 0) {
		UE_LOG(LogTemp, Warning, TEXT("ATank Move %f"), value);
		AddMovementInput(FVector(-100, 0, 0));
	}
	/*
	if (value > 0) {
		movement_component->SetMovement(ETankMovement::MoveForward);
	}else if (value < 0) {
		movement_component->SetMovement(ETankMovement::MoveBackward);
	}else {
		movement_component->SetMovement(ETankMovement::Stand);
	}
	*/
	//FVector force_to_add = movement_force * value * FVector(0, 1, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_left = -value;
}

void ATank::input_rotate_left() {
	//FVector force_to_add = movement_force * value * FVector(1, 0, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_up = value;
}

void ATank::input_rotate_right() {
	//FVector force_to_add = movement_force * value * FVector(1, 0, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_up = value;
}

void ATank::input_shoot() {
	//FVector force_to_add = movement_force * value * FVector(1, 0, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_up = value;
}

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

//Network

void ATank::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate health.
	DOREPLIFETIME(ATank, Health);
}

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