// Fill out your copyright notice in the Description page of Project Settings.


#include "Buff/DamageGameplayEffect.h"

#include "Buff/HealthAttributeSet.h"

UDamageGameplayEffect::UDamageGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.Attribute = UHealthAttributeSet::GetHealthAttribute();
	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag("Ability.OnBeDamaged.Damage");
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	Modifiers.Add(ModifierInfo);
}
