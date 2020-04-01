// Fill out your copyright notice in the Description page of Project Settings.


#include "TankMovementComponent.h"
#include "Engine/World.h"
#include "Tank.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"
#include "Net/UnrealNetwork.h"

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
	Movement_State = ETankMovementState::Stand;
	MovementInertia = ETankMovementInertia::None;
	RotationInertia = ETankRotationInertia::None;
	RotationInertiaTimer = 0;

	Yaw = 0;
	CurrentSpeed = 0;
	CurrentSpinSpeed = 0;
	SpinShakeSpeed = 0;
	RotateOn = 0;

	MaxSpeedForward = 1000.f;
	AccelerationForward = 500;
	DecelerationForward = 500;

	MaxSpinSpeed = 60;
	AccelerationSpin = 90;
	DecelerationSpin = 180;

	//TurningBoost = 8.0f;
	bPositionCorrected = false;

	SynchronizationWithServerTimer = 0;


	ResetMoveState(); //зачем?
}

// Called when the game starts
void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	//TODO в чём отличие от конструктора? Как установить заданный актором поворот?

	Movement_State = ETankMovementState::Stand;
	IsMovementCommand = false;
	MovementCommand = ETankMovementCommand::Stand;
	Movement_State = ETankMovementState::Stand;
	MovementInertia = ETankMovementInertia::None;
	RotationInertia = ETankRotationInertia::None;
	RotationInertiaTimer = 0;

	Yaw = 0; //TODO rotate body..
	//RotateSteps = 0;

	SynchronizationWithServerTimer = 0;


	
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

	if (Controller->IsLocalPlayerController()) {
		ProcessCommandsOfUser();
	}

	if (Controller->HasAuthority()) {
		ProcessCommandsOnServer();
		/*
		if (Controller->IsPlayerController()) {
			auto pos = Tank->GetActorLocation();
			FRotator rot = Tank->GetActorRotation();
			UE_LOG(LogTemp, Warning, TEXT("Pos On Server %f %f %f, %f %f %f, %f"), pos.X, pos.Y, pos.Z, rot.Pitch, rot.Yaw, rot.Roll, RotateOn);

			switch (MovementState) {
			case ETankMovementState::Stand:
				UE_LOG(LogTemp, Warning, TEXT("ETankMovementState::Stand"));
				break;
			case ETankMovementState::MoveForward:
				UE_LOG(LogTemp, Warning, TEXT("ETankMovementState::MoveForward"));
				break;
			case ETankMovementState::MoveBackward:
				UE_LOG(LogTemp, Warning, TEXT("ETankMovementState::MoveBackward"));
				break;
			case ETankMovementState::RotateLeft:
				UE_LOG(LogTemp, Warning, TEXT("ETankMovementState::RotateLeft"));
				break;
			case ETankMovementState::RotateRight:
				UE_LOG(LogTemp, Warning, TEXT("ETankMovementState::RotateRight"));
				break;
			}
		}
		*/
	}

	//TODO this should works different on replication. How to check, if it is replication?
	switch (Movement_State) {
	case ETankMovementState::Stand: {
		//Velocity = FVector::ZeroVector;
		break;
	}
	case ETankMovementState::MoveForward: {
		CurrentSpeed += AccelerationForward * dt;

		if (CurrentSpeed > MaxSpeedForward) {
			CurrentSpeed = MaxSpeedForward;
		}

		break;
	}
	case ETankMovementState::MoveBackward: {
		CurrentSpeed -= AccelerationForward * dt;

		if (CurrentSpeed < -MaxSpeedForward) {
			CurrentSpeed = -MaxSpeedForward;
		}

		break;
	}
	case ETankMovementState::RotateLeft: {
		CurrentSpinSpeed -= AccelerationSpin * dt;

		if (CurrentSpinSpeed < -MaxSpinSpeed) {
			CurrentSpinSpeed = -MaxSpinSpeed;
		}

		break;
	}
	case ETankMovementState::RotateRight: {
		CurrentSpinSpeed += AccelerationSpin * dt;

		if (CurrentSpinSpeed > MaxSpinSpeed) {
			CurrentSpinSpeed = MaxSpinSpeed;
		}

		break;
	}
	}

	switch (MovementInertia) {
	case ETankMovementInertia::MoveForward: {
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
	}

	switch (RotationInertia) {
	case ETankRotationInertia::RotateLeft: {
		//TODO only visual, do not affects Yaw
		RotationInertia = ETankRotationInertia::None;
		break;
	}
	case ETankRotationInertia::RotateRight: {
		//TODO only visual, do not affects Yaw
		RotationInertia = ETankRotationInertia::None;
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

			EnableInertia(Movement_State);
			SetMovementState(ETankMovementState::Stand);
		} else { //Мы должны остановиться и начать двигаться вперёд
			/* //надеюсь, ошибок во флотах не будет
			if (RotateOn > 0) {
				Yaw += TickSpinSpeed;
				RotateOn -= TickSpinSpeed;

				if (RotateOn <= 0) {//проехали
					Yaw += -RotateOn;
					RotateOn = 0;
					EnableInertia(MovementState);
					SetMovementState(ETankMovementState::Stand);
				}
			}
			*/
			Yaw += TickSpinSpeed;
			RotateOn -= TickSpinSpeed;
		}

		//if (Controller->HasAuthority()) {
			UE_LOG(LogTemp, Warning, TEXT("NewYaw %f"), Yaw);
		//}

		if (Yaw > 180) {
			Yaw -= 360;
		}else if (Yaw < -180) {
			Yaw += 360;
		}

		DesiredRotation.Yaw = Yaw + YAW_OFFSET;
	}

	if (!DesiredMovement.IsNearlyZero() || Rotated) {

		/*
		if (Controller->HasAuthority()) {
			ProcessCommandsOnServer();

			if (Controller->IsPlayerController()) {
				UE_LOG(LogTemp, Warning, TEXT("DesiredMovement %f %f %f, %f %f %f"), DesiredMovement.X, DesiredMovement.Y, DesiredMovement.Z, DesiredRotation.Pitch, DesiredRotation.Yaw, DesiredRotation.Roll);
			}
		}
		*/

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

	if (Controller->HasAuthority()) {
		if (Controller->IsPlayerController()) {
			SynchronizePlayerWithClientTick(dt);
		}
	}
}

void UTankMovementComponent::SynchronizePlayerWithClientTick(float dt) {
	SynchronizationWithServerTimer += dt;

	if (SynchronizationWithServerTimer > SYNCHRONIZE_WITH_SERVER_DELAY) {
		SynchronizationWithServerTimer = 0;

		auto Tank = (ATank*)PawnOwner; //TODO safe cast
		auto Controller = Tank->GetController();

		if (!Controller) {
			return;
		}

		//FPlayerSynchronizationData SyncData;

		//SyncData.Position = Tank->GetActorLocation();

		FPlayerSynchronizationData SyncData = FPlayerSynchronizationData{
			Tank->GetActorLocation(),
			Yaw,
			Movement_State,
			MovementInertia,
			CurrentSpeed,
			CurrentSpinSpeed, //А можно взять, и забыть эту строчку, не инициализовав поле, например, добавить поле, и всё, компилятор скажет Ок -- С++ же
			RotateOn
		};

		/*
		SyncData.Position = GetActorLocation();
		SyncData.RotYaw = Yaw;
		SyncData.MovementState = MovementState;
		SyncData.MovementInertia = MovementInertia;
		SyncData.CurrentSpeed = CurrentSpeed;
		SyncData.CurrentSpinSpeed = CurrentSpinSpeed;
		SyncData.RotateOn = RotateOn;
		*/

		SynchronizePlayerWithClient(SyncData);
	}
}

void UTankMovementComponent::SendMovementCommand(ETankMovementCommand NewMovementCommand) {
	IsMovementCommand = true;
	MovementCommand = NewMovementCommand;
}

void UTankMovementComponent::ProcessCommandsOfUser() {
	if (IsMovementCommand) {
		IsMovementCommand = false;

		switch (MovementCommand) {
		case ETankMovementCommand::Stand: {
			//Нельзя прервать операцию поворота
			if (Movement_State == ETankMovementState::MoveForward || Movement_State == ETankMovementState::MoveBackward) {
				//UE_LOG(LogTemp, Warning, TEXT("Stand"));
				EnableInertia(Movement_State);
				SetMovementState(ETankMovementState::Stand);
			}
			break;
		}
		case ETankMovementCommand::MoveForward: {
			if (Movement_State == ETankMovementState::Stand) {
				//UE_LOG(LogTemp, Warning, TEXT("MoveForward"));
				if (MovementInertia == ETankMovementInertia::MoveForward) {//TODO 2 disable if inertia exists не факт, что надо инерцию выключать, тк это и анимация
					DisableMovementInertia(false); //Воспользуемся существующим ускорением
				}

				SetMovementState(ETankMovementState::MoveForward);
			}
			else {
				//UE_LOG(LogTemp, Warning, TEXT("Can not MoveForward"));
			}

			break;
		}
		case ETankMovementCommand::MoveBackward: {
			if (Movement_State == ETankMovementState::Stand) {
				//UE_LOG(LogTemp, Warning, TEXT("MoveBackward"));
				if (MovementInertia == ETankMovementInertia::MoveBackward) {//TODO 2 disable if inertia exists не факт, что надо инерцию выключать, тк это и анимация
					DisableMovementInertia(false); //Воспользуемся существующим ускорением
				}

				SetMovementState(ETankMovementState::MoveBackward);
			}

			break;
		}
		case ETankMovementCommand::RotateLeft: {
			if (Movement_State != ETankMovementState::RotateLeft) {
				EnableInertia(Movement_State);
				SetMovementState(ETankMovementState::RotateLeft);

				if (RotateOn == 0) {
					RotateOn = -90;
				}
				else if (RotateOn > 0) {
					RotateOn = -(90 - RotateOn);
				}

				//UE_LOG(LogTemp, Warning, TEXT("RotateLeft--- %f"), RotateOn);
			}

			break;
		}
		case ETankMovementCommand::RotateRight: {
			if (Movement_State != ETankMovementState::RotateRight) {
				EnableInertia(Movement_State);
				SetMovementState(ETankMovementState::RotateRight);

				if (RotateOn == 0) {
					RotateOn = 90;
				}
				else if (RotateOn < 0) {
					RotateOn = (90 + RotateOn);
				}

				//UE_LOG(LogTemp, Warning, TEXT("RotateRight--- %f"), RotateOn);
			}

			break;
		}
		}
	}
}

void UTankMovementComponent::ProcessCommandsOnServer() {
	if (IsMovementCommand) {
		IsMovementCommand = false;

		switch (MovementCommand) {
		case ETankMovementCommand::Stand: {
			//Нельзя прервать операцию поворота
			if (Movement_State == ETankMovementState::MoveForward || Movement_State == ETankMovementState::MoveBackward) {
				//UE_LOG(LogTemp, Warning, TEXT("Stand"));
				EnableInertia(Movement_State);
				SetMovementState(ETankMovementState::Stand);
			}
			break;
		}
		case ETankMovementCommand::MoveForward: {
			if (Movement_State == ETankMovementState::Stand) {
				//UE_LOG(LogTemp, Warning, TEXT("MoveForward"));
				if (MovementInertia == ETankMovementInertia::MoveForward) {//TODO 2 disable if inertia exists не факт, что надо инерцию выключать, тк это и анимация
					DisableMovementInertia(false); //Воспользуемся существующим ускорением
				}

				SetMovementState(ETankMovementState::MoveForward);
			}
			else {
				//UE_LOG(LogTemp, Warning, TEXT("Can not MoveForward"));
			}

			break;
		}
		case ETankMovementCommand::MoveBackward: {
			if (Movement_State == ETankMovementState::Stand) {
				//UE_LOG(LogTemp, Warning, TEXT("MoveBackward"));
				if (MovementInertia == ETankMovementInertia::MoveBackward) {//TODO 2 disable if inertia exists не факт, что надо инерцию выключать, тк это и анимация
					DisableMovementInertia(false); //Воспользуемся существующим ускорением
				}

				SetMovementState(ETankMovementState::MoveBackward);
			}

			break;
		}
		case ETankMovementCommand::RotateLeft: {
			if (Movement_State != ETankMovementState::RotateLeft) {
				EnableInertia(Movement_State);
				SetMovementState(ETankMovementState::RotateLeft);

				if (RotateOn == 0) {
					RotateOn = -90;
				}
				else if (RotateOn > 0) {
					RotateOn = -(90 - RotateOn);
				}

				//UE_LOG(LogTemp, Warning, TEXT("RotateLeft--- %f"), RotateOn);
			}

			break;
		}
		case ETankMovementCommand::RotateRight: {
			if (Movement_State != ETankMovementState::RotateRight) {
				EnableInertia(Movement_State);
				SetMovementState(ETankMovementState::RotateRight);

				if (RotateOn == 0) {
					RotateOn = 90;
				}
				else if (RotateOn < 0) {
					RotateOn = (90 + RotateOn);
				}

				//UE_LOG(LogTemp, Warning, TEXT("RotateRight--- %f"), RotateOn);
			}

			break;
		}
		}
	}
}

void UTankMovementComponent::EnableInertia(ETankMovementState PrevMovementState) {
	auto Tank = (ATank*)PawnOwner; //TODO safe cast
	auto Controller = Tank->GetController();

	switch (PrevMovementState) {
	case ETankMovementState::MoveForward:
		MovementInertia = ETankMovementInertia::MoveForward;

		if (Controller->IsLocalController()) {
			Tank->OnMovementInertiaEnabled(MovementInertia);
		}

		break;
	case ETankMovementState::MoveBackward:
		MovementInertia = ETankMovementInertia::MoveBackward;

		if (Controller->IsLocalController()) {
			Tank->OnMovementInertiaEnabled(MovementInertia);
		}

		break;
	case ETankMovementState::RotateLeft:
		RotationInertia = ETankRotationInertia::RotateLeft;
		RotationInertiaTimer = 1;
		SpinShakeSpeed = CurrentSpinSpeed;
		CurrentSpinSpeed = 0;

		if (Controller->IsLocalController()) {
			Tank->OnRotationInertiaEnabled(RotationInertia);
		}

		break;
	case ETankMovementState::RotateRight:
		RotationInertia = ETankRotationInertia::RotateRight;
		RotationInertiaTimer = 1;
		SpinShakeSpeed = CurrentSpinSpeed;
		CurrentSpinSpeed = 0;

		if (Controller->IsLocalController()) {
			Tank->OnRotationInertiaEnabled(RotationInertia);
		}
		
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
		}
	}

	MovementInertia = ETankMovementInertia::None;
}

void UTankMovementComponent::DisableRotationInertia() {
	RotationInertia = ETankRotationInertia::None;
	RotationInertiaTimer = 0;
}

void UTankMovementComponent::SetMovementState(ETankMovementState NewMovementState) {
	Movement_State = NewMovementState;

	auto Tank = (ATank*)PawnOwner; //TODO safe cast
	auto Controller = Tank->GetController();

	if (!Controller) {
		return;
	}

	if (Controller->IsLocalPlayerController()) {//Client sends to server his new filtered state
		OnMovementStateChanged(Movement_State);
		Tank->OnMovementStateChanged(Movement_State);
	}
}

//Works on server
void UTankMovementComponent::OnMovementStateChanged_Implementation(ETankMovementState NewMovementState) {
	auto Tank = (ATank*)PawnOwner; //TODO safe cast
	auto Controller = Tank->GetController();

	if (!Controller) {
		return;
	}

	if (Controller->HasAuthority()) {
		if (Controller->IsPlayerController()) {
			UE_LOG(LogTemp, Warning, TEXT("Server:controlled by player"));

			//Just send it like command, and server will react on this(+dt), in best case we need filter command here
			auto NewMovementCommand = ETankMovementCommand::Stand;

			switch (NewMovementState) {
			case ETankMovementState::Stand:
				NewMovementCommand = ETankMovementCommand::Stand;
				break;
			case ETankMovementState::MoveForward:
				NewMovementCommand = ETankMovementCommand::MoveForward;
				break;
			case ETankMovementState::MoveBackward:
				NewMovementCommand = ETankMovementCommand::MoveBackward;
				break;
			case ETankMovementState::RotateLeft:
				NewMovementCommand = ETankMovementCommand::RotateLeft;
				break;
			case ETankMovementState::RotateRight:
				NewMovementCommand = ETankMovementCommand::RotateRight;
				break;
			}
			
			SendMovementCommand(NewMovementCommand);
		}
	}
}

