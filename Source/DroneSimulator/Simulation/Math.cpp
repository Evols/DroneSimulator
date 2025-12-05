#include "DroneSimulator/Simulation/Math.h"

FVector math::delta_rotation_to_angular_velocity(const FRotator& delta_rotation, double delta_time)
{
	return rotator_to_euler_rad(delta_rotation) / delta_time;
}

FVector math::rotator_to_euler_rad(const FRotator& rotator)
{
	return rotator.Euler() * (PI / 180.0);
}
