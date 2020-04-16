// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingText.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

// Sets default values
AFlyingText::AFlyingText()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));

	TextComponent->SetupAttachment(RootComponent);
	TextComponent->SetRelativeRotation(FRotator(0, 90, 0));
	TextComponent->SetXScale(4);
	TextComponent->SetYScale(4);

	RiseSpeed = 100;
	BeginZ = 0;
	EndZ = 400;

	FText Text = FText::FromString(TEXT("Hello"));
	TextComponent->SetText(Text);

	auto TextSize = TextComponent->GetTextLocalSize();
	OffsetX = -(TextSize.Y / 2.0);

	TextComponent->SetRelativeLocation(FVector(OffsetX, 0, BeginZ));
}

// Called when the game starts or when spawned
void AFlyingText::BeginPlay()
{
	Super::BeginPlay();

	auto location = TextComponent->GetRelativeLocation();
	TextComponent->SetRelativeLocation(FVector(location.X, location.Y, BeginZ));

	//NOTE it works after assignment material in BP editor
	auto Material = TextComponent->GetMaterial(0);
	DynamicMaterial = UMaterialInstanceDynamic::Create(Material, nullptr);
	TextComponent->SetMaterial(0, DynamicMaterial);
}

// Called every frame
void AFlyingText::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto DeltaZ = RiseSpeed * DeltaTime;

	auto location = TextComponent->GetRelativeLocation();
	auto CurZ = location.Z + DeltaZ;
	if (CurZ > EndZ) {
		Destroy();
		return;
	}

	TextComponent->SetRelativeLocation(FVector(location.X, location.Y, CurZ));

	auto KZ = (CurZ - BeginZ) / (EndZ - BeginZ);

	DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), 1.0 - KZ);
}

void AFlyingText::SetText(FText Text) {
	TextComponent->SetText(Text);

	auto location = TextComponent->GetRelativeLocation();
	auto RealLocationX = location.X - OffsetX;

	auto TextSize = TextComponent->GetTextLocalSize();
	OffsetX = -(TextSize.Y / 2.0);

	TextComponent->SetRelativeLocation(FVector(OffsetX + RealLocationX, location.Y, location.Z));
}
