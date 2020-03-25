// Fill out your copyright notice in the Description page of Project Settings.


#include "TankMovementComponent.h"
#include "Engine/World.h"
#include "Tank.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"

#include "Math/UnrealMathUtility.h"

//NavMesh не юзает

/*
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

	ResetMoveState(); //зачем?
}
*/

UTankMovementComponent::UTankMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//MovementState = ETankMovementState::Stand;
	//RotateSteps = 0;

	IsMovementCommand = false;
	MovementCommand = ETankMovementCommand::Stand;
	MovementState = ETankMovementState::Stand;
	MovementInertia = ETankMovementInertia::None;
	MovementInertiaTimer = 0;

	Yaw = 0;
	CurrentSpeed = 0;
	CurrentSpinSpeed = 0;
	SpinShakeSpeed = 0;
	RotateOn = 0;

	MaxSpeedForward = 600.f;
	AccelerationForward = 1000.f;
	DecelerationForward = 200.f;

	MaxSpinSpeed = 30;
	AccelerationSpin = 90;
	DecelerationSpin = 180;

	//TurningBoost = 8.0f;
	bPositionCorrected = false;


	ResetMoveState(); //зачем?
}

// Called when the game starts
void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementState = ETankMovementState::Stand;
	IsMovementCommand = false;
	MovementCommand = ETankMovementCommand::Stand;
	MovementState = ETankMovementState::Stand;
	MovementInertia = ETankMovementInertia::None;
	MovementInertiaTimer = 0;

	Yaw = 0; //TODO rotate body..
	//RotateSteps = 0;


	
}

