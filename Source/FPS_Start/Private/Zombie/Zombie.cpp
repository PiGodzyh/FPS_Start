// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie/Zombie.h"
#include "Engine/DamageEvents.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Navigation/PathFollowingComponent.h" 
#include "Zombie/ZombiePool.h"

void AZombie::OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	PlayingHitAnim = nullptr; // 清除正在播放的受击动画
}

void AZombie::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false; // 攻击动画结束，重置攻击状态
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

	GetMesh()->SetAnimInstanceClass(GetZombieData().AnimInstanceClass);
	AnimInst = GetMesh()->GetAnimInstance();
	check(AnimInst);

	BackToPool();
	UE_LOG(LogTemp, Warning, TEXT("this=%p, Name=%s"), this, *GetName());

	// 让丧尸忽略自定义通道1（丧尸攻击检测通道）
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
}

// Called every frame
void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsAttacking)
	{
		UE_LOG(LogTemp, Warning, TEXT("Vel=%.1f Accel=%.1f"),
			GetVelocity().Size(),
			GetCharacterMovement()->GetCurrentAcceleration().Size());
	}
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
		EndDelegate.BindUObject(this, &AZombie::OnHitMontageEnded);
		if (AnimInst)
		{
			AnimInst->Montage_Play(HitAnimMontage);
			AnimInst->Montage_SetEndDelegate(EndDelegate, HitAnimMontage);
		}
	}

	// 扣血
	Health = FMath::Clamp(Health - Damage, 0.f, GetZombieData().MaxHealth);

	// 死亡
	if (Health <= 0.f)
	{
		Die(DamageCauser);
	}
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

void AZombie::Attack()
{
	// 如果没有正在播放受击动画，且不在攻击状态，且有攻击动画
	if (!PlayingHitAnim && !bIsAttacking && !bIsDead && GetZombieData().AttackMontage.Num() > 0)
	{
		// 随机选择一个攻击动画播放
		bIsAttacking = true;
		int32 RandomIndex = FMath::RandRange(0, GetZombieData().AttackMontage.Num() - 1);
		AnimInst->Montage_Play(GetZombieData().AttackMontage[RandomIndex]);
		// 绑定动画结束回调
		FOnMontageEnded AttackEndDelegate;
		AttackEndDelegate.BindUObject(this, &AZombie::OnAttackMontageEnded);
		AnimInst->Montage_SetEndDelegate(AttackEndDelegate, GetZombieData().AttackMontage[RandomIndex]);
	}
}

void AZombie::TryDoDamage(FName StartBoneName, FName EndBoneName)
{
	FVector StartLoc = GetMesh()->GetSocketLocation(StartBoneName);
	FVector EndLoc = GetMesh()->GetSocketLocation(EndBoneName);
	FHitResult HitResult;
	if (!bDoneDamage && UKismetSystemLibrary::SphereTraceSingle(
		this,
		StartLoc,
		EndLoc,
		20,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		HitResult,
		true))
	{
		bDoneDamage = true; // 标记已造成伤害，防止多次伤害
		UGameplayStatics::ApplyDamage(HitResult.GetActor(), AttackPoint, nullptr, this, UDamageType::StaticClass());
	}

}

void AZombie::EndDoDamage()
{
	bDoneDamage = false; // 重置标记，允许下一次造成伤害
}

void AZombie::Die(AActor* DamageCauser)
{
	UE_LOG(LogTemp, Log, TEXT("丧尸已死亡"));
	bIsDead = true;
	// 关闭胶囊体碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(FName("DeadZombie"));
	// 开启网格体模拟物理
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAnimInstanceClass(nullptr);
	AnimInst = nullptr;
	GetMesh()->SetCastShadow(false);
	GetMesh()->SetForcedLOD(3);

	// 把 AI 控制器从 Pawn 上摘掉
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->UnPossess();     // 取消控制
		AICon->Destroy();       // 彻底销毁控制器
	}

	// 延迟10s后回收到对象池
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (UZombiePool* Pool = GetGameInstance()->GetSubsystem<UZombiePool>(); Pool)
				{
					Pool->Release(this);
				}
			}),
		10.f,
		false);
}

void AZombie::BackToPool()
{
	Health = GetZombieData().MaxHealth;
	AttackPoint = GetZombieData().AttackPoint;
	bIsDead = false;
	PlayingHitAnim = nullptr;

	SetActorTickEnabled(false);
	//SetActorEnableCollision(false);
	SetActorHiddenInGame(true);

	GetMesh()->SetSimulatePhysics(false);
}

void AZombie::StartPlay()
{
	SetActorTickEnabled(true);
	//SetActorEnableCollision(true);
	SetActorHiddenInGame(false);

	GetMesh()->SetAnimInstanceClass(GetZombieData().AnimInstanceClass);  
	GetMesh()->SetCastShadow(true);
	GetMesh()->SetForcedLOD(0);
	GetMesh()->SetCollisionProfileName(FName("AliveZombie"));

	if (GetLocalRole() == ROLE_Authority)       // 只在服务器做
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.Owner = this;

		AAIController* NewAICon = GetWorld()->SpawnActor<AAIController>(GetZombieData().AIControllerClass, GetActorLocation(), GetActorRotation(), SpawnInfo);
		NewAICon->Possess(nullptr);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	AnimInst = GetMesh()->GetAnimInstance();
}

