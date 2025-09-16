// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Buff/HealthAttributeSet.h"
#include "Zombie.generated.h"

// Debuff粒子效果数据行
USTRUCT(BlueprintType)
struct FParticleInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystemComponent> ParticleSystemComponent;
};

USTRUCT(BlueprintType)
struct FZombieDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	float AttackPoint = 10.f;
	// 左半身骨骼列表
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TSet<FName> LeftBones;
	// 右半身骨骼列表
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TSet<FName> RightBones;
	// 头部骨骼列表
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TSet<FName> HeadBones;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TObjectPtr<UAnimMontage> HeadHitMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TObjectPtr<UAnimMontage> LeftArmHitMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Property")
	TObjectPtr<UAnimMontage> RightArmHitMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim")
	TArray<TObjectPtr<UAnimMontage>> AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim")
	TSubclassOf<UAnimInstance> AnimInstanceClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim")
	TArray<TObjectPtr<UAnimSequenceBase>> IdleAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim")
	TObjectPtr<UAnimSequenceBase> RunningAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TSubclassOf<AAIController> AIControllerClass;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZombieDiedSignature, class AZombie*, Zombie);

UCLASS()
class FPS_START_API AZombie : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FDataTableRowHandle ZombieDataRowHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float AttackPoint = 10.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UHealthAttributeSet> AttributeSet;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr <UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsDead = false;
	// 网格体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	TObjectPtr<UAnimInstance> AnimInst = nullptr;
	// 正在播放的受击动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<UAnimMontage> PlayingHitAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	bool bDoneDamage = false; // 标记是否已造成伤害，防止多次伤害
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsAttacking = false; // 标记是否正在攻击
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TMap<FGameplayTag, FParticleInfo> ParticleMap; // 存储粒子特效信息
	// 声明回调函数
	UFUNCTION()
	void OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	FZombieDataRow GetZombieData() const
	{
		if (ZombieDataRowHandle.DataTable)
		{
			return *ZombieDataRowHandle.GetRow<FZombieDataRow>(TEXT("丧尸数据未设置"));
		}
		return FZombieDataRow();
	}

public:
	// Sets default values for this character's properties
	AZombie();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// 初始化属性
	virtual void InitialAttributeSet();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	virtual UAnimMontage* SelectHitReactMontage(FName BoneName) const;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void HandlePointDamage(float Damage,
		const FPointDamageEvent& PointEvent,
		AController* EventInstigator,
		AActor* DamageCauser);

	virtual float TakeDamage(float Damage,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	virtual void Attack();
	UFUNCTION(BlueprintCallable)
	virtual void TryDoDamage(FName StartBoneName, FName EndBoneName);
	UFUNCTION(BlueprintCallable)
	virtual void EndDoDamage();

	// 处理死亡逻辑
	UFUNCTION(BlueprintCallable)
	virtual void Die(AActor* Attacker);

	// 重置状态，准备复用
	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void BackToPool();

	// 开始运行
	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void StartPlay();

	UFUNCTION(BlueprintCallable, Category = "Effect")
	virtual void SpawnParticle(const FGameplayTag& ParticleTag, UParticleSystem* Particle);

	UFUNCTION(BlueprintCallable, Category = "Effect")
	virtual void RemoveParticle(const FGameplayTag& ParticleTag);

	// 清空所有Particle
	UFUNCTION(BlueprintCallable, Category = "Effect")
	virtual void RemoveAllParticle();

	// 丧尸死亡事件
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FZombieDiedSignature OnZombieDied;
};
