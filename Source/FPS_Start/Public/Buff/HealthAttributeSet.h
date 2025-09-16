// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.generated.h"

/**
 * 基础血量属性集
 */
UCLASS()
class FPS_START_API UHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
private:
	UPROPERTY(Transient)
	float OldMaxHealth;
	bool bIsDead = false;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Health;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxHealth;
public:
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UHealthAttributeSet, Health);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Health);
	void SetHealth(float NewValue);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Health);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UHealthAttributeSet, MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxHealth);

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void ResetDeadStatus();
};
