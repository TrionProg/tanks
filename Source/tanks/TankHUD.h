/*
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TankHUD.generated.h"

UCLASS()
class TANKS_API ATankHUD : public AHUD
{
	GENERATED_BODY()
	
};
*/

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "TankHUD.generated.h"

UCLASS(Abstract)
class TANKS_API UTankHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateScore(int32 score);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealth(const int32 health);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateAmmo(const int32 ammo);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateShootInterval(const float percent);
};