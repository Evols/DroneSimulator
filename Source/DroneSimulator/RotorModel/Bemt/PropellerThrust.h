#pragma once

#include "CoreMinimal.h"

#include "DroneSimulator/RotorModel/Bemt/ComputePropellerThrust.h"
#include "DroneSimulator/Simulation/Structural.h"
#include "DroneSimulator/Simulation/LogDebug.h"

struct FDroneBattery;
struct FDroneMotor;
struct FDronePropellerBemt;
struct FSubstepBody;
class USimulationWorld;

struct FPropellerSimInfo
{
	double angular_speed = 0.0; // rad/s
	double thrust = 0.0; // Newtons
	double torque = 0.0; // N.m
	double angle_of_attack = 0.0; // In radians
	TArray<double> reynolds = {};
	double v_induced = 0.0; // In m/s
	double v_axial = 0.0; // In m/s

	FPropellerSimInfo() = default;

	FPropellerSimInfo(double in_angular_speed, double in_thrust, double in_torque, double in_angle_of_attack,
		const TArray<double>& in_reynolds, double in_v_induced, double in_v_axial, const FDebugLog& in_debug_log)
		: angular_speed(in_angular_speed)
		, thrust(in_thrust)
		, torque(in_torque)
		, angle_of_attack(in_angle_of_attack)
		, reynolds(in_reynolds)
		, v_induced(in_v_induced)
		, v_axial(in_v_axial)
	{
	}

	FPropellerSimInfo(const FPropThrustResult& sim_result, double in_angular_speed, double torque_override)
		: angular_speed(in_angular_speed)
		, thrust(sim_result.thrust)
		, torque(torque_override)
		, angle_of_attack(sim_result.angle_of_attack)
		, reynolds(sim_result.reynolds)
		, v_induced(sim_result.v_induced)
		, v_axial(sim_result.v_axial)
	{
	}
};

namespace simulation_bemt
{
	/**
	 * Computes and applies the thrust of the propeller to the body instance
	 *
	 * @param substep_body Body instance to apply the propeller thrust to
	 * @param throttle Propeller throttle, in a 0..1 range
	 * @param propeller Propeller info
	 * @param motor Motor info. Kv is in rad/s
	 * @param battery Battery info
	 * @param propeller_location_local Local location of the propeller, relative to the frame
	 * @param is_clockwise Is a clockwise propeller
	 * @param simulation_world World
	 *
	 * @return Useful information for displaying. The function already applies the changes to the body instance
	 */
	TTuple<FPropellerSimInfo, FDebugLog> simulate_propeller_thrust(FSubstepBody* substep_body, double throttle,
		const FDronePropellerBemt* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
		const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world);
}
