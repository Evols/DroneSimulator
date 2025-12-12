#pragma once

#include "CoreMinimal.h"
#include "DroneInputTypes.generated.h"

UENUM(BlueprintType)
enum class EDroneInputAxis : uint8
{
    None,
    Throttle,
    Yaw,
    Pitch,
    Roll
};

UENUM(BlueprintType)
enum EThrottleCalibrationSpace : uint8
{
    PositiveNegative,
    PositiveOnly
};

UENUM(BlueprintType)
enum class EDroneCalibrationPhase : uint8
{
    None,
    WaitingForInput, // Phase 1: Waiting for user to move a stick to map it
    MeasuringLimits  // Phase 2: User wiggles sticks to find min/max
};

UENUM(BlueprintType)
enum class EDroneInputPrecisionMode : uint8
{
    LowPrecision,
    HighPrecision
};

USTRUCT(BlueprintType)
struct FInputAxisMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName input_device_axis_name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDroneInputAxis game_axis = EDroneInputAxis::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool is_inverted = false;
};

USTRUCT(BlueprintType)
struct FAxisCalibrationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float min_value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float max_value = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float zero_value = 0.5f;

    // Deadzone could be added here
};

USTRUCT(BlueprintType)
struct FDroneInputDevice
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 device_id = -1;

    UPROPERTY(BlueprintReadOnly)
    FString device_name;

    // Stable identifier for persistence (e.g. HID device path)
    UPROPERTY(BlueprintReadOnly)
    FString device_uid;

    // Map from Axis Name (e.g. "GenericUSBController_Axis1") to current raw value
    // Map from Axis Name (e.g. "GenericUSBController_Axis1") to current raw value
    UPROPERTY(BlueprintReadOnly)
    TMap<FName, float> raw_axes;

    // Raw Integer values for debugging
    UPROPERTY(BlueprintReadOnly)
    TMap<FName, int32> raw_int_axes;
};
