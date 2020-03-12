// Fill out your copyright notice in the Description page of Project Settings.


#include "TankPlayerController.h"
#include "TankPlayerState.h"
#include "Tank.h"
#include "Runtime/Engine/Classes/Engine/World.h"

//My methods

OptionPtr<UWorld> ATankPlayerController::get_world() {
	return OptionPtr<UWorld>::new_unchecked(GetWorld());
}

OptionPtr<ATank> ATankPlayerController::get_tank() {
	auto pawn = GetPawn();
	auto spectator = (ATank*)pawn;

	return OptionPtr<ATank>::new_unchecked(spectator);
}

OptionPtr<ATankPlayerState> ATankPlayerController::get_player_state() {
	return OptionPtr<ATankPlayerState>::new_unchecked((ATankPlayerState*)PlayerState);
}

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