void UTankMovementComponent::TickComponent(float dt, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	if (ShouldSkipUpdate(dt)) {
		return;
	}

	Super::TickComponent(dt, TickType, ThisTickFunction);

	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !UpdatedComponent) {
		return;
	}

	auto Tank = (ATank*)PawnOwner; //TODO safe cast

	auto DesiredMovement = FVector(0, 0, 0);
	FRotator DesiredRotation = Tank->GetActorRotation();
	bool Rotated = false;

	auto Controller = Tank->GetController();

	if (!Controller) {
		return;
	}

	if (Controller->IsLocalController()) {
		if (Controller->IsLocalPlayerController()) {//On client side (player's pawn)
			if (IsMovementCommand) {
				IsMovementCommand = false;

				switch (MovementCommand) {
				case ETankMovementCommand::Stand: {
					if (MovementState != ETankMovementState::Stand) {
						UE_LOG(LogTemp, Warning, TEXT("Stand"));
						EnableMovementInertia(MovementState);
						SetMovementState(ETankMovementState::Stand);
					}
					break;
				}
				case ETankMovementCommand::MoveForward: {
					if (MovementState == ETankMovementState::Stand) {
						UE_LOG(LogTemp, Warning, TEXT("MoveForward"));
						if (MovementInertia == ETankMovementInertia::MoveForward) {//TODO 2 disable if inertia exists не факт, что надо инерцию выключать, тк это и анимация
							DisableMovementInertia(false); //Воспользуемся существующим ускорением
						} else if (MovementInertia == ETankMovementInertia::MoveBackward){
							//DisableMovementInertia(false);
						}

						SetMovementState(ETankMovementState::MoveForward);
					}

					break;
				}
				case ETankMovementCommand::MoveBackward: {
					if (MovementState == ETankMovementState::Stand) {
						UE_LOG(LogTemp, Warning, TEXT("MoveBackward"));
						if (MovementInertia == ETankMovementInertia::MoveBackward) {//TODO 2 disable if inertia exists не факт, что надо инерцию выключать, тк это и анимация
							DisableMovementInertia(false); //Воспользуемся существующим ускорением
						} else if (MovementInertia == ETankMovementInertia::MoveForward) {
							//DisableMovementInertia(false);
						}

						SetMovementState(ETankMovementState::MoveBackward);
					}

					break;
				}
				case ETankMovementCommand::RotateLeft: {
					if (MovementState != ETankMovementState::RotateLeft) {
						EnableMovementInertia(MovementState);
						SetMovementState(ETankMovementState::RotateLeft);

						if (RotateOn == 0) {
							RotateOn = -90;
						}else if (RotateOn > 0) {
							RotateOn = -(90 - RotateOn);
						}

						UE_LOG(LogTemp, Warning, TEXT("RotateLeft--- %f"), RotateOn);
					}else {
						//UE_LOG(LogTemp, Warning, TEXT("RotateLeft=== %f"), RotateOn);
						//RotateOn -= 90;
					}

					break;
				}
				case ETankMovementCommand::RotateRight: {
					if (MovementState != ETankMovementState::RotateRight) {
						EnableMovementInertia(MovementState);
						SetMovementState(ETankMovementState::RotateRight);

						if (RotateOn == 0) {
							RotateOn = 90;
						}else if (RotateOn < 0) {
							UE_LOG(LogTemp, Warning, TEXT("RotateRight %f"), RotateOn);
							RotateOn = (90 + RotateOn);
							UE_LOG(LogTemp, Warning, TEXT("RotateRight--- %f"), RotateOn);
						}
						UE_LOG(LogTemp, Warning, TEXT("RotateRight--- %f"), RotateOn);
					} else {
						//UE_LOG(LogTemp, Warning, TEXT("RotateRight=== %f"), RotateOn);
						//RotateOn += 90;
					}

					break;
				}
				}
			}
		}
		/*
		// apply input for local players but also for AI that's not following a navigation path at the moment
		if (Controller->IsLocalPlayerController() || Controller->IsFollowingAPath() || bUseAccelerationForPaths) {

			//UE_LOG(LogTemp, Warning, TEXT("ApplyControlInputToVelocity") );
			//ApplyControlInputToVelocity(DeltaTime);
		} else {
			// if it's not player controller, but we do have a controller, then it's AI and we need to limit the speed
			if (Velocity.IsNearlyZero(1e-6f)) {
				//TODO do not change if we need rotate
				if (MovementState != ETankMovementState::Stand) {
					SetMovementState(ETankMovementState::Stand);
				}
			} else if (IsExceedingMaxSpeed(MaxSpeed)) {
				Velocity = Velocity.GetUnsafeNormal() * MaxSpeed;

				SetMovementState(ETankMovementState::MoveForward);
			}
		}
		*/

		//LimitWorldBounds();
		//bPositionCorrected = false;

		/*
		switch (MovementState) {
		case ETankMovementState::MoveForward: {
			//TODO Rotate and move

			//Velocity = GetMaxSpeed();
			auto ForwardVector = Tank->GetForwardVector();
			DesiredMovement += Velocity * dt;

			break;
		}case ETankMovementState::MoveBackward: {
			//TODO Rotate and move

			auto ForwardVector = Tank->GetForwardVector();
			DesiredMovement += Velocity * -dt;

			break;
		}
		}
		*/


	}

	switch (MovementState) {
	case ETankMovementState::Stand: {
		//Velocity = FVector::ZeroVector;
		break;
	}
	case ETankMovementState::MoveForward: {
		if (Controller->IsLocalPlayerController()) { //Controlled by Player on Client
			CurrentSpeed += AccelerationForward * dt;

			if (CurrentSpeed > MaxSpeedForward) {
				CurrentSpeed = MaxSpeedForward;
			}
		}

		break;
	}
	case ETankMovementState::MoveBackward: {
		if (Controller->IsLocalPlayerController()) { //Controlled by Player on Client
			CurrentSpeed -= AccelerationForward * dt;

			if (CurrentSpeed < -MaxSpeedForward) {
				CurrentSpeed = -MaxSpeedForward;
			}

			//UE_LOG(LogTemp, Warning, TEXT("MoveBackward Speed %f %f"));
		}

		break;
	}
	case ETankMovementState::RotateLeft: {
		if (Controller->IsLocalPlayerController()) { //Controlled by Player on Client
			CurrentSpinSpeed -= AccelerationSpin * dt;

			if (CurrentSpinSpeed < -MaxSpinSpeed) {
				CurrentSpinSpeed = -MaxSpinSpeed;
			}
		}

		break;
	}
	case ETankMovementState::RotateRight: {
		if (Controller->IsLocalPlayerController()) { //Controlled by Player on Client
			CurrentSpinSpeed += AccelerationSpin * dt;

			if (CurrentSpinSpeed > MaxSpinSpeed) {
				CurrentSpinSpeed = MaxSpinSpeed;
			}
		}

		break;
	}
	}

	switch (MovementInertia) {
	case ETankMovementInertia::MoveForward: {
		//Тут выходит прикол, что танк может двигаться назад, а это ускорение будет помогать, а сравнение с 0 мешать. Надо отменить.
		CurrentSpeed -= DecelerationForward * dt;

		if (CurrentSpeed < 0) {
			MovementInertia = ETankMovementInertia::None;
			CurrentSpeed = 0;
		}

		break;
	}
	case ETankMovementInertia::MoveBackward: {
		CurrentSpeed += DecelerationForward * dt;

		if (CurrentSpeed > 0) {
			MovementInertia = ETankMovementInertia::None;
			CurrentSpeed = 0;
		}

		break;
	}
	case ETankMovementInertia::RotateLeft: {
		//TODO only visual, do not affects Yaw
		break;
	}
	case ETankMovementInertia::RotateRight: {
		//TODO only visual, do not affects Yaw
		break;
	}
	}

	if (CurrentSpeed != 0) {
		auto ForwardVector = Tank->GetForwardVector();
		Velocity = ForwardVector * CurrentSpeed;
		DesiredMovement = Velocity * dt;
	}else {
		Velocity = FVector::ZeroVector;
	}

	if (CurrentSpinSpeed != 0) {
		Rotated = true;

		auto TickSpinSpeed = CurrentSpinSpeed * dt;

		if (FMath::Abs(RotateOn) < FMath::Abs(TickSpinSpeed)) { //мы можем выровниться
			Yaw += RotateOn;
			RotateOn = 0;

			EnableMovementInertia(MovementState);
			SetMovementState(ETankMovementState::Stand);
		} else { //Мы должны остановиться и начать двигаться вперёд
			Yaw += TickSpinSpeed;
			RotateOn -= TickSpinSpeed;
		}

		if (Yaw > 180) {
			Yaw -= 360;
		}else if (Yaw < -180) {
			Yaw += 360;
		}

		DesiredRotation.Yaw = Yaw + 180;
	}

	//TODO and rotate
	

	//Надо получить Velocity(в случае управления самим игроком), DesiredMovement -- это Velocity*tick(иначе из-за ускорения).

	//он игрока, но сторона неизвестна
	if (!Controller->IsPlayerController()) {
		//auto rot = Controller->GetControlRotation();

		//DesiredRotation.Yaw = rot.Yaw - 90;

		//auto Angle = -calc_angle_between_vectors_2d(InputVectorNorm, ForwardVector);

		//UE_LOG(LogTemp, Warning, TEXT("ForwardVector %f %f %f %f"), ForwardVector.X, ForwardVector.Y, ForwardVector.Z, Angle);

		//auto TickSpinSpeed = SpinSpeed * dt;
		//bool MoveBackward = false;

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
				}*/
	}

	//TODO process always, check if rotation has been changed

	if (!DesiredMovement.IsNearlyZero() || Rotated) {
		bPositionCorrected = false;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(DesiredMovement, DesiredRotation, true, Hit);

		if (Hit.IsValidBlockingHit()){
			HandleImpact(Hit, dt, DesiredMovement);
			// Try to slide the remaining distance along the surface.
			SlideAlongSurface(DesiredMovement, 1.f - Hit.Time, Hit.Normal, Hit, true);
		}

		// Update velocity
		// We don't want position changes to vastly reverse our direction (which can happen due to penetration fixups etc)
		if (!bPositionCorrected){
			const FVector NewLocation = UpdatedComponent->GetComponentLocation();
			Velocity = ((NewLocation - OldLocation) / dt);
		}

		UpdateComponentVelocity();
	}
}

