// Fill out your copyright notice in the Description page of Project Settings.


#include "Specter.h"

const float ZOOM_STEP = 100;
const float MIN_ZOOM_DIST = 400;
const float MAX_ZOOM_DIST = 2000;

// Sets default values
ASpecter::ASpecter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	spring_arm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	camera = CreateDefaultSubobject<UCameraComponent>("Camera");

	spring_arm->SetupAttachment(RootComponent);
	camera->SetupAttachment(spring_arm, USpringArmComponent::SocketName); //TODO , USpringArmComponent::SocketName ??

	RootComponent->SetWorldLocation(FVector(0.0, 0.0, 200.0));
	spring_arm->bAbsoluteRotation = true;
	spring_arm->bDoCollisionTest = false; //Let objects to hide the pawn
	spring_arm->SetWorldRotation(FRotator(-60.0, -90.0, 0.0));
	spring_arm->TargetArmLength = 1200.0;

	MovementForce = 1000;

	should_move_left = 0;
	should_move_up = 0;

	this->OnDestroyed.AddDynamic(this, &ASpecter::Destroyed);
}

// Called when the game starts or when spawned
void ASpecter::BeginPlay()
{
	Super::BeginPlay();

	should_move_left = 0;
	should_move_up = 0;
}

// Called to bind functionality to input
void ASpecter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent = PlayerInputComponent;

	InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ASpecter::input_zoom_in);
	InputComponent->BindAction("ZoomOut", IE_Pressed, this, &ASpecter::input_zoom_out);

	InputComponent->BindAxis("MoveForward", this, &ASpecter::input_move_forward).bExecuteWhenPaused = true;
	InputComponent->BindAxis("MoveRight", this, &ASpecter::input_move_right).bExecuteWhenPaused = true;
}

// Called every frame
void ASpecter::Tick(float dt)
{
	Super::Tick(dt);

	auto location = GetActorLocation();
	auto new_location = FVector(
		location.X + should_move_left * MovementForce * dt,
		location.Y + should_move_up * MovementForce * dt,
		location.Z
	);

	SetActorLocation(new_location);
}

void ASpecter::Destroyed(AActor* DestroyedActor) {
	UE_LOG(LogTemp, Warning, TEXT("ASpecter Destroyed"));
}

//My methods

void ASpecter::input_move_right(float value) {
	should_move_left = value;
}

void ASpecter::input_move_forward(float value) {
	should_move_up = -value;
}

void ASpecter::input_zoom_in() {
	if (spring_arm->TargetArmLength - ZOOM_STEP > MIN_ZOOM_DIST) {
		spring_arm->TargetArmLength -= ZOOM_STEP;
	}
	else {
		spring_arm->TargetArmLength = MIN_ZOOM_DIST;
	}
}

void ASpecter::input_zoom_out() {
	if (spring_arm->TargetArmLength + ZOOM_STEP < MAX_ZOOM_DIST) {
		spring_arm->TargetArmLength += ZOOM_STEP;
	}
	else {
		spring_arm->TargetArmLength = MAX_ZOOM_DIST;
	}
}

void ASpecter::SetZoom(float Distance) {
	UE_LOG(LogTemp, Warning, TEXT("Zoom %f"), spring_arm->TargetArmLength);
	spring_arm->TargetArmLength = Distance;


	UE_LOG(LogTemp, Warning, TEXT("Zook %f"), spring_arm->TargetArmLength);
}