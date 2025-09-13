// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WavesDataAsset.h"
#include "ZombiePool.generated.h"

class AZombie;
/**
 * 丧尸对象池
 * 用于管理丧尸的创建和回收，提升性能
 */
UCLASS()
class UZombiePool : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    // 初始化时调用
    UFUNCTION(BlueprintCallable)
    void WarmPool(const TArray<FZombieSpawnData>& ZombieSpawnDataArray);

    UFUNCTION(BlueprintCallable)
    AZombie* Acquire(const TSubclassOf<AZombie>& ZombieClass);

    UFUNCTION(BlueprintCallable)
    void Release(AZombie* Zombie);

private:
	FTransform PoolTransform = FTransform(FVector(0,0,500));
    // 每个派生类一条“原型 + 空闲队列”
    TMap<TSubclassOf<AZombie>, TArray<AZombie*>> Buckets;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BatchCount = 10;

    int32 TotalZombieCount = 0;

    void CreateBucket(const TSubclassOf<AZombie>& ZombieClass);

    // 批量创建
    void BatchSpawn(const TSubclassOf<AZombie>& Bucket, const int32& Count);
};
