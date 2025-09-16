#pragma once
#include "CoreMinimal.h"
#include "FiringPattern.generated.h"

class AWeaponBase;
class AController;

/* 开火策略模式基类 */
UCLASS(Blueprintable)
class UFiringPattern : public UObject
{
    GENERATED_BODY()

public:
    /* 单次开火 */
    virtual void FireSingle(AWeaponBase* Weapon, AController* Instigator);

    UFUNCTION()
    static void ApplyGameplayEffect(AController* Instigator, const FHitResult& Hit);
};
