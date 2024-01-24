// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "LyraRPGRecuperateExecution.generated.h"

class UObject;


/**
 * ULyraHealExecution
 *
 *    Execution used by gameplay effects to apply healing to the health attributes.
 */
UCLASS()
class ULyraRPGRecuperateExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:

    ULyraRPGRecuperateExecution();

protected:

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
