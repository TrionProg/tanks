// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OptionPtr.h"


#include "TankPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TANKS_API ATankPlayerState : public APlayerState {
	GENERATED_BODY()

public:

	int32 GetPlayerId();

public:

	void SetScore(float score);
	void GiveScore(float score);
	float GetScore();
	//UPROPERTY()
	//float Score;
	
};
