// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponDataRow.h"
#include "FiringPatterns/FiringPattern.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

USTRUCT(BlueprintType)
struct FPlayReloadAnimParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float PlayRate;

	UPROPERTY(BlueprintReadOnly)
	bool IsDry;
};

UCLASS(Blueprintable, Abstract)
class FPS_START_API AWeaponBase : public AActor
{
	GENERATED_BODY()

protected:
	// 定时器句柄
	FTimerHandle FireTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponData")
	FDataTableRowHandle WeaponDataRowHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponData")
	FWeaponDataRow WeaponData;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire")
	float CurrentSpread = 0.f;

	UPROPERTY(VisibleDefaultsOnly, Category = "Fire")
	TObjectPtr<UFiringPattern> FiringPattern = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire")
	int32 CurrentBullet = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire") 
	bool bIsFiring = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* WeaponMesh;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	FTimerHandle ReloadTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	UAnimInstance* AnimInst;

public:
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 获取当前散射角度
	float GetCurrentSpread() const { return CurrentSpread; }

	// 获取武器数据
	const FWeaponDataRow& GetWeaponData() const { return WeaponData; }

	// 获取武器Tah
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FGameplayTag GetWeaponTag() const { return WeaponData.WeaponTag; }

	// 发射单发后减少子弹等逻辑
	virtual void AfterFireSingle();

	// 单次发射
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void FireSingle(AController* Attacker);

	// 开始发射
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFire(AController* Attacker);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Reload();


};
