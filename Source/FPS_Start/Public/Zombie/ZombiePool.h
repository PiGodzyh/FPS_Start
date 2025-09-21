// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WavesDataAsset.h"
#include "ZombiePool.generated.h"

class AZombie;

// 丧尸存储类型
enum class EZombieStorageType : uint8
{
	Common, // 普通
	Grid // 网格存储
};

// 网格体，用于空间划分，提升查找效率
struct FZombieGrid
{
	static float GridSize;
	TSet<TWeakObjectPtr<AZombie>> Zombies;

	static FIntPoint LocationToGrid(const FVector& Location)
	{
		int32 X = FMath::FloorToInt(Location.X / GridSize);
		int32 Y = FMath::FloorToInt(Location.Y / GridSize);
		return { X, Y };
	}
};
/**
 * 丧尸对象池
 * 用于管理丧尸的创建和回收，提升性能
 */
UCLASS()
class UZombiePool : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	static int32 CommonGate;
	static int32 GridGate;
public:
	// 初始化时调用
	UFUNCTION(BlueprintCallable)
	void WarmPool(const TArray<FZombieSpawnData>& ZombieSpawnDataArray);

	UFUNCTION(BlueprintCallable)
	AZombie* Acquire(const TSubclassOf<AZombie>& ZombieClass);

	UFUNCTION(BlueprintCallable)
	void RemoveZombieFromAlive(AZombie* Zombie);

	UFUNCTION(BlueprintCallable)
	void AddZombieToAlive(AZombie* Zombie);

	UFUNCTION(BlueprintCallable)
	void Release(AZombie* Zombie);

	UFUNCTION(BlueprintCallable)
	int32 GetAliveZombieCount()const { return AliveZombies.Num(); }

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

	UFUNCTION(BlueprintCallable)
	void RemoveZombieFromGrid(AZombie* Zombie);

	UFUNCTION(BlueprintCallable)
	void AddZombieToGrid(AZombie* Zombie);

	UFUNCTION(BlueprintCallable)
	void MoveZombie(AZombie* Zombie, const FIntPoint& OldGrid, const FIntPoint& NewGrid);

private:
	FTransform PoolTransform = FTransform(FVector(0, 0, 500));
	// 每个派生类一条空闲队列
	TMap<TSubclassOf<AZombie>, TArray<TWeakObjectPtr<AZombie>>> Buckets;
	// 存活的丧尸
	TSet<TWeakObjectPtr<AZombie>> AliveZombies;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BatchCount = 10;

	int32 TotalZombieCount = 0;

	TMap<FIntPoint, FZombieGrid> ZombieGrids;

	void CreateBucket(const TSubclassOf<AZombie>& ZombieClass);

	// 批量创建
	void BatchSpawn(const TSubclassOf<AZombie>& Bucket, const int32& Count);

public:
	EZombieStorageType ZombieStorage = EZombieStorageType::Common;
};
