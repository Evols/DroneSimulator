#include "DroneSimulatorCore/Public/RotorModel/RotorModelSimplified.h"
#include "DroneSimulatorCore/Public/Simulation/SimulationWorld.h"
#include "DroneSimulatorCore/Public/Simulation/SubstepBody.h"


FRotorSimulationResult URotorModelSimplified::simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
	const TDronePropeller* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
	const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world)
{
	if (!propeller->IsType<FDronePropellerSimplified>())
	{
		return FRotorSimulationResult(FThrustSimValue(), {}, FDebugLog());
	}

	const auto& propeller_simplified = propeller->Get<FDronePropellerSimplified>();
	const auto [air_density, wind_velocity] = simulation_world->get_wind_and_air_density();

	const auto rotor_angular_speed = throttle * battery->voltage * motor->kv; // in rad/s
	const auto rotor_rps = rotor_angular_speed / TWO_PI; // In rev/s

	const auto diameter = propeller_simplified.blade_diameter;

	const auto diameter_pow_4 = diameter * diameter * diameter * diameter;
	const auto thrust = propeller_simplified.thrust_coefficient * air_density * FMath::Square(rotor_rps) * diameter_pow_4;

	const auto diameter_pow_5 = diameter_pow_4 * diameter;
	const auto torque = propeller_simplified.torque_coefficient * air_density * FMath::Square(rotor_rps) * diameter_pow_5;

	// World-space prop axis (unit)
	const FVector thrust_axis = substep_body->transform_world.TransformVectorNoScale(FVector::UpVector).GetSafeNormal();
	const FVector force = thrust_axis * thrust;
	substep_body->add_force_at_point(force, propeller_location_local);

	const auto clockwise_factor = is_clockwise ? -1.0 : 1.0;
	const auto final_torque_value = clockwise_factor * FMath::Abs(torque);
	const FVector torque_vector = thrust_axis * final_torque_value;

	// debug_log.log(FString::Printf(TEXT("axis=%s"), *thrust_axis.ToCompactString()));
	// debug_log.log(FString::Printf(TEXT("torque=%s"), *torque.ToCompactString()));

	if (torque_vector.SizeSquared() > 0.0)
	{
		substep_body->add_torque(torque_vector);
	}

	const auto simulation_value = FThrustSimValue(thrust, torque);

	return FRotorSimulationResult(simulation_value, {}, FDebugLog());
}
