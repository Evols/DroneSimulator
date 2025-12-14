#include "DroneSimulatorCore/Public/Controller/DroneController.h"


FPropellerSetThrottle UDroneController::tick_controller(float delta_time, const FDroneSetpoint& setpoint, const FVector& current_angular_velocity)
{
	return FPropellerSetThrottle { 0.0, 0.0, 0.0, 0.0 };
}
