#pragma once

#include "CoreMinimal.h"
#include "LogDebug.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDroneSimulatorDebug, Log, All);

USTRUCT(BlueprintType)
struct FDebugLog
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    TArray<FString> logs;

    void log(const FString& log);

    void append_debug_log(const FDebugLog& other);
};
