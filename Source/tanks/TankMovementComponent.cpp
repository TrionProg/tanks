// Fill out your copyright notice in the Description page of Project Settings.


#include "TankMovementComponent.h"
#include "Engine/World.h"
#include "Tank.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"

#include "Math/UnrealMathUtility.h"

//NavMesh не юзает

// Sets default values for this component's properties
UTankMovementComponent::UTankMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MovementState = ETankMovementState::Stand;
	RotateSteps = 0;

	MaxSpeed = 1200.f;
	Acceleration = 4000.f;
	Deceleration = 8000.f;
	TurningBoost = 8.0f;
	bPositionCorrected = false;

	//ResetMoveState(); //зачем?
}

/*
UTankMovementComponent::UTankMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxSpeed = 1200.f;
	Acceleration = 4000.f;
	Deceleration = 8000.f;
	TurningBoost = 8.0f;
	bPositionCorrected = false;

	ResetMoveState();
}
*/

// Called when the game starts
void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementState = ETankMovementState::Stand;
	RotateSteps = 0;


	
}


// Called every frame
void UTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	auto Tank = (ATank*)PawnOwner;

	auto DesiredMovement = FVector(0, 0, 0);
	FRotator DesiredRotation = Tank->GetActorRotation();

	auto Controller = Tank->GetController();

	if (Controller && Controller->IsLocalController())
	{
		// apply input for local players but also for AI that's not following a navigation path at the moment
		if (Controller->IsLocalPlayerController() == true || Controller->IsFollowingAPath() == false || bUseAccelerationForPaths)
		{
			ApplyControlInputToVelocity(DeltaTime);
		}
		// if it's not player controller, but we do have a controller, then it's AI
		// (that's not following a path) and we need to limit the speed
		else if (IsExceedingMaxSpeed(MaxSpeed) == true)
		{
			Velocity = Velocity.GetUnsafeNormal() * MaxSpeed;
		}

		LimitWorldBounds();
		bPositionCorrected = false;

		// Move actor
		FVector Delta = Velocity * DeltaTime;

		if (!Delta.IsNearlyZero(1e-6f))
		{
			const FVector OldLocation = UpdatedComponent->GetComponentLocation();
			const FQuat Rotation = UpdatedComponent->GetComponentQuat();

			FHitResult Hit(1.f);
			SafeMoveUpdatedComponent(Delta, Rotation, true, Hit);

			if (Hit.IsValidBlockingHit())
			{
				HandleImpact(Hit, DeltaTime, Delta);
				// Try to slide the remaining distance along the surface.
				SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
			}

			// Update velocity
			// We don't want position changes to vastly reverse our direction (which can happen due to penetration fixups etc)
			if (!bPositionCorrected)
			{
				const FVector NewLocation = UpdatedComponent->GetComponentLocation();
				Velocity = ((NewLocation - OldLocation) / DeltaTime);
			}
		}

		// Finalize
		UpdateComponentVelocity();
	}

	if (Controller) {
		if (!Controller->IsPlayerController()) {
			auto rot = Controller->GetControlRotation();

			DesiredRotation.Yaw = rot.Yaw - 90;

			//UE_LOG(LogTemp, Warning, TEXT("Rot %f %f %f"), rot.Pitch, rot.Yaw, rot.Roll);

			//auto rot2 = controller->GetDesiredRotation();
			//auto rot3 = controller->GetTransform.

			//UE_LOG(LogTemp, Warning, TEXT("Rot2 %f %f %f"), rot2.Pitch, rot2.Yaw, rot2.Roll);

			//auto InputVector = Tank->ConsumeMovementInputVector();

			//const FVector ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize(1.f);
			//UE_LOG(LogTemp, Warning, TEXT("ControlAcceleration %f %f %f"), ControlAcceleration.X, ControlAcceleration.Y, ControlAcceleration.Z);

			/*

			//auto InputVector = ConsumeInputVector();
			UE_LOG(LogTemp, Warning, TEXT("InputVector %f %f %f"), InputVector.X, InputVector.Y, InputVector.Z);

			//мб работает, но выдаёт большие цифры и они прыгают. Но X и Y кратны позиции. Кратно где-то 30. Но куда ехать учитывается
			auto InputVector2 = Tank->GetVelocity();
			UE_LOG(LogTemp, Warning, TEXT("InputVector2 %f %f %f"), InputVector2.X, InputVector2.Y, InputVector2.Z);

			//нули, мб т.к. нет NavMesh
			auto InputVector3 = controller->GetMoveGoalOffset(Tank);
			UE_LOG(LogTemp, Warning, TEXT("InputVector3 %f %f %f"), InputVector3.X, InputVector3.Y, InputVector3.Z);

			auto InputVector4 = controller->GetReplicatedMovement().LinearVelocity;
			UE_LOG(LogTemp, Warning, TEXT("InputVector4 %f %f %f"), InputVector4.X, InputVector4.Y, InputVector4.Z);

			auto InputVector5 = Tank->GetMoveGoalOffset(Tank);
			UE_LOG(LogTemp, Warning, TEXT("InputVector5 %f %f %f"), InputVector5.X, InputVector5.Y, InputVector5.Z);

			auto InputVector6 = Tank->K2_GetMovementInputVector();
			UE_LOG(LogTemp, Warning, TEXT("InputVector6 %f %f %f"), InputVector6.X, InputVector6.Y, InputVector6.Z);

			// = Tank->GetVelocity();
			auto InputVector7 = this->Velocity;
			UE_LOG(LogTemp, Warning, TEXT("InputVector7 %f %f %f"), InputVector7.X, InputVector7.Y, InputVector7.Z);
			*/

		}
	}
	/*
	auto InputVector = ConsumeInputVector();

	if (!InputVector.IsNearlyZero()) {
		auto InputVectorNorm = InputVector.GetClampedToMaxSize(1.0f);

		auto ForwardVector = Tank->GetForwardVector();

		//UE_LOG(LogTemp, Warning, TEXT("InputVector %f %f %f"), InputVector.X, InputVector.Y, InputVector.Z);

		auto Angle = -calc_angle_between_vectors_2d(InputVectorNorm, ForwardVector);

		//UE_LOG(LogTemp, Warning, TEXT("ForwardVector %f %f %f %f"), ForwardVector.X, ForwardVector.Y, ForwardVector.Z, Angle);

		auto TickSpinSpeed = SpinSpeed * dt;
		bool MoveBackward = false;
		*/

		//TODO добавить движение по дуге -- будет указана коненая точка, и мы рассчитываем угол с условием, что мы проделаем этот путь, и как мы за это время сможем повернуть
		//А навмеш, InputVector будет нам говорить, мол мы так не можем. Посмотрим

		/*
		if (Angle > 90+10 || Angle < -90-10) {//Двигаться задом выгоднее, Хотя если это САУ, то надо двигаться лобовой бронёй и пушкой к врагу
			MoveBackward = true;

			if (Angle < 0) {
				Angle = 180 + Angle;
			}else if (Angle > 0) {
				Angle = -180 + Angle;
			}
		}
		*/

	/*
		if (FMath::Abs(Angle) < FMath::Abs(TickSpinSpeed)) { //TODO мы можем выровниться
			DesiredRotation.Yaw += Angle;

			if (!MoveBackward) {
				if (MovementState != ETankMovementState::MoveForward) {
					SetMovementState(ETankMovementState::MoveForward);
				}
			} else {
				if (MovementState != ETankMovementState::MoveBackward) {
					SetMovementState(ETankMovementState::MoveBackward);
				}
			}
		}
		else { //Мы должны остановиться и начать двигаться вперёд
			if (Angle > 0) {
				if (MovementState != ETankMovementState::RotateRight) {
					SetMovementState(ETankMovementState::RotateRight);
				}

				DesiredRotation.Yaw += TickSpinSpeed;
			}
			else {
				if (MovementState != ETankMovementState::RotateLeft) {
					SetMovementState(ETankMovementState::RotateLeft);
				}

				DesiredRotation.Yaw -= TickSpinSpeed;
			}
		}
	} else {
		if (MovementState != ETankMovementState::Stand) {
			SetMovementState(ETankMovementState::Stand);
		}
	}

	if (DesiredRotation.Yaw > 180) {
		DesiredRotation.Yaw -= 360;
	}else if (DesiredRotation.Yaw < -180) {
		DesiredRotation.Yaw += 360;
	}

	DesiredMovement.Z = -10; //TODO tmp gravity, we can move then faling, and we need impulse, etc

	switch (MovementState) {
	case ETankMovementState::MoveForward: {
		//TODO Rotate and move

		auto ForwardVector = Tank->GetForwardVector();
		DesiredMovement += ForwardVector * dt * Speed;

		break;
	}case ETankMovementState::MoveBackward: {
		//TODO Rotate and move

		auto ForwardVector = Tank->GetForwardVector();
		DesiredMovement += ForwardVector * dt * -Speed;

		break;
	}
	}
	*/

	/*
	FHitResult Hit;
	SafeMoveUpdatedComponent(DesiredMovement, DesiredRotation, true, Hit);

	if (Hit.IsValidBlockingHit()) { // If we bumped into something, try to slide along it
		SlideAlongSurface(DesiredMovement, 1.f - Hit.Time, Hit.Normal, Hit);
	}
	*/

}

