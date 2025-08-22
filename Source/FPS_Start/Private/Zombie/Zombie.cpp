// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie/Zombie.h"
#include "Engine/DamageEvents.h"
#include "ProfilingDebugging/CookStats.h"

void AZombie::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	PlayingHitAnim = nullptr; // 清除正在播放的受击动画
	UE_LOG(LogTemp, Log, TEXT("当前受击动画播放完成"));
}

// Sets default values
AZombie::AZombie()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AZombie::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* ZombieMesh = Cast<USkeletalMeshComponent>(GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	check(ZombieMesh)

		AnimInst = ZombieMesh->GetAnimInstance();
	check(AnimInst);
}

// Called every frame
void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AZombie::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

inline UAnimMontage* AZombie::SelectHitReactMontage(FName BoneName) const
{
	if (GetZombieData().LeftBones.Contains(BoneName))
	{
		return GetZombieData().LeftArmHitMontage;
	}
	if (GetZombieData().RightBones.Contains(BoneName))
	{
		return GetZombieData().RightArmHitMontage;
	}
	return GetZombieData().HeadHitMontage;
}

void AZombie::HandlePointDamage(float Damage, const FPointDamageEvent& PointEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	// 骨骼命中位置（可用于精准断肢）
	FName BoneName = PointEvent.HitInfo.BoneName;
	UAnimMontage* HitAnimMontage = SelectHitReactMontage(BoneName);

	if (GetZombieData().HeadBones.Contains(BoneName))Damage *= 2;// 如果是头部受击，则伤害翻倍

	if (!HitAnimMontage)
	{
		// 如果没有找到对应的受击动画，则直接返回
		return;
	}

	// 1播放受击动画
	if (PlayingHitAnim != HitAnimMontage)
	{
		// 如果当前正在播放的受击动画和新动画不一致，先停止当前动画
		if (PlayingHitAnim)
		{
			StopAnimMontage(PlayingHitAnim);
		}
		PlayingHitAnim = HitAnimMontage;
		// 绑定动画结束回调
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AZombie::OnMontageEnded);
		AnimInst->Montage_Play(HitAnimMontage);
		AnimInst->Montage_SetEndDelegate(EndDelegate, HitAnimMontage);

	}

	// 扣血
	Health = FMath::Clamp(Health - Damage, 0.f, GetZombieData().MaxHealth);

	//// 6. 死亡
	//if (Health <= 0.f)
	//{
	//	Die(EventInstigator, DamageCauser);
	//}
}

float AZombie::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f)
	{
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		{
			const FPointDamageEvent& PointEvent = StaticCast<FPointDamageEvent const&>(DamageEvent);
			HandlePointDamage(ActualDamage, PointEvent, EventInstigator, DamageCauser);
		}
		else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
		{

		}
	}

	return ActualDamage;
}

