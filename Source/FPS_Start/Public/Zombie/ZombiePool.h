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
	void RemoveZombieFromAlive(AZombie* Zombie);

	UFUNCTION(BlueprintCallable)
	void Release(AZombie* Zombie);

	/*
	 * 在指定位置和半径内查找丧尸
	 * 参数：
	 * CenterLocation - 中心位置
	 * Radius - 半径
	 * Ordered - 是否按距离排序，默认不排序
	 * Count - 返回的丧尸数量，-1表示返回所有
	 * 如果Ordered为true且Count大于0，则返回距离中心位置最近的Count个丧尸
	 * 如果Ordered为false且Count大于0，则返回随机Count个丧尸
	 */
	UFUNCTION(BlueprintCallable)
	void FindZombieInRadius(const FVector& CenterLocation, const float& Radius, TArray<AZombie*>& OutZombies, bool Ordered = false, int32 Count = -1);

private:
	FTransform PoolTransform = FTransform(FVector(0, 0, 500));
	// 每个派生类一条空闲队列
	TMap<TSubclassOf<AZombie>, TArray<AZombie*>> Buckets;
	// 存活的丧尸
	TSet<AZombie*> AliveZombies;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BatchCount = 10;

	int32 TotalZombieCount = 0;

	void CreateBucket(const TSubclassOf<AZombie>& ZombieClass);

	// 批量创建
	void BatchSpawn(const TSubclassOf<AZombie>& Bucket, const int32& Count);
};
