// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/FiringPatterns/FiringPatternInstantNoPen.h"

#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponBase.h"
#include <Weapon/HitImpact/HitImpactRow.h>

UFiringPatternInstantNoPen::UFiringPatternInstantNoPen()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> HitImpactTableFinder(TEXT("/Game/Data/Weapon/DT_HitImpactInstant.DT_HitImpactInstant"));
	if (HitImpactTableFinder.Succeeded())
	{
		HitImpactTable = HitImpactTableFinder.Object;
	}
	if (!HitImpactTable)
	{
		UE_LOG(LogTemp, Error, TEXT("HitImpactTable not found!"));
	}
}

void UFiringPatternInstantNoPen::FireSingle(AWeaponBase* Weapon, AController* Instigator)
{
	if (!Weapon || !Instigator) return;
	UE_LOG(LogTemp, Log, TEXT("FireSingle"));
	UWorld* World = Weapon->GetWorld();

	// 计算基础视线方向
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("获取世界失败"));
		return;
	}
	FVector StartLoc;
	FRotator Rotation;
	Instigator->GetPlayerViewPoint(StartLoc, Rotation);
	FVector BaseDir = Rotation.Vector();

	// 获取散布角度
	const float SpreadRad = FMath::DegreesToRadians(Weapon->GetCurrentSpread() * 0.5f);

	// 配置射线检测通用参数
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Weapon);
	Params.AddIgnoredActor(Instigator->GetPawn());
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = true;

	const float Damage = Weapon->GetWeaponData().DamagePerPellet;

	// 多次射线检测
	int32 PelletCount = Weapon->GetWeaponData().PelletCount;
	while (PelletCount--)
	{
		UE_LOG(LogTemp, Log, TEXT("计算某个弹丸"));
		// 计算单个弹丸的射线方向
		const FVector ShotDir = FMath::VRandCone(BaseDir, SpreadRad);

		// 计算射线检测起点和终点
		constexpr float Range = 10000.f;
		const FVector EndLoc = StartLoc + ShotDir * Range;

		// 射线检测结果
		FHitResult Hit;

		// 执行射线检测并应用伤害	
		if (World->LineTraceSingleByChannel(Hit, StartLoc, EndLoc, ECC_Visibility, Params))
		{
			UE_LOG(LogTemp, Log, TEXT("碰撞成功"));
			UGameplayStatics::ApplyPointDamage(
				Hit.GetActor(),
				Damage,
				ShotDir,
				Hit,
				Instigator,
				Weapon,
				nullptr);
			FName PhysicalMaterialName = Hit.PhysMaterial->GetFName();
			FHitImpactRow* HitImpactRow = HitImpactTable->FindRow<FHitImpactRow>(PhysicalMaterialName, FString::Printf(TEXT("%s Row Not Found"), *PhysicalMaterialName.ToString()));
			SpawnDecal(Hit, HitImpactRow->Decal);
			SpawnEffect(Hit, HitImpactRow->Effect);
			PlaySound(Hit, HitImpactRow->Sound);
			if (Hit.Component->IsSimulatingPhysics())
			{
				AddImpulse(Hit, ShotDir,Weapon->GetWeaponData().PelletMass, Weapon->GetWeaponData().InitialSpeed);
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("碰撞失败"));
		}
	}
	
}

void UFiringPatternInstantNoPen::SpawnDecal(const FHitResult& HitResult, UMaterial* Decal)
{
	if (Decal)
	{
		const FVector DecalSize = FVector(7, 7, 7);
		UGameplayStatics::SpawnDecalAttached(
			Decal,
			DecalSize,
			HitResult.Component.Get(),
			HitResult.BoneName,
			HitResult.ImpactPoint,
			HitResult.Normal.Rotation(),
			EAttachLocation::KeepWorldPosition,
			10);
	}
}

void UFiringPatternInstantNoPen::SpawnEffect(const FHitResult& HitResult, UParticleSystem* Effect)
{
	if (Effect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			HitResult.GetActor()->GetWorld(),
			Effect,
			HitResult.ImpactPoint,
			HitResult.Normal.Rotation()
		);
	}
}

void UFiringPatternInstantNoPen::PlaySound(const FHitResult& HitResult, USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			HitResult.GetActor()->GetWorld(),
			Sound,
			HitResult.ImpactPoint
		);
	}
}

void UFiringPatternInstantNoPen::AddImpulse(const FHitResult& HitResult, const FVector& ShotDir, float PelletMass, float InitialSpeed)
{
	if (HitResult.GetActor() && HitResult.GetActor()->IsRootComponentMovable())
	{
		const FVector Impulse = ShotDir * (PelletMass * InitialSpeed);
		HitResult.Component->AddImpulse(Impulse, HitResult.BoneName, true);
	}
}
