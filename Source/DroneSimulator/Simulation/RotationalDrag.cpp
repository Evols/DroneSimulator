#include "DroneSimulator/Simulation/RotationalDrag.h"
#include "DroneSimulator/Simulation/SubstepBody.h"


void simulation::calculate_rotational_drag(FSubstepBody* substep_body, const FDroneFrame& frame, const USimulationWorld* simulation_world)
{
	const auto& transform = substep_body->transform_world;

	// Rotational aero drag (yaw/pitch/roll), quadratic in angular speed
	const FVector omega_ws = substep_body->angular_velocity_radians_world;
	const FVector omega_ls = transform.InverseTransformVectorNoScale(omega_ws);
	const FVector k_omega(0.001, 0.001, 0.002); // tune per axis (N·m per rad/s^2-ish)

	FVector tau_aero_ls(
		-k_omega.X * omega_ls.X * FMath::Abs(omega_ls.X),
		-k_omega.Y * omega_ls.Y * FMath::Abs(omega_ls.Y),
		-k_omega.Z * omega_ls.Z * FMath::Abs(omega_ls.Z)
	);


	const auto torque = transform.TransformVectorNoScale(tau_aero_ls);
	substep_body->add_torque(torque);
}