void UTankMovementComponent::OnRep_MovementState() {
	auto Tank = (ATank*)PawnOwner; //TODO safe cast

	UE_LOG(LogTemp, Warning, TEXT("=============="));

	FString message = FString::Printf(TEXT("Other tank moved"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, message); //печатает всем

	switch (Tank->GetNetMode()) {
	case NM_Standalone: UE_LOG(LogTemp, Warning, TEXT("---Standalone")); break;
	case NM_DedicatedServer: UE_LOG(LogTemp, Warning, TEXT("---DedicatedServer")); break;
	case NM_ListenServer: UE_LOG(LogTemp, Warning, TEXT("---ListenServer")); break;
	case NM_Client: UE_LOG(LogTemp, Warning, TEXT("---Client")); break;
	case NM_MAX: UE_LOG(LogTemp, Warning, TEXT("---MAX")); break;
	}

	UE_LOG(LogTemp, Warning, TEXT("======---========"));
}

/*
void UTankMovementComponent::SynchronizePlayerWithClient_Implementation(FPlayerSynchronizationData SyncData) {
	UE_LOG(LogTemp, Warning, TEXT("Teleport to %f %f %f %f %f"), SyncData.Position.X, SyncData.Position.Y, SyncData.Position.Z, SyncData.CurrentSpeed, SyncData.CurrentSpinSpeed);
}
*/

/*
void UTankMovementComponent::SynchronizePlayerWithClient_Implementation(FVector Pos) {
	UE_LOG(LogTemp, Warning, TEXT("Teleport to %f %f %f f"), Pos.X,Pos.Y, Pos.Z);
}
*/

void UTankMovementComponent::SynchronizePlayerWithClient_Implementation(FPlayerSynchronizationData SyncData) {
	auto Tank = (ATank*)PawnOwner; //TODO safe cast

	UE_LOG(LogTemp, Warning, TEXT("Sync2 %f %f %f %f"), SyncData.Yaw, SyncData.CurrentSpeed, SyncData.CurrentSpinSpeed, SyncData.RotateOn);

	auto Location = Tank->GetActorLocation();

	if (FVector::DistSquared(Location, SyncData.Position) > FMath::Pow(SYNCHRONIZE_WITH_SERVER_MAX_POS_ERROR, 2.0)) {
		CorrectMovement(SyncData.Position, SyncData.MovementState, SyncData.MovementInertia, SyncData.CurrentSpeed);
	}

	if (FMath::Abs(Yaw - SyncData.Yaw) > SYNCHRONIZE_WITH_SERVER_MAX_ROT_ERROR) {
		CorrectRotation(SyncData.Yaw, SyncData.CurrentSpinSpeed, SyncData.RotateOn);
	}
}

void UTankMovementComponent::CorrectMovement(FVector NewPosition, ETankMovementState NewMovementState, ETankMovementInertia NewMovementInertia, float NewCurrentSpeed) {
	auto Tank = (ATank*)PawnOwner;

	auto Rotation = Tank->GetActorRotation();

	Tank->TeleportTo(NewPosition, Rotation, false, false);
	Movement_State = NewMovementState;
	MovementInertia = NewMovementInertia;
	CurrentSpeed = NewCurrentSpeed;
}

void UTankMovementComponent::CorrectRotation(float NewYaw, float NewCurrentSpinSpeed, float NewRotateOn) {
	auto Tank = (ATank*)PawnOwner;

	FRotator NewRotation = Tank->GetActorRotation();
	NewRotation.Yaw = NewYaw + YAW_OFFSET;
	
	Tank->SetActorRotation(NewRotation, ETeleportType::None);
	Yaw = NewYaw;
	CurrentSpinSpeed = NewCurrentSpinSpeed;
	RotateOn = NewRotateOn;
}

/*
void AThirdPersonMPCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetControlRotation().Vector()  * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = Instigator;
	spawnParameters.Owner = this;

	AThirdPersonMPProjectile* spawnedProjectile = GetWorld()->SpawnActor<AThirdPersonMPProjectile>(spawnLocation, spawnRotation, spawnParameters);
}
*/

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

void UTankMovementComponent::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate health.
	DOREPLIFETIME(UTankMovementComponent, Movement_State);
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