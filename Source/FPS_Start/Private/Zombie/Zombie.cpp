// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie/Zombie.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/DamageEvents.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Buff/DamageGameplayEffect.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Particles/ParticleSystemComponent.h"
#include "Zombie/ZombiePool.h"
#include "Zombie/ZombieAttributeSet.h"

void AZombie::OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	PlayingHitAnim = nullptr; // 清除正在播放的受击动画
}

void AZombie::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false; // 攻击动画结束，重置攻击状态

	// 通知行为树
	if (AAIController* AICon = Cast<AAIController>(GetController()))
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
			BB->SetValueAsBool("IsAttacking", false);
}

// Sets default values
AZombie::AZombie()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 创建 ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// 创建属性集并自动注册
	AttributeSet = CreateDefaultSubobject<UZombieAttributeSet>(TEXT("AttributeSet"));
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

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

	InitialAttributeSet();
}

void AZombie::InitialAttributeSet()
{
	AttributeSet->SetMaxHealth(GetZombieData().MaxHealth);
	AttributeSet->SetHealth(GetZombieData().MaxHealth);
	AttributeSet->SetMaxWalkSpeed(200.f);
	AttributeSet->ResetDeadStatus();
}

// Called every frame
void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 每帧读取速度属性，更新角色移动速度
	GetCharacterMovement()->MaxWalkSpeed = AttributeSet->GetMaxWalkSpeed();
	FString Message = FString::Printf(TEXT("Speed: %.1f"), AttributeSet->GetMaxWalkSpeed());

	if (UZombiePool* Pool = GetWorld()->GetSubsystem<UZombiePool>();Pool->ZombieStorage == EZombieStorageType::Grid)
	{
		const FIntPoint NewGrid = FZombieGrid::LocationToGrid(GetActorLocation());
		Pool->MoveZombie(this, OldGrid, NewGrid);
		UpdateOldGridWithNewGrid(NewGrid);
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

UAbilitySystemComponent* AZombie::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZombie::HandlePointDamage(float Damage, const FPointDamageEvent& PointEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	// 骨骼命中位置
	FName BoneName = PointEvent.HitInfo.BoneName;
	UAnimMontage* HitAnimMontage = SelectHitReactMontage(BoneName);

	if (GetZombieData().HeadBones.Contains(BoneName))Damage *= 2;// 如果是头部受击，则伤害翻倍

	if (!HitAnimMontage)
	{
		// 如果没有找到对应的受击动画，则直接返回
		return;
	}

	// 播放受击动画
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

	if (UAbilitySystemComponent* ASC = EventInstigator->GetCharacter()->GetComponentByClass<UAbilitySystemComponent>())
	{
		FGameplayEffectSpecHandle Handle = ASC->MakeOutgoingSpec(UDamageGameplayEffect::StaticClass(), 1.f, ASC->MakeEffectContext());
		Handle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Ability.OnBeDamaged.Damage"), -Damage);
		ASC->ApplyGameplayEffectSpecToTarget(*Handle.Data, AbilitySystemComponent);
	}
}

float AZombie::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (bIsDead) return ActualDamage;
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

void AZombie::Die(AActor* Attacker)
{
	// 触发击杀相关的 GA
	if (Attacker)
	{
		if (UAbilitySystemComponent* ASC = Attacker->FindComponentByClass<UAbilitySystemComponent>())
		{
			// 所有带 Ability.OnKill 的 Tag
			FGameplayTagContainer KillTags;
			KillTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.OnKill"));
			ASC->TryActivateAbilitiesByTag(KillTags, true);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("丧尸已死亡"));
	bIsDead = true;
	// 关闭胶囊体碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(FName("DeadZombie"));
	// 开启网格体模拟物理
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationCustomMode, false);
	GetMesh()->SetCastShadow(false);
	GetMesh()->SetForcedLOD(3);

	// 把 AI 控制器从 Pawn 上摘掉
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->UnPossess();     // 取消控制
		AICon->Destroy();       // 彻底销毁控制器
	}

	// 从存活列表中移除
	if (UZombiePool* Pool = GetWorld()->GetSubsystem<UZombiePool>(); Pool)
	{
		Pool->RemoveZombieFromAlive(this);
	}
	// 延迟后回收到对象池
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (UZombiePool* Pool = GetWorld()->GetSubsystem<UZombiePool>(); Pool)
				{
					Pool->Release(this);
				}
			}),
		5.f,
		false);
	// 触发丧尸死亡事件
	OnZombieDied.Broadcast(this);
}

void AZombie::BackToPool()
{
	RemoveAllParticle();

	InitialAttributeSet();
	AttackPoint = GetZombieData().AttackPoint;
	bIsDead = false;
	PlayingHitAnim = nullptr;

	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);

	GetMesh()->SetSimulatePhysics(false);

	// 网格体复位
	float OffsetZ = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -OffsetZ));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0));
	// 重置物理状态
	GetMesh()->ResetAllBodiesSimulatePhysics();
	GetMesh()->RecreatePhysicsState();
}

void AZombie::StartPlay()
{
	SetActorTickEnabled(true);
	SetActorHiddenInGame(false);

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint, false);
	GetMesh()->SetCastShadow(true);
	GetMesh()->SetForcedLOD(0);
	GetMesh()->SetCollisionProfileName(FName("AliveZombie"));

	if (GetLocalRole() == ROLE_Authority)       // 只在服务器做
	{
		UE_LOG(LogTemp, Warning, TEXT("丧尸AI开始运行"));
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AAIController* NewAICon = GetWorld()->SpawnActor<AAIController>(GetZombieData().AIControllerClass, GetActorLocation(), GetActorRotation(), SpawnInfo);
		NewAICon->Possess(this);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	AnimInst = GetMesh()->GetAnimInstance();
}

void AZombie::SpawnParticle(const FGameplayTag& ParticleTag, UParticleSystem* Particle)
{
	if (FParticleInfo* ParticleInfo = ParticleMap.Find(ParticleTag))// 找到对应的粒子特效信息
	{
		if (ParticleInfo->Count++==0)
		{
			ParticleInfo->ParticleSystemComponent->Activate();
		}
	}
	else // 没有找到对应的粒子特效信息
	{
		UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(
			Particle,
			GetMesh(),
			FName("pelvis"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(2, 2, 2),
			EAttachLocation::KeepRelativeOffset,
			false);
		ParticleMap.Add(ParticleTag, { 1, ParticleSystemComponent });
	}
}

void AZombie::RemoveParticle(const FGameplayTag& ParticleTag)
{
	if (FParticleInfo* ParticleInfo = ParticleMap.Find(ParticleTag))// 找到对应的粒子特效信息
	{
		if (--ParticleInfo->Count == 0)
		{
			ParticleInfo->ParticleSystemComponent.Get()->Deactivate();
		}
	}
}

void AZombie::RemoveAllParticle()
{
	for (auto ParticleInfo : ParticleMap)
	{
		if (ParticleInfo.Value.ParticleSystemComponent)
		{
			ParticleInfo.Value.ParticleSystemComponent.Get()->Deactivate();
			ParticleInfo.Value.ParticleSystemComponent.Get()->DestroyComponent();
		}
	}
	ParticleMap.Reset();
}

void AZombie::UpdateOldGrid()
{
	OldGrid = FZombieGrid::LocationToGrid(GetActorLocation());
}

void AZombie::UpdateOldGridWithNewGrid(const FIntPoint& NewGrid)
{
	OldGrid = NewGrid;
}
