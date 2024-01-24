// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Executions/LyraRPGRecuperateExecution.h"

struct FRecuperateStatics
{
    FGameplayEffectAttributeCaptureDefinition BaseRecuperateDef;

    FRecuperateStatics()
    {
        BaseRecuperateDef = FGameplayEffectAttributeCaptureDefinition(ULyraRPGStaminaSet::GetBaseRecuperateAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
    }
};

static FRecuperateStatics& RecuperateStatics()
{
    static FRecuperateStatics Statics;
    return Statics;
}


ULyraRPGRecuperateExecution::ULyraRPGRecuperateExecution()
{
    RelevantAttributesToCapture.Add(RecuperateStatics().BaseRecuperateDef);
}

void ULyraRPGRecuperateExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluateParameters;
    EvaluateParameters.SourceTags = SourceTags;
    EvaluateParameters.TargetTags = TargetTags;

    float BaseRecuperate = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(RecuperateStatics().BaseRecuperateDef, EvaluateParameters, BaseRecuperate);

    const float RecuperatingDone = FMath::Max(0.0f, BaseRecuperate);

    if (RecuperatingDone > 0.0f)
    {
        // Apply a recupering modifier, this gets turned into + stamina on the target
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULyraRPGStaminaSet::GetRecuperatingAttribute(), EGameplayModOp::Additive, RecuperatingDone));
    }
#endif // #if WITH_SERVER_CODE
}

