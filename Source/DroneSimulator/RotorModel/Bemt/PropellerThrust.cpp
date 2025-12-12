#include "DroneSimulator/RotorModel/Bemt/PropellerThrust.h"
#include "DroneSimulator/RotorModel/Bemt/AirfoilCoefficients.h"
#include "DroneSimulator/Simulation/SubstepBody.h"
#include "DroneSimulator/Simulation/SimulationWorld.h"


double compute_motor_angular_speed(double throttle, const FDroneMotor* motor, const FDroneBattery* battery)
{
	const double motor_voltage = FMath::Clamp(throttle, 0.0, 1.0) * battery->voltage;
	constexpr auto motor_load = 0.7;

	// Motor.Kv is in radians per second per volt, not in RPM per volt
	return motor->kv * motor_voltage * motor_load;
}

TTuple<FPropellerSimInfo, FDebugLog> simulation_bemt::simulate_propeller_thrust(FSubstepBody* substep_body, double throttle,
	const FDronePropellerBemt* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
	const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world)
{
	FDebugLog debug_log;

	const double angular_speed_load = 0.8;
	const double angular_speed = compute_motor_angular_speed(throttle, motor, battery) * angular_speed_load;

	// Get world transform from the body (thread-safe for physics callback).
	const auto& transform = substep_body->transform_world;

	const auto [air_density, wind_velocity] = simulation_world->get_wind_and_air_density();

	// World-space prop axis (unit)
	const FVector thrust_axis = transform.TransformVectorNoScale(FVector::UpVector).GetSafeNormal();

	// Body linear velocity at hub
	const FVector component_velocity = substep_body->get_velocity_at_location(propeller_location_local); // m/s

	// Solve in SI
	const auto [result, result_log] = compute_thrust_and_torque(angular_speed, thrust_axis, wind_velocity, component_velocity,
		air_density, propeller);
	debug_log.append_debug_log(result_log);

	const FVector force = thrust_axis * result.thrust;
	substep_body->add_force_at_point(force, propeller_location_local);

	const auto clockwise_factor = is_clockwise ? -1.0 : 1.0;
	const auto final_torque_value = clockwise_factor * FMath::Abs(result.torque);
	const FVector torque = thrust_axis * final_torque_value;

	// debug_log.log(FString::Printf(TEXT("axis=%s"), *thrust_axis.ToCompactString()));
	// debug_log.log(FString::Printf(TEXT("torque=%s"), *torque.ToCompactString()));

	if (torque.SizeSquared() > 0.0)
	{
		substep_body->add_torque(torque);
	}

	return TTuple<FPropellerSimInfo, FDebugLog> {
		FPropellerSimInfo(result, angular_speed, final_torque_value),
		debug_log
	};
}
