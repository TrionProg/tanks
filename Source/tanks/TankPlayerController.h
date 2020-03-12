// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OptionPtr.h"

#include "TankPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TANKS_API ATankPlayerController : public APlayerController
{
	GENERATED_BODY()
//UE variables
//UE events and methods

//My variables
private:
	//bool paused;
//My methods
private:
	OptionPtr<UWorld> get_world();
	OptionPtr<class ATank> get_tank();
	OptionPtr<class ATankPlayerState> get_player_state();
	//OptionPtr<class ABasicGameModeBase> get_game_mode(); //TODO only on server
	//OptionPtr<class ABasicGameStateBase> get_game_state();

};
