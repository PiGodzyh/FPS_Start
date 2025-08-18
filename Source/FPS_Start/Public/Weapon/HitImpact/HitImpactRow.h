// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "HitImpactRow.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FHitImpactRow: public FTableRowBase
{
	GENERATED_BODY()

	/// 命中贴花
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterial> Decal;

	/// 命中声音
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> Sound;

	/// 命中效果
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Effect;

};
