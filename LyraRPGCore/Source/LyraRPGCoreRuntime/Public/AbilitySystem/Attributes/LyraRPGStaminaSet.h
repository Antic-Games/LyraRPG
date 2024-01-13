// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "NativeGameplayTags.h"

#include "LyraRPGStaminaSet.generated.h"

class UObject;
struct FFrame;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_StaminaImmunity);

namespace LyraRPGGameplayTags
{
LYRARPGCORERUNTIME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedStamina);
}

struct FGameplayEffectModCallbackData;

/**
 *
 */
UCLASS(BlueprintType)
class LYRARPGCORERUNTIME_API ULyraRPGStaminaSet : public ULyraAttributeSet
{
    GENERATED_BODY()
    
public:

    ULyraRPGStaminaSet();

    ATTRIBUTE_ACCESSORS(ULyraRPGStaminaSet, Stamina);
    ATTRIBUTE_ACCESSORS(ULyraRPGStaminaSet, MaxStamina);
    ATTRIBUTE_ACCESSORS(ULyraRPGStaminaSet, Recuperiating);

    // Delegate when health changes due to damage/healing, some information may be missing on the client
    mutable FLyraAttributeEvent OnStaminaChanged;

    // Delegate when max health changes
    mutable FLyraAttributeEvent OnMaxStaminaChanged;

    // Delegate to broadcast when the health attribute reaches zero
    mutable FLyraAttributeEvent OnOutOfStamina;

protected:

    UFUNCTION()
    void OnRep_Stamina(const FGameplayAttributeData& OldValue);

    UFUNCTION()
    void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

    virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:

    // The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "LyraRPG|Stamina", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData Stamina;

    // The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "LyraRPG|Stamina", Meta = (AllowPrivateAccess = true))
    FGameplayAttributeData MaxStamina;

    // Used to track when the health reaches 0.
    bool bOutOfStamina;

    // Store the health before any changes
    float MaxStaminaBeforeAttributeChange;
    float StaminaBeforeAttributeChange;

    // -------------------------------------------------------------------
    //    Meta Attribute (please keep attributes that aren't 'stateful' below
    // -------------------------------------------------------------------

    // Incoming stamina. This is mapped directly to +Stamina
    UPROPERTY(BlueprintReadOnly, Category="LyraRPG|Stamina", Meta=(AllowPrivateAccess=true))
    FGameplayAttributeData Recuperiating;
};
