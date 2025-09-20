// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Buff/HealthAttributeSet.h"
#include "ZombieAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class FPS_START_API UZombieAttributeSet : public UHealthAttributeSet
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxWalkSpeed;
public:
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UZombieAttributeSet, MaxWalkSpeed);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxWalkSpeed);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxWalkSpeed);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxWalkSpeed);
};
