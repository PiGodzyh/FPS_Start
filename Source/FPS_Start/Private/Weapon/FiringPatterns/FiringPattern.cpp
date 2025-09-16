// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/FiringPatterns/FiringPattern.h"

#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

void UFiringPattern::FireSingle(AWeaponBase* Weapon, AController* Instigator)
{
	UE_LOG(LogTemp, Warning, TEXT("FireSingle called in base class UFiringPattern. This should be overridden in derived classes."));
}

void UFiringPattern::ApplyGameplayEffect(AController* Instigator, const FHitResult& Hit)
{
    if (UAbilitySystemComponent* ASC = Instigator->GetCharacter()->FindComponentByClass<UAbilitySystemComponent>())
    {
        // 所有带 Ability.OnAttackHit 的 Tag
        FGameplayTagContainer AttackTags;
        AttackTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.OnAttackHit"));
        ASC->TryActivateAbilitiesByTag(AttackTags, true);
    }

    FGameplayEventData Payload;
    Payload.Instigator = Instigator;
    Payload.Target = Hit.GetActor();
    Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        Instigator->GetCharacter(),
        FGameplayTag::RequestGameplayTag("Event.OnAttackHit"),
        Payload);
}