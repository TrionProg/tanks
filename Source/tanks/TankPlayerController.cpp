// Fill out your copyright notice in the Description page of Project Settings.


#include "TankPlayerController.h"
#include "TankPlayerState.h"
#include "Tank.h"
#include "Specter.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "TankHUD.h"
#include "SpecterHUD.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TankPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Math/UnrealMathUtility.h"


ATankPlayerController::ATankPlayerController() : Super() {
	PrimaryActorTick.bCanEverTick = true;

	bShowMouseCursor = true;
	CurrentWidget = nullptr;

	TankPawn = ATank::StaticClass();

	PlayingState = EPlayerPlayingState::Playing;
}

//---UE Events---

void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();

	change_menu_widget(TankHUD);

	UE_LOG(LogTemp, Warning, TEXT("Player Controller begin play"));

	SetInputMode(FInputModeGameAndUI());
}

void ATankPlayerController::SetupInputComponent() {
	// set up gameplay key bindings
	Super::SetupInputComponent();

	/*
	InputComponent->BindAction("LMBClick", IE_Released, this, &AMyPlayerController::on_lmb_release);

	InputComponent->BindAction("Pause", IE_Released, this, &AMyPlayerController::on_key_space_release).bExecuteWhenPaused = true;
	InputComponent->BindAction("Restart", IE_Released, this, &AMyPlayerController::on_key_r_release).bExecuteWhenPaused = true;
	InputComponent->BindAction("Quit", IE_Pressed, this, &AMyPlayerController::on_key_q_press).bExecuteWhenPaused = true;
	*/

}

void ATankPlayerController::Tick(float dt) {
	Super::Tick(dt);

	tick(dt);
}

void ATankPlayerController::PlayerTick(float dt) {
	Super::PlayerTick(dt);

	update_hud();
}



//---Util---

OptionPtr<UWorld> ATankPlayerController::get_world() {
	return OptionPtr<UWorld>::new_unchecked(GetWorld());
}

OptionPtr<ATank> ATankPlayerController::get_tank() {
	auto pawn = GetPawn();
	auto spectator = Cast<ATank>(pawn);

	return OptionPtr<ATank>::new_unchecked(spectator);
}

OptionPtr<ASpecter> ATankPlayerController::get_specter() {
	auto pawn = GetPawn();
	auto specter = Cast<ASpecter>(pawn);

	return OptionPtr<ASpecter>::new_unchecked(specter);
}

OptionPtr<ATankPlayerState> ATankPlayerController::get_player_state() {
	return OptionPtr<ATankPlayerState>::new_unchecked((ATankPlayerState*)PlayerState);
}

//---Plaing---

/*
OptionPtr<ABasicGameModeBase> AMyPlayerController::get_game_mode() {
	if (auto world = get_world().match()) {
		auto abstract_game_mode = world->GetAuthGameMode();
		if (abstract_game_mode) {
			ABasicGameModeBase* game_mode = Cast<ABasicGameModeBase>(abstract_game_mode);

			return OptionPtr<ABasicGameModeBase>::new_unchecked(game_mode);
		}
	}

	return OptionPtr<ABasicGameModeBase>::new_none();
}

OptionPtr<ABasicGameStateBase> AMyPlayerController::get_game_state() {
	if (auto world = get_world().match()) {
		return OptionPtr<ABasicGameStateBase>::new_unchecked(world->GetGameState<ABasicGameStateBase>());
	}
	else {
		return OptionPtr<ABasicGameStateBase>::new_none();
	}
}
*/

//---GUI---

void ATankPlayerController::change_menu_widget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (!IsLocalController()) {
		return;
	}

	if (CurrentWidget != nullptr) {
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}
	if (NewWidgetClass != nullptr) {
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), NewWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}

void ATankPlayerController::on_key_q_press() {
	if (auto world = get_world().match()) {
		//UKismetSystemLibrary::QuitGame(world, this, EQuitPreference::Type::Quit);
	}
}

