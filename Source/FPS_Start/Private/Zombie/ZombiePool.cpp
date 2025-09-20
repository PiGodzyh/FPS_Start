// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie/ZombiePool.h"
#include "Zombie/Zombie.h"

void UZombiePool::WarmPool(const TArray<FZombieSpawnData>& ZombieSpawnDataArray)
{
	UE_LOG(LogTemp, Log, TEXT("对象池开始预热"));
	TMap<TSubclassOf<AZombie>, int32> ZombieSumMap;
	// 统计每种丧尸的总数量
	for (const FZombieSpawnData& ZombieSpawnData : ZombieSpawnDataArray)
	{
		TSubclassOf<AZombie> ZombieClass = ZombieSpawnData.ZombieClass;
		int32 Count = ZombieSpawnData.Count; // 预生成所有丧尸
		ZombieSumMap.FindOrAdd(ZombieClass) += Count;

	}
	// 为每种丧尸创建桶
	for (const auto& Pair : ZombieSumMap)
	{
		TSubclassOf<AZombie> ZombieClass = Pair.Key;
		int32 Count = Pair.Value;
		if (!Buckets.Contains(ZombieClass))
			CreateBucket(ZombieClass);
		// 补充到指定数量
		BatchSpawn(ZombieClass, Count - Buckets[ZombieClass].Num());
	}
}

AZombie* UZombiePool::Acquire(const TSubclassOf<AZombie>& ZombieClass)
{
	// 确保有这个类的桶
	if (!Buckets.Contains(ZombieClass))
		CreateBucket(ZombieClass);

	auto& Bucket = Buckets[ZombieClass];
	// 对象池即将空了，批量补充
	if (Bucket.Num() <= 3)
	{
		BatchSpawn(ZombieClass, BatchCount);
	}
	if (Bucket.Num() > 0)
	{
		// 对象池中有空闲的，直接取
		AZombie* NewZombie = Bucket.Pop();
		AliveZombies.Add(NewZombie);
		NewZombie->StartPlay();
		return NewZombie;
	}
	return nullptr;
}

void UZombiePool::RemoveZombieFromAlive(AZombie* Zombie)
{
	if (!Zombie) return;
	AliveZombies.Remove(Zombie);
}

void UZombiePool::Release(AZombie* Zombie)
{
	if (!Zombie) return;
	TSubclassOf<AZombie> ZombieClass = Zombie->GetClass();
	if (!Buckets.Contains(ZombieClass))
		CreateBucket(ZombieClass);
	auto& Bucket = Buckets[ZombieClass];
	Zombie->BackToPool();
	Zombie->SetActorTransform(PoolTransform);
	Bucket.Add(Zombie);
}

void UZombiePool::FindZombieInRadius(
	const FVector& CenterLocation,
	const float& Radius,
	TArray<AZombie*>& OutZombies,
	bool Ordered,
	int32 Count)
{
	OutZombies.Reset();

	// 参数检查
	if (Count == 0)return;

	// 范围内僵尸一次性收进来
	for (AZombie* Z : AliveZombies)
	{
		if (Z && FVector::DistSquared(Z->GetActorLocation(), CenterLocation) <= Radius * Radius)
		{
			OutZombies.Add(Z);
		}
	}

	if (Ordered)
	{
		// 按距离从近到远排序（Lambda 捕获 CenterLocation）
		Algo::Sort(OutZombies, [&CenterLocation](AZombie* A, AZombie* B)
			{
				float DistA = FVector::DistSquared(A->GetActorLocation(), CenterLocation);
				float DistB = FVector::DistSquared(B->GetActorLocation(), CenterLocation);
				return DistA < DistB;
			});
	}

	if (Count >= 0)
	{
		OutZombies.SetNum(FMath::Min(Count, OutZombies.Num()));
	}
}

void UZombiePool::CreateBucket(const TSubclassOf<AZombie>& ZombieClass)
{
	// 创建一个新的桶
	TArray<AZombie*> NewBucket;
	FActorSpawnParameters Params = FActorSpawnParameters();
	Buckets.Add(ZombieClass, NewBucket);
}


void UZombiePool::BatchSpawn(const TSubclassOf<AZombie>& ZombieClass, const int32& Count)
{
	for (int32 i = 0; i < Count; i++)
	{
		// 生成一个新的丧尸实例
		FActorSpawnParameters Params = FActorSpawnParameters();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AZombie* Zombie = GetWorld()->SpawnActor<AZombie>(ZombieClass, PoolTransform, Params);
		Buckets[ZombieClass].Add(Zombie);
	}
}