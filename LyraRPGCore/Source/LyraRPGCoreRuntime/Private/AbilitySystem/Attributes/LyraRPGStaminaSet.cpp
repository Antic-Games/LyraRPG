// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/LyraRPGStaminaSet.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "LyraGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameplayEffectExtension.h"
#include "Messages/LyraVerbMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraRPGStaminaSet)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_StaminaImmunity, "Gameplay.StaminaImmunity");

namespace LyraRPGGameplayTags
{
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedStamina, "Cheat.UnlimitedStamina", "UnlimitedStamina cheat is active on the owner.");
}

ULyraRPGStaminaSet::ULyraRPGStaminaSet()
    : Stamina(100.0f)
    , MaxStamina(100.0f)
{
    bOutOfStamina = false;
    MaxStaminaBeforeAttributeChange = 0.0f;
    StaminaBeforeAttributeChange = 0.0f;
}

void ULyraRPGStaminaSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ULyraRPGStaminaSet, Stamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(ULyraRPGStaminaSet, MaxStamina, COND_None, REPNOTIFY_Always);
}

void ULyraRPGStaminaSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraRPGStaminaSet, Stamina, OldValue);

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    // These events on the client should not be changing attributes

    const float CurrentStamina = GetStamina();
    const float EstimatedMagnitude = CurrentStamina - OldValue.GetCurrentValue();
    
    OnStaminaChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentStamina);

    if (!bOutOfStamina && CurrentStamina <= 0.0f)
    {
        OnOutOfStamina.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentStamina);
    }

    bOutOfStamina = (CurrentStamina <= 0.0f);
}

void ULyraRPGStaminaSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraRPGStaminaSet, MaxStamina, OldValue);

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    OnMaxStaminaChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxStamina() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxStamina());
}

bool ULyraRPGStaminaSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
    if (!Super::PreGameplayEffectExecute(Data))
    {
        return false;
    }

    // Save the current health
    StaminaBeforeAttributeChange = GetStamina();
    MaxStaminaBeforeAttributeChange = GetMaxStamina();

    return true;
}

void ULyraRPGStaminaSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    float MinimumStamina = 0.0f;

#if !UE_BUILD_SHIPPING
    // Godmode and unlimited health stop death unless it's a self destruct
    if (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) || Data.Target.HasMatchingGameplayTag(LyraRPGGameplayTags::Cheat_UnlimitedStamina) )
    {
        MinimumStamina = 1.0f;
    }
#endif // #if !UE_BUILD_SHIPPING

    const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
    AActor* Instigator = EffectContext.GetOriginalInstigator();
    AActor* Causer = EffectContext.GetEffectCauser();

    if (Data.EvaluatedData.Attribute == GetRecuperiatingAttribute())
    {
        // Convert into +Stamina and then clamo
        SetStamina(FMath::Clamp(GetStamina() + GetRecuperiating(), MinimumStamina, GetMaxStamina()));
        SetRecuperiating(0.0f);
    }
    else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        // Clamp and fall into out of stamina handling below
        SetStamina(FMath::Clamp(GetStamina(), MinimumStamina, GetMaxStamina()));
    }
    else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
    {
        // TODO clamp current health?

        // Notify on any requested max health changes
        OnMaxStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxStaminaBeforeAttributeChange, GetMaxStamina());
    }

    // If health has actually changed activate callbacks
    if (GetStamina() != StaminaBeforeAttributeChange)
    {
        OnStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
    }

    if ((GetStamina() <= 0.0f) && !bOutOfStamina)
    {
        OnOutOfStamina.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
    }

    // Check health again in case an event above changed it.
    bOutOfStamina = (GetStamina() <= 0.0f);
}

void ULyraRPGStaminaSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);

    ClampAttribute(Attribute, NewValue);
}

void ULyraRPGStaminaSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    ClampAttribute(Attribute, NewValue);
}

void ULyraRPGStaminaSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxStaminaAttribute())
    {
        // Make sure current health is not greater than the new max health.
        if (GetStamina() > NewValue)
        {
            ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
            check(LyraASC);

            LyraASC->ApplyModToAttribute(GetStaminaAttribute(), EGameplayModOp::Override, NewValue);
        }
    }

    if (bOutOfStamina && (GetStamina() > 0.0f))
    {
        bOutOfStamina = false;
    }
}

void ULyraRPGStaminaSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetStaminaAttribute())
    {
        // Do not allow health to go negative or above max health.
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    }
    else if (Attribute == GetMaxStaminaAttribute())
    {
        // Do not allow max health to drop below 1.
        NewValue = FMath::Max(NewValue, 1.0f);
    }
}
