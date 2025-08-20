// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponDataRow.generated.h"

class UFiringPattern;

USTRUCT(BlueprintType)
struct FWeaponDataRow : public FTableRowBase
{
    GENERATED_BODY()

	// 弹夹容量
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ClipSize = 30;

	// 初始散布
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadInitial = 0.f;

	// 最大散布
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadMax = 5.f;

	// 每次开火增加的散布
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadIncPerShot = 0.5f;

	// 散布从最大恢复到最初值的时间
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadRecoveryTime = 1.f;

	// 每次开火发射的子弹数
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 PelletCount = 1;

	// 换弹时间
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ReloadTime = 2.f;

	// 每颗子弹的伤害
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DamagePerPellet = 10.f;

	// 开火间隔，单位为秒
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FireIntervals = 0.6f;

	// 子弹初速度，单位为米每秒
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float InitialSpeed = 200.f;

	// 子弹重量，用于计算冲量，单位为千克
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float PelletMass = 0.025f;

	// 是否允许自动开火
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bAllowAuto = false;

    /* 使用的开火策略类 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UFiringPattern> FiringPatternClass;

	// 开火动作蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> FireMontage;

	// 换弹动作蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> ReloadMontage;

	// 子弹耗光时的换弹动作蒙太奇
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> DryReloadMontage;
};
