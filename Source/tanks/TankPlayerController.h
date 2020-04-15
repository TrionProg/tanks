// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OptionPtr.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#include "TankPlayerController.generated.h"


UENUM(BlueprintType)
enum class EPlayerPlayingState : uint8
{
	WaitingToSpawn 				UMETA(DisplayName = "Player is waiting to spawn"),
	Playing 					UMETA(DisplayName = "Player is playing"),
};

UCLASS()
class TANKS_API ATankPlayerController : public APlayerController
{
	GENERATED_BODY()
//UE variables
//UE events and methods

//===UE events===

virtual void BeginPlay() override;
virtual void SetupInputComponent() override;
virtual void Tick(float DeltaTime) override;
virtual void PlayerTick(float DeltaTime) override;

public:
	ATankPlayerController();

//===Util===
private:
	OptionPtr<UWorld> get_world();
	OptionPtr<class ATank> get_tank();
	OptionPtr<class ASpecter> get_specter();
	OptionPtr<class ATankPlayerState> get_player_state();
	//OptionPtr<class ABasicGameModeBase> get_game_mode(); //TODO only on server
	//OptionPtr<class ABasicGameStateBase> get_game_state();

//===GUI===
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UMG")
	TSubclassOf<UUserWidget> SpecterHUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UMG")
	TSubclassOf<UUserWidget> TankHUD;
private:
	UPROPERTY()
	UUserWidget* CurrentWidget;

	UFUNCTION(BlueprintCallable, Category = "UMG")
	void change_menu_widget(TSubclassOf<UUserWidget> NewWidgetClass);
	void on_key_q_press();

	void update_hud();

//===Playing===
private:
	UPROPERTY(EditDefaultsOnly, Category = Tank)
	TSubclassOf<class ATank> TankPawn;

	EPlayerPlayingState PlayingState;
	float wait_to_spawn_counter;

	void tick(float dt);
public:
	void OnDeathOnServer();
private:
	UFUNCTION(NetMulticast, Reliable)
	void OnDeathMulticast(float counter);

	void OnDeathOnClient(float counter);

	void OnSpawnOnServer();

	OptionPtr<ATank> SpawnTank();

	UFUNCTION(NetMulticast, Reliable)
	void OnSpawnMulticast();

	void OnSpawnOnClient();
};