void UTankMovementComponent::SetMovementState(ETankMovementState new_movement_state) {
	MovementState = new_movement_state;
}

void UTankMovementComponent::SetSpeed(float new_speed) {
	Speed = new_speed;
}

void UTankMovementComponent::SetSpinSpeed(float new_speed) {
	SpinSpeed = new_speed;
}

/*

void UTankMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) {
	UE_LOG(LogTemp, Warning, TEXT("RequestDirectMove %f %f %f"), MoveVelocity.X, MoveVelocity.Y, MoveVelocity.Z);
}


void UTankMovementComponent::RequestPathMove(const FVector& MoveInput) {
	UE_LOG(LogTemp, Warning, TEXT("RequestPathMove %f %f %f"), MoveInput.X, MoveInput.Y, MoveInput.Z);
}

bool UTankMovementComponent::CanStartPathFollowing() const {
	return true;
}
bool UTankMovementComponent::CanStopPathFollowing() const{
	return true;
}

float UTankMovementComponent::GetPathFollowingBrakingDistance(float MaxiSpeed) const {
	return 500;
}
*/

bool UTankMovementComponent::LimitWorldBounds()
{
	AWorldSettings* WorldSettings = PawnOwner ? PawnOwner->GetWorldSettings() : NULL;
	if (!WorldSettings || !WorldSettings->bEnableWorldBoundsChecks || !UpdatedComponent)
	{
		return false;
	}

	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	if (CurrentLocation.Z < WorldSettings->KillZ)
	{
		Velocity.Z = FMath::Min(GetMaxSpeed(), WorldSettings->KillZ - CurrentLocation.Z + 2.0f);
		return true;
	}

	return false;
}

void UTankMovementComponent::ApplyControlInputToVelocity(float DeltaTime)
{
	const FVector ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed(MaxPawnSpeed);

	UE_LOG(LogTemp, Warning, TEXT("ControlAcceleration %f %f %f"), ControlAcceleration.X, ControlAcceleration.Y, ControlAcceleration.Z);

	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (Velocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp(DeltaTime * TurningBoost, 0.f, 1.f);
			Velocity = Velocity + (ControlAcceleration * Velocity.Size() - Velocity) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if (Velocity.SizeSquared() > 0.f)
		{
			const FVector OldVelocity = Velocity;
			const float VelSize = FMath::Max(Velocity.Size() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
			Velocity = Velocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if (bExceedingMaxSpeed && Velocity.SizeSquared() < FMath::Square(MaxPawnSpeed))
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = (IsExceedingMaxSpeed(MaxPawnSpeed)) ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * FMath::Abs(Acceleration) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);

	ConsumeInputVector();
}

bool UTankMovementComponent::ResolvePenetrationImpl(const FVector& Adjustment, const FHitResult& Hit, const FQuat& NewRotationQuat)
{
	bPositionCorrected |= Super::ResolvePenetrationImpl(Adjustment, Hit, NewRotationQuat);
	return bPositionCorrected;
}


//Network
/*
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

	
}
*/