void UTankMovementComponent::SendMovementCommand(ETankMovementCommand NewMovementCommand) {
	IsMovementCommand = true;
	MovementCommand = NewMovementCommand;
}

void UTankMovementComponent::EnableMovementInertia(ETankMovementState PrevMovementState) {
	switch (PrevMovementState) {
	case ETankMovementState::MoveForward:
		MovementInertia = ETankMovementInertia::MoveForward;
		break;
	case ETankMovementState::MoveBackward:
		MovementInertia = ETankMovementInertia::MoveBackward;
		break;
	case ETankMovementState::RotateLeft:
		MovementInertia = ETankMovementInertia::RotateLeft;
		MovementInertiaTimer = 1;
		SpinShakeSpeed = CurrentSpinSpeed;
		CurrentSpinSpeed = 0;
		break;
	case ETankMovementState::RotateRight:
		MovementInertia = ETankMovementInertia::RotateRight;
		MovementInertiaTimer = 1;
		SpinShakeSpeed = CurrentSpinSpeed;
		CurrentSpinSpeed = 0;
		break;
	}
}

void UTankMovementComponent::DisableMovementInertia(bool ClearAcceleration) {
	if (ClearAcceleration) {
		switch (MovementInertia) {
		case ETankMovementInertia::MoveForward:
		case ETankMovementInertia::MoveBackward:
			CurrentSpeed = 0;
			break;
		case ETankMovementInertia::RotateLeft:
		case ETankMovementInertia::RotateRight:
			CurrentSpinSpeed = 0;
			break;
		}
	}

	MovementInertia = ETankMovementInertia::None;
	MovementInertiaTimer = 0;
}

void UTankMovementComponent::SetMovementState(ETankMovementState NewMovementState) {
	MovementState = NewMovementState;

	//TODO send command or play sound
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
		//Velocity.Z = FMath::Min(GetMaxSpeed(), WorldSettings->KillZ - CurrentLocation.Z + 2.0f);
		return true;
	}

	return false;
}

/*
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
*/

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