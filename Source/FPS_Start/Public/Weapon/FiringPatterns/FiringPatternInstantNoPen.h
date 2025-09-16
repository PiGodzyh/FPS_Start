// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FiringPattern.h"
#include "FiringPatternInstantNoPen.generated.h"

/**
 * 即时无穿透
 */
UCLASS(BlueprintType)
class UFiringPatternInstantNoPen : public UFiringPattern
{
	GENERATED_BODY()

public:
	UFiringPatternInstantNoPen();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "HitImpact")
	TObjectPtr<UDataTable> HitImpactTable = nullptr;

public:
	virtual void FireSingle(AWeaponBase* Weapon, AController* Instigator) override;

	virtual void SpawnDecal(const FHitResult& HitResult, UMaterial* Decal);

	virtual void SpawnEffect(const FHitResult& HitResult, UParticleSystem* Effect);

	virtual void PlaySound(const FHitResult& HitResult, USoundBase* Sound);

	virtual void AddImpulse(const FHitResult& HitResult, const FVector& ShotDir, float PelletMass, float InitialSpeed);
};
