#pragma once

#include "Setpoint.generated.h"

USTRUCT(BlueprintType)
struct DRONESIMULATORCORE_API FDroneSetpoint
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite)
    double throttle;

    UPROPERTY(BlueprintReadWrite)
    FVector angular_velocity_radians;

    FDroneSetpoint();
    FDroneSetpoint(double in_throttle, const FVector& in_angular_velocity_radians);
};
