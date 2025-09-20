// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie/ZombiePool.h"
#include "Zombie/Zombie.h"

float FZombieGrid::GridSize = 500.f;

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
		NewZombie->StartPlay();
		return NewZombie;
	}
	return nullptr;
}

void UZombiePool::RemoveZombieFromAlive(AZombie* Zombie)
{
	if (!Zombie) return;
	AliveZombies.Remove(Zombie);
	// 从网格中移除
	RemoveZombieFromGrid(Zombie);
	// 更改存储策略
	if (ZombieStorage == EZombieStorageType::Grid && AliveZombies.Num() < 100)
	{
		ZombieStorage = EZombieStorageType::Common;
	}
}

void UZombiePool::AddZombieToAlive(AZombie* Zombie)
{
	if (!Zombie) return;
	AliveZombies.Add(Zombie);
	// 加入网格
	AddZombieToGrid(Zombie);
	// 更改存储策略
	if (ZombieStorage == EZombieStorageType::Common && AliveZombies.Num() > 150)
	{
		ZombieStorage = EZombieStorageType::Grid;
	}
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
	if (Radius <= 0 || Count == 0)return;

	if (ZombieStorage == EZombieStorageType::Common)
	{
		for (AZombie* Z : AliveZombies)
		{
			if (Z && FVector::DistSquared(Z->GetActorLocation(), CenterLocation) <= Radius * Radius)
			{
				OutZombies.Add(Z);
			}
		}
	}
	else if (ZombieStorage == EZombieStorageType::Grid)
	{
		TPair<int32, int32> Grid = FZombieGrid::LocationToGrid(CenterLocation);
		int32 Range = FMath::CeilToInt(Radius / FZombieGrid::GridSize);
		for (int32 X = Grid.Key - Range; X <= Grid.Key + Range; X++)
		{
			for (int32 Y = Grid.Value - Range; Y <= Grid.Value + Range; Y++)
			{
				TPair<int32, int32> CheckGrid(X, Y);
				if (ZombieGrids.Contains(CheckGrid))
				{
					for (AZombie* Z : ZombieGrids[CheckGrid].Zombies)
					{
						if (Z && FVector::DistSquared(Z->GetActorLocation(), CenterLocation) <= Radius * Radius)
						{
							OutZombies.Add(Z);
						}
					}
				}
			}
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

void UZombiePool::InitialZombieGrid(AZombie* Zombie)
{
	AddZombieToGrid(Zombie);
	Zombie->UpdateOldLocation();
}

void UZombiePool::RemoveZombieFromGrid(AZombie* Zombie)
{
	const TPair<int32, int32> Grid = FZombieGrid::LocationToGrid(Zombie->GetOldLocation());
	if (ZombieGrids.Contains(Grid))
	{
		ZombieGrids[Grid].Zombies.Remove(Zombie);
	}
}

void UZombiePool::AddZombieToGrid(AZombie* Zombie)
{
	const TPair<int32, int32> Grid = FZombieGrid::LocationToGrid(Zombie->GetActorLocation());
	ZombieGrids.FindOrAdd(Grid).Zombies.Add(Zombie);
}

void UZombiePool::MoveZombie(AZombie* Zombie, const FVector& OldLocation, const FVector& NewLocation)
{
	// 计算旧位置和新位置所在的网格
	const TPair<int32, int32> OldGrid = FZombieGrid::LocationToGrid(OldLocation);
	const TPair<int32, int32> NewGrid = FZombieGrid::LocationToGrid(NewLocation);
	// 如果网格没有变化，直接返回
	if (OldGrid == NewGrid)
	{
		return;
	}

	ZombieGrids[OldGrid].Zombies.Remove(Zombie);
	ZombieGrids[NewGrid].Zombies.Add(Zombie);
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