// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LyraRPGStaminaComponent.h"

#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraRPGStaminaSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"

#include "LyraLogChannels.h"

ULyraRPGStaminaComponent::ULyraRPGStaminaComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    AbilitySystemComponent = nullptr;
    StaminaSet = nullptr;
}

void ULyraRPGStaminaComponent::InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC)
{
    AActor* Owner = GetOwner();
    check(Owner);

    if (AbilitySystemComponent)
    {
        UE_LOG(LogLyra, Error, TEXT("LyraRPGStaminaComponent: Stamina component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
        return;
    }

    AbilitySystemComponent = InASC;
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogLyra, Error, TEXT("LyraRPGStaminaComponent: Cannot initialize Stamina component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
        return;
    }

    StaminaSet = AbilitySystemComponent->GetSet<ULyraRPGStaminaSet>();
    if (!StaminaSet)
    {
        UE_LOG(LogLyra, Error, TEXT("LyraRPGStaminaComponent: Cannot initialize Stamina component for owner [%s] with NULL Stamina set on the ability system."), *GetNameSafe(Owner));
        return;
    }

    // Register to listen for attribute changes.
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ULyraRPGStaminaSet::GetStaminaAttribute()).AddUObject(this, &ThisClass::HandleStaminaChanged);
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ULyraRPGStaminaSet::GetMaxStaminaAttribute()).AddUObject(this, &ThisClass::HandleMaxStaminaChanged);
    
    OnStaminaChanged.Broadcast(this, 0.0f, 100.0f);
}

void ULyraRPGStaminaComponent::UninitializeFromAbilitySystem()
{
    StaminaSet = nullptr;
    AbilitySystemComponent = nullptr;
}

float ULyraRPGStaminaComponent::GetStamina() const
{
    return (StaminaSet ? StaminaSet->GetStamina() : 0.0f);
}

float ULyraRPGStaminaComponent::GetMaxStamina() const
{
    return (StaminaSet ? StaminaSet->GetMaxStamina() : 0.0f);
}

float ULyraRPGStaminaComponent::GetStaminaNormalized() const
{
    if (StaminaSet)
    {
        const float Stamina = StaminaSet->GetStamina();
        const float MaxStamina = StaminaSet->GetMaxStamina();

        return ((MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f);
    }

    return 0.0f;
}

void ULyraRPGStaminaComponent::OnUnregister()
{
    UninitializeFromAbilitySystem();
    
    Super::OnUnregister();
}

void ULyraRPGStaminaComponent::HandleStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
    OnStaminaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
}

void ULyraRPGStaminaComponent::HandleMaxStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
    OnMaxStaminaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
}
