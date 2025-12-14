#include "DroneSimulatorCore/Public/Controller/Setpoint.h"


FDroneSetpoint::FDroneSetpoint()
	: throttle(0.0), angular_velocity_radians(FVector::ZeroVector)
{
}

FDroneSetpoint::FDroneSetpoint(double in_throttle, const FVector& in_angular_velocity_radians)
    : throttle(in_throttle), angular_velocity_radians(in_angular_velocity_radians)
{
}