void ATankPlayerController::update_hud() {
	if (!IsLocalController()) {
		return;
	}

	if (CurrentWidget == nullptr) {
		return;
	}

	if (auto tank_hud = Cast<UTankHUD>(CurrentWidget)) {
		if (auto Tank = get_tank().match()) {
			tank_hud->UpdateHealth(Tank->GetHealth());
		}
		//if (auto player_state = get_player_state().match()) {
		//	hud->SetMoney(player_state->money);
		//}

	}else if (auto specter_hud = Cast<USpecterHUD>(CurrentWidget)) {
		specter_hud->UpdateCounter(wait_to_spawn_counter);
	}


}

//---Playing---

void ATankPlayerController::tick(float dt) {
	if (PlayingState == EPlayerPlayingState::WaitingToSpawn) {
		if (HasAuthority()) {
			UE_LOG(LogTemp, Warning, TEXT("Tick2"));
		}
		wait_to_spawn_counter -= dt;
		
		if (wait_to_spawn_counter <= 0) {
			wait_to_spawn_counter = 0;

			if (HasAuthority()) {
				OnSpawnOnServer();
			}
		}
	}
}

void ATankPlayerController::OnDeathOnServer() {
	float counter = 4;//TODO

	PlayingState = EPlayerPlayingState::WaitingToSpawn;
	wait_to_spawn_counter = counter;

	if (!HasAuthority()) {
		return;
	}

	OnDeathMulticast(counter);
}

UFUNCTION(NetMulticast, Reliable)
void ATankPlayerController::OnDeathMulticast_Implementation(float counter) {
	OnDeathOnClient(counter);
}

void ATankPlayerController::OnDeathOnClient(float counter) {
	PlayingState = EPlayerPlayingState::WaitingToSpawn;
	wait_to_spawn_counter = counter;

	change_menu_widget(SpecterHUD);
}

void ATankPlayerController::OnSpawnOnServer() {
	UE_LOG(LogTemp, Warning, TEXT("Spawn on server"));
	if (auto world = get_world().match()) {
		if (auto Specter = get_specter().match()) {
			if (auto Tank = SpawnTank().match()) {
				Possess(Tank);

				Specter->Destroy();

				PlayingState = EPlayerPlayingState::Playing;
				OnSpawnMulticast();
			}
			//Else OnSpawnOnServer will be called on next tick(very, very rary);
		}
	}
}

OptionPtr<ATank> ATankPlayerController::SpawnTank() {
	if (auto world = get_world().match()) {
		TArray<AActor*> found_spawn_points; //TODO ofcourse list is better, but collisions are very rary

		UGameplayStatics::GetAllActorsOfClass((UObject*)world, APlayerStart::StaticClass(), found_spawn_points);

		while (found_spawn_points.Num() > 0) {
			auto random_index = FMath::RandRange(0, found_spawn_points.Num());

			if (found_spawn_points.IsValidIndex(random_index)) {
				if (auto spawn_point = Cast<APlayerStart>(found_spawn_points[random_index])) {
					auto location = spawn_point->GetActorLocation();
					auto rotation = FRotator::ZeroRotator;

					auto spawn_info = FActorSpawnParameters();
					spawn_info.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					ATank* tank = (ATank*)world->SpawnActor(TankPawn, &location, &rotation, spawn_info);

					if (tank) {
						return OptionPtr<ATank>::new_some(tank);
					} else {
						found_spawn_points.RemoveAt(random_index);
					}
				} else {
					found_spawn_points.RemoveAt(random_index);
				}
			}
		}
	}

	return OptionPtr<ATank>::new_none();
}

void ATankPlayerController::OnSpawnMulticast_Implementation() {
	OnSpawnOnClient();
}

void ATankPlayerController::OnSpawnOnClient() {
	UE_LOG(LogTemp, Warning, TEXT("Spawn on client"));
	PlayingState = EPlayerPlayingState::Playing;

	change_menu_widget(TankHUD);
}