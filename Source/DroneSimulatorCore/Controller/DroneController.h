#pragma once

#include "CoreMinimal.h"

#include "DroneSimulatorCore/Controller/Throttle.h"

#include "DroneController.generated.h"

struct FDroneSetpoint;

/**
 * Drone controller class, that computes the command to send to the motors to achieve the desired drone state (also
 * called a setpoint).
 * 
 * The only "real" implementation of this is a PID. Other implementations are mostly for debugging.
 * 
 * From an architectural perspective, the controller takes a setpoint (FDroneSetpoint) and the current state of the 
 * drone, and computes the command to send to the motors (FPropellerSetThrottle)
 */
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
