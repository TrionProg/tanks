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

	//TODO * to allow engine to serialize it?
	OptionPtr<class ATankState> Tank;
	
};
