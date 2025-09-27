// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BuffSelectFunctionLibrary.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FBuffDataRow : public FTableRowBase
{
	GENERATED_BODY()

	// buff类
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility>BuffClass;
	// buff名称，用于展示给玩家
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText BuffName;
	// buff标签
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag BuffTag;
	// buff所需武器标签容器
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredWeaponTags;
	// buff前置标签容器
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredBuffTags;
	// buff排斥标签容器
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RepulsiveBuffTags;
	// buff最大等级
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxLevel;
	// buff描述
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;
};


USTRUCT(BlueprintType)
struct FSelectedBuff
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BuffRowName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Level;
};

// 用于记录buff等级和AddAbility时的对应InputID
USTRUCT(BlueprintType)
struct FBuffInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Level;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 InputID;
};
/**
 *
 */
UCLASS()
class FPS_START_API UBuffSelectFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
protected:
	// 判断BuffTag是否CurrentBuffTags中，如果有并且没满级则返回true并且OutLevel设置为当前等级+1，否则返回false
	static bool IsInCurrentBuffs(const FGameplayTag& BuffTag, const TMap<FGameplayTag, FBuffInfo>& BuffLevel, int32& NextLevel);
	// 判断WeaponTag是否在WeaponTags中
	static bool IsMatchedWeapon(FGameplayTag WeaponTag, const FGameplayTagContainer& WeaponTags);
public:
	// 从数据表中选择符合条件的buff类，返回buff类名和对应等级的键值对
	UFUNCTION(BlueprintCallable)
	static TArray<FSelectedBuff> SelectBuffClass(const UDataTable* BuffDataTable, const FGameplayTag& CurrentWeaponTag, const TMap<FGameplayTag, FBuffInfo>& BuffLevel);
	// 给ASC添加buff能力，并更新BuffInfo
	UFUNCTION(BlueprintCallable)
	static FGameplayAbilitySpecHandle AddBuff(UAbilitySystemComponent* ASC, UPARAM(Ref) TMap<FGameplayTag, FBuffInfo>& BuffInfo, TSubclassOf<UGameplayAbility> AbilityClass, const FGameplayTag& BuffTag);
};
