// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlyingText.generated.h"

class UMaterialInstanceDynamic;

UCLASS()
class TANKS_API AFlyingText : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTextRenderComponent* TextComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RiseSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BeginZ;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float EndZ;

	float OffsetX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInstanceDynamic* DynamicMaterial;
	
public:	
	// Sets default values for this actor's properties
	AFlyingText();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetText(FText Text);
};
