// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/FiringPatterns/FiringPattern.h"

void UFiringPattern::FireSingle(AWeaponBase* Weapon, AController* Instigator)
{
	UE_LOG(LogTemp, Warning, TEXT("FireSingle called in base class UFiringPattern. This should be overridden in derived classes."));
}
