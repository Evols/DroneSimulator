#include "DroneSimulatorCore/Public/Simulation/LinearDrag.h"
#include "DroneSimulatorCore/Public/Simulation/SubstepBody.h"
#include "DroneSimulatorCore/Public/Simulation/SimulationWorld.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"


void simulation::calculate_linear_drag(FSubstepBody* substep_body, const FDroneFrame& frame, const FDronePropellerBemt& propeller, const USimulationWorld* simulation_world)
{
	// We want it in m/s

	// Frame cda
	const auto frame_cda = frame.area * frame.drag_coefficient;

	// Props cda
	const auto prop_disc_area = PI * FMath::Square(propeller.radius);
	const auto prop_height = propeller.radius * 0.15; // assume the prop height is 15% of its radius
	const auto prop_side_area = propeller.radius * prop_height;
	const auto props_cda = FVector(prop_side_area * 2.0 * 0.95, prop_side_area * 2.0 * 0.95, prop_disc_area * 4.0 * 0.95);

	const auto total_cda = frame_cda + props_cda;

	// Apply linear drag

	const auto& transform = substep_body->transform_world;
	const auto frame_velocity = substep_body->linear_velocity_world;
	const auto [air_density, wind_velocity] = simulation_world->get_wind_and_air_density();

	// Air velocity relative to the drone
	const auto air_velocity = wind_velocity - frame_velocity;
	// The drone can be rotated. So, we want the air velocity relative to the drone's frame of reference
	const auto air_velocity_local = transform.InverseTransformVectorNoScale(air_velocity);

	// Quadratic per-axis: F = -0.5*rho * CdA_axis * v*|v|
	const auto drag_force_local = FVector(
		0.5 * air_density * total_cda.X * air_velocity_local.X * FMath::Abs(air_velocity_local.X),
		0.5 * air_density * total_cda.Y * air_velocity_local.Y * FMath::Abs(air_velocity_local.Y),
		0.5 * air_density * total_cda.Z * air_velocity_local.Z * FMath::Abs(air_velocity_local.Z)
	).GetClampedToMaxSize(500.0);

	const auto drag_force = transform.TransformVectorNoScale(drag_force_local);
	substep_body->add_force(drag_force);
}
