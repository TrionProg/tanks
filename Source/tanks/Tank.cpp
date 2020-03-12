// Fill out your copyright notice in the Description page of Project Settings.


#include "Tank.h"
#include "Runtime/Engine/Classes/Components/InputComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"


const float ZOOM_STEP = 100;
const float MIN_ZOOM_DIST = 400;
const float MAX_ZOOM_DIST = 2000;
const float DEFAULT_ZOOM_DIST = 2000;

//UE events and methods

// Sets default values
ATank::ATank()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = body;
	gun_muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));

	RootComponent = body;
	spring_arm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	camera = CreateDefaultSubobject<UCameraComponent>("Camera");

	gun_muzzle->SetupAttachment(RootComponent);
	spring_arm->SetupAttachment(RootComponent);
	camera->SetupAttachment(spring_arm, USpringArmComponent::SocketName); //TODO , USpringArmComponent::SocketName ??

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TankBodyModel(TEXT("StaticMesh'/Game/Models/panzer.panzer'"));
	if (TankBodyModel.Succeeded()) {
		UE_LOG(LogTemp, Warning, TEXT("LoadTankModel"));
		body->SetStaticMesh(TankBodyModel.Object);
	}

	body->SetWorldRotation(FRotator(0, 180, 0.0));


	spring_arm->bAbsoluteRotation = true;
	spring_arm->bDoCollisionTest = false; //Let objects to hide the pawn
	spring_arm->SetWorldRotation(FRotator(-60.0, -90.0, 0.0));
	spring_arm->TargetArmLength = DEFAULT_ZOOM_DIST;

	shot_interval = 1;

	this->OnDestroyed.AddDynamic(this, &ATank::Destroyed);
}

// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void ATank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent = PlayerInputComponent;

	InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ATank::on_zoom_in);//TODO may be and released
	InputComponent->BindAction("ZoomOut", IE_Pressed, this, &ATank::on_zoom_out);

	InputComponent->BindAction("RotateLeft", IE_Pressed, this, &ATank::on_rotate_left);
	InputComponent->BindAction("RotateRight", IE_Pressed, this, &ATank::on_rotate_right);
	InputComponent->BindAction("Shoot", IE_Pressed, this, &ATank::on_shoot);

	InputComponent->BindAxis("MoveForward", this, &ATank::on_move_forward).bExecuteWhenPaused = true;

	//InputComponent->BindAction("Pause", IE_Released, this, &ATank::on_key_space_release).bExecuteWhenPaused = true;
	//InputComponent->BindAction("Restart", IE_Released, this, &ATank::on_key_r_release).bExecuteWhenPaused = true;

}

// Called every frame
void ATank::Tick(float dt)
{
	Super::Tick(dt);



}

void ATank::Destroyed(AActor* DestroyedActor) {
	UE_LOG(LogTemp, Warning, TEXT("ASpectator Destroyed"));
}

//My methods

OptionPtr<UWorld> ATank::get_world() {
	return OptionPtr<UWorld>::new_unchecked(GetWorld());
}

void ATank::on_move_forward(float value) {
	//FVector force_to_add = movement_force * value * FVector(0, 1, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_left = -value;
}

void ATank::on_rotate_left() {
	//FVector force_to_add = movement_force * value * FVector(1, 0, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_up = value;
}

void ATank::on_rotate_right() {
	//FVector force_to_add = movement_force * value * FVector(1, 0, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_up = value;
}

void ATank::on_shoot() {
	//FVector force_to_add = movement_force * value * FVector(1, 0, 0);
	//auto root = (USceneComponent*)RootComponent;
	//root->AddLocalOffset(force_to_add);
	//should_move_up = value;
}

void ATank::on_zoom_in() {
	if (spring_arm->TargetArmLength - ZOOM_STEP > MIN_ZOOM_DIST) {
		spring_arm->TargetArmLength -= ZOOM_STEP;
	}
	else {
		spring_arm->TargetArmLength = MIN_ZOOM_DIST;
	}
}

void ATank::on_zoom_out() {
	if (spring_arm->TargetArmLength + ZOOM_STEP < MAX_ZOOM_DIST) {
		spring_arm->TargetArmLength += ZOOM_STEP;
	}
	else {
		spring_arm->TargetArmLength = MAX_ZOOM_DIST;
	}
}
