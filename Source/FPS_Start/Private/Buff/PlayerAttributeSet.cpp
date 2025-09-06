// Fill out your copyright notice in the Description page of Project Settings.


#include "Buff/PlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

void UPlayerAttributeSet::SetHealth(float NewValue)
{
	NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		ASC->SetNumericAttributeBase(GetHealthAttribute(), NewValue);
	}
}

bool UPlayerAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	Super::PreGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// 记录旧的最大生命值
		OldMaxHealth = GetMaxHealth();
	}
	return true;
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(GetHealth());
	}

	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// 根据新的最大生命值调整当前生命值
		const float NewMaxHealth = GetMaxHealth();
		const float OldHealth = GetHealth();
		const float NewHealth = NewMaxHealth - OldMaxHealth + OldHealth;
		SetHealth(NewHealth);
	}
}
