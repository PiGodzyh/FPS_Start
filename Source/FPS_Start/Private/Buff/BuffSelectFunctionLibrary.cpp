// Fill out your copyright notice in the Description page of Project Settings.


#include "Buff/BuffSelectFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

bool UBuffSelectFunctionLibrary::IsInCurrentBuffs(const FGameplayTag& BuffTag, const TMap<FGameplayTag, FBuffInfo>& BuffLevel,
	int32& NextLevel)
{
	if (BuffLevel.Contains(BuffTag))
	{
		NextLevel = BuffLevel[BuffTag].Level + 1;
		return true;
	}
	return false;
}

bool UBuffSelectFunctionLibrary::IsMatchedWeapon(FGameplayTag WeaponTag, const FGameplayTagContainer& WeaponTags)
{
	// 如果没有要求武器标签，则视为匹配
	if (WeaponTags.Num() == 0)
		return true;
	return WeaponTag.MatchesAny(WeaponTags);
}

TArray<FSelectedBuff> UBuffSelectFunctionLibrary::SelectBuffClass(const UDataTable* BuffDataTable, const FGameplayTag& CurrentWeaponTag,
	const TMap<FGameplayTag, FBuffInfo>& BuffLevel)
{
	TArray<FSelectedBuff> AvailableBuffs = TArray<FSelectedBuff>();
	TArray<FSelectedBuff> SelectedBuffs = TArray<FSelectedBuff>();

	FGameplayTagContainer CurrentBuffTags;
	for (const auto& BuffTag:BuffLevel)
	{
		CurrentBuffTags.AddTagFast(BuffTag.Key);
	}

	if (!BuffDataTable) return AvailableBuffs;

	const TMap<FName, uint8*>& RowMap = BuffDataTable->GetRowMap();

	for (const TPair<FName, uint8*>& Pair : RowMap)
	{
		const FBuffDataRow* Row = reinterpret_cast<const FBuffDataRow*>(Pair.Value);
		FGameplayTag BuffTag = Row->BuffTag;
		int32 NextLevel = 1;
		if (IsInCurrentBuffs(BuffTag, BuffLevel, NextLevel))
		{
			// 已有该buff且未满级，升级
			if (NextLevel <= Row->MaxLevel)
			{
				AvailableBuffs.Add(FSelectedBuff(Pair.Key, NextLevel));
			}
		}
		else
		{
			// 没有该buff，检查武器标签、前置标签和排斥标签
			if (IsMatchedWeapon(CurrentWeaponTag, Row->RequiredWeaponTags) &&
				CurrentBuffTags.HasAll(Row->RequiredBuffTags) &&
				!CurrentBuffTags.HasAny(Row->RepulsiveBuffTags))
			{
				AvailableBuffs.Add(FSelectedBuff(Pair.Key, 1));
			}
		}
	}

	// 索引洗牌
	TArray<int32> Indices;
	for (int32 i = 0; i < AvailableBuffs.Num(); ++i) Indices.Add(i);
	// Fisher-Yates 洗牌
	for (int32 i = Indices.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0, i);
		Indices.Swap(i, j);
	}
	// 选择前3个
	for (int32 i = 0; i < FMath::Min(3, AvailableBuffs.Num()); ++i)
	{
		SelectedBuffs.Add(AvailableBuffs[Indices[i]]);
	}
	return SelectedBuffs;
}

FGameplayAbilitySpecHandle UBuffSelectFunctionLibrary::AddBuff(UAbilitySystemComponent* ASC, TMap<FGameplayTag, FBuffInfo>& BuffInfo,
	TSubclassOf<UGameplayAbility> AbilityClass, const FGameplayTag& BuffTag)
{
	if (!ASC || !AbilityClass) return FGameplayAbilitySpecHandle();
	if (BuffInfo.Contains(BuffTag))
	{
		ASC->ClearAllAbilitiesWithInputID(BuffInfo[BuffTag].InputID);
		BuffInfo[BuffTag].Level += 1;
	}
	else
	{
		BuffInfo.Add(BuffTag, FBuffInfo(1, BuffInfo.Num()));
	}
	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, BuffInfo[BuffTag].Level, BuffInfo[BuffTag].InputID);
	return ASC->GiveAbility(AbilitySpec);
}
