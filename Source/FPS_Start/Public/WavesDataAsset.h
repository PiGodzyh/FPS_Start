// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/TargetPoint.h"
#include "Zombie/Zombie.h"
#include "WavesDataAsset.generated.h"

// 丧尸生成数据结构，包含丧尸类、数量和生成点列表
USTRUCT(BlueprintType)
struct FZombieSpawnData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AZombie> ZombieClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<ATargetPoint>>SpawnLocationArray;
};

USTRUCT(BlueprintType)
struct FWaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FZombieSpawnData> ZombieSpawnDataArray;
};
/**
 * 
 */
UCLASS(BlueprintType)
class FPS_START_API UWavesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWaveData> WaveDataArray;
};
