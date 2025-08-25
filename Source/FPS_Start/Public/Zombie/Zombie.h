// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Zombie.generated.h"

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
};

UCLASS()
class FPS_START_API AZombie : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FDataTableRowHandle ZombieDataRowHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float Health = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	float AttackPoint = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	bool bIsDead = false;
	// 网格体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	TObjectPtr<UAnimInstance> AnimInst = nullptr;
	// 正在播放的受击动画
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<UAnimMontage> PlayingHitAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
	bool bDoneDamage = false; // 标记是否已造成伤害，防止多次伤害
	// 声明回调函数
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);


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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	virtual UAnimMontage* SelectHitReactMontage(FName BoneName) const;

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
	virtual void Die(AActor* DamageCauser);

	// 重置状态，准备复用
	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void BackToPool();

	// 开始运行
	UFUNCTION(BlueprintCallable, Category = "State")
	virtual void StartPlay();
};
