/*
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpecterHUD.generated.h"

UCLASS()
class TANKS_API ASpecterHUD : public AHUD
{
	GENERATED_BODY()
	
};
*/

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "SpecterHUD.generated.h"

/**
 *
 */
UCLASS(Abstract)
class TANKS_API USpecterHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateCounter(int32 counter);
};