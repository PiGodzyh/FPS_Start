// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponBase.h"
#include "Weapon/WeaponDataRow.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UDataTable> WeaponDataTableFinder(TEXT("/Game/Data/Weapon/DT_WeaponData.DT_WeaponData"));
	if (WeaponDataTableFinder.Succeeded())
	{
		WeaponDataTable = WeaponDataTableFinder.Object;
	}
	if (!WeaponDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("HitImpactTable not found!"));
	}
	WeaponData = FWeaponDataRow();
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	check(WeaponDataTable);
	FWeaponDataRow* WeaponDataRow = WeaponDataTable->FindRow<FWeaponDataRow>(Name, FString::Printf(TEXT("%s"), *Name.ToString()));

	WeaponData = *WeaponDataRow;
	FiringPattern = NewObject<UFiringPattern>(this, WeaponData.FiringPatternClass);
	check(FiringPattern);

	CurrentBullet = WeaponData.ClipSize;

	WeaponMesh = Cast<USkeletalMeshComponent>(GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	check(WeaponMesh)

	AnimInst = WeaponMesh->GetAnimInstance();
	check(AnimInst);

}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponBase::AfterFireSingle()
{
	CurrentSpread = FMath::Min(CurrentSpread + WeaponData.SpreadIncPerShot, WeaponData.SpreadMax);
	CurrentBullet--;
}

void AWeaponBase::FireSingle(AController* Attacker)
{
	if (CurrentBullet == 0)
	{
		StopFire();
		return;
	}
	if (FiringPattern)FiringPattern->FireSingle(this, Attacker);
	AfterFireSingle();

	AnimInst->Montage_Play(WeaponData.FireMontage, 1.2f);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	FString FuncName("PlayFireAnimation");
	if (OwnerPawn)
	{
		UFunction* Func = OwnerPawn->GetClass()->FindFunctionByName(FName(FuncName));
		if (Func)
		{
			OwnerPawn->ProcessEvent(Func, nullptr);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(3, 10, FColor::Red, TEXT("未找到函数"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(3, 10, FColor::Red, TEXT("武器没有拥有者"));
	}
}

void AWeaponBase::StartFire(AController* Attacker)
{
	GEngine->AddOnScreenDebugMessage(1, 10, FColor::Green, TEXT("开始射击"));
	// 设置定时器：每 FireInterval 调用一次 FireSingle
	GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, Attacker]()
			{
				FireSingle(Attacker);
			}),
		WeaponData.FireIntervals,
		WeaponData.bAllowAuto,
		0.f);
}

void AWeaponBase::StopFire()
{
	GEngine->AddOnScreenDebugMessage(1, 10, FColor::Green, TEXT("停止射击"));
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
	CurrentSpread = 0.f; // 停止射击时重置散射角度
}

void AWeaponBase::Reload()
{
	if (CurrentBullet >= WeaponData.ClipSize)
	{
		GEngine->AddOnScreenDebugMessage(1, 10, FColor::Red, TEXT("弹夹已满"));
		return;
	}
	UAnimMontage* ReloadMontage = nullptr;
	bool bIsDry = false;
	if (CurrentBullet == 0 && WeaponData.DryReloadMontage)
	{
		bIsDry = true;
		ReloadMontage = WeaponData.DryReloadMontage;
	}
	if (CurrentBullet > 0 && WeaponData.ReloadMontage)
	{
		bIsDry = false;
		ReloadMontage = WeaponData.ReloadMontage;
	}
	if (ReloadMontage)
	{
		float Duration = ReloadMontage->GetPlayLength();
		float ReloadTime = WeaponData.ReloadTime;
		AnimInst->Montage_Play(ReloadMontage, Duration / ReloadTime);


		FString FuncName("PlayReloadAnimation");
		FPlayReloadAnimParams Params = { Duration / ReloadTime, bIsDry};
		GEngine->AddOnScreenDebugMessage(30, 10, FColor::Red, FString::Printf(TEXT("%f"), Params.PlayRate));
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn)
		{
			UFunction* Func = OwnerPawn->GetClass()->FindFunctionByName(FName(FuncName));
			if (Func)
			{
				OwnerPawn->ProcessEvent(Func, &Params);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(3, 10, FColor::Red, TEXT("未找到函数"));
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(3, 10, FColor::Red, TEXT("武器没有拥有者"));
		}
	}
	GetWorldTimerManager().SetTimer(
		ReloadTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				CurrentBullet = WeaponData.ClipSize;
			}),
		WeaponData.ReloadTime,
		false);
}


