#pragma once

#include "CoreMinimal.h"

#include "DroneSimulator/Controller/Throttle.h"

#include "DroneController.generated.h"

struct FDroneSetpoint;

UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class UDroneController : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @param current_angular_velocity In rad per second, around each axis
	 */
	UFUNCTION()
	virtual FPropellerSetThrottle tick_controller(float delta_time, const FDroneSetpoint& setpoint, const FVector& current_angular_velocity);
};
