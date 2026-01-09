#include "DroneSimulatorCore/Public/RotorModel/RotorModelDebug.h"
#include "DroneSimulatorCore/Public/Simulation/SubstepBody.h"

URotorModelDebug::URotorModelDebug()
{
	max_thrust = 15.0; // Assuming drone AUW = 600g, with a 10:1 thrust to weight ratio, and 4 propellers
	max_torque = 0.25;
}

FRotorSimulationResult URotorModelDebug::simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
	const TDronePropeller* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
	const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world)
{
	const double throttle_clamped = FMath::Clamp(throttle, 0.0, 1.0);
	const double thrust = this->max_thrust * throttle_clamped;
	double torque = this->max_torque * throttle_clamped;

	// World-space prop axis (unit)
	const FVector thrust_axis = substep_body->transform_world.TransformVectorNoScale(FVector::UpVector).GetSafeNormal();
	const FVector force = thrust_axis * thrust;
	substep_body->add_force_at_point(force, propeller_location_local);

	const auto clockwise_factor = is_clockwise ? -1.0 : 1.0;
	const auto final_torque_value = clockwise_factor * FMath::Abs(torque);
	const FVector torque_vector = thrust_axis * final_torque_value;

	if (torque_vector.SizeSquared() > 0.0)
	{
		substep_body->add_torque(torque_vector);
	}

	const auto simulation_value = FThrustSimValue(thrust, torque);

	return FRotorSimulationResult(simulation_value, {}, FDebugLog());
}
