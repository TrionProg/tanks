// Fill out your copyright notice in the Description page of Project Settings.


// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Tank.h"

// Sets default values
AProjectile::AProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	//CollisionComp->bGenerateOverlapEvents = true;
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	//CollisionComp->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnActorBeginOverlap);

	// Players can't walk on it
	//CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	//CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	//ProjectileMovement->Trace

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
	//bGenerateOverlapEvents = true;

	instigator = nullptr;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	if (ATank* tank = Cast<ATank>(OtherActor)) {
		if (tank->HasAuthority()) {
			auto DamageLocation = tank->CalcDamageLocation(OtherComp);
			OnTankHit(tank, DamageLocation);

			if (instigator) {
				UE_LOG(LogTemp, Warning, TEXT("Give score"));
				instigator->GiveScoreOnServer(20);
			}
		}
		else {
			//TODO play hit animation, sparks, sound
		}
	}

	Destroy();
}

void AProjectile::OnActorBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (ATank* tank = Cast<ATank>(OtherActor)) {
		UE_LOG(LogTemp, Warning, TEXT("Tank Overlap"));
		//OnEnemyHit(enemy);
	}
}

void AProjectile::SetInstigator(ATank* new_instigator) {
	instigator = new_instigator;
}

ATank* AProjectile::GetInstigator() {
	return instigator;
}