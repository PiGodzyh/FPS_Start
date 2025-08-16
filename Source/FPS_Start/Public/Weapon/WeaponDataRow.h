// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#pragma once

#include "Engine/DataTable.h"
#include "WeaponDataRow.generated.h"

class UFiringPattern;

USTRUCT(BlueprintType)
struct FWeaponDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ClipSize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadInitial = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadMax = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SpreadIncPerShot = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 PelletCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ReloadTime = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DamagePerPellet = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FireRateRPM = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bAllowAuto = false;

    /* 决定发射方式的策略类 */
    //UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MustImplement = "FiringPattern"))
    //TSubclassOf<class UFiringPattern> FiringPatternClass;
};
