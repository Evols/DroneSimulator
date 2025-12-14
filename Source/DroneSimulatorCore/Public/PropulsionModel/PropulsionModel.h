#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/RotorModel/Bemt/PropellerThrust.h"
#include "DroneSimulatorCore/Public/Simulation/LogDebug.h"

#include "PropulsionModel.generated.h"

class USimulationWorld;
struct FDroneBattery;
struct FDroneMotor;
struct FDronePropellerBemt;
struct FDroneSetpoint;
struct FSubstepBody;

struct DRONESIMULATORCORE_API FDynamicsPropellerInfo
{
	// In rad/s
	double angular_speed = 0.0;

	// In N
	double thrust = 0.0;

	// In N/m
	double torque = 0.0;

	// In radians
	double angle_of_attack = 0.0;

	// No unit
	TArray<double> reynolds = {};

	// 0..1
	double throttle = 0.0;

	// m/s
	double velocity_axial = 0.0;

	// m/s
	double velocity_induced = 0.0;

	FDebugLog debug_log;

	FDynamicsPropellerInfo() = default;

	FDynamicsPropellerInfo(double in_angular_speed, double in_thrust, double in_torque, double in_angle_of_attack,
		const TArray<double>& in_reynolds, double in_throttle, double in_velocity_axial, double in_velocity_induced,
		const FDebugLog& in_debug_log)
		: angular_speed(in_angular_speed)
		, thrust(in_thrust)
		, torque(in_torque)
		, angle_of_attack(in_angle_of_attack)
		, reynolds(in_reynolds)
		, throttle(in_throttle)
		, velocity_axial(in_velocity_axial)
		, velocity_induced(in_velocity_induced)
		, debug_log(in_debug_log)
	{}

	FDynamicsPropellerInfo(const FPropellerSimInfo& sim_info, const FDebugLog& debug_log, double in_throttle)
		: angular_speed(sim_info.angular_speed)
		, thrust(sim_info.thrust)
		, torque(sim_info.torque)
		, angle_of_attack(sim_info.angle_of_attack)
		, reynolds(sim_info.reynolds)
		, throttle(in_throttle)
		, velocity_axial(sim_info.v_axial)
		, velocity_induced(sim_info.v_induced)
		, debug_log(debug_log)
	{}
};

/**
 * Info for each propeller
 */
struct DRONESIMULATORCORE_API FDynamicsPropellerSetInfo
{
	FDynamicsPropellerInfo front_left;

	FDynamicsPropellerInfo front_right;

	FDynamicsPropellerInfo rear_left;

	FDynamicsPropellerInfo rear_right;
};

struct DRONESIMULATORCORE_API FPropulsionDroneSetup
{
	const FDroneFrame* frame;
	const FDroneMotor* motor;
	const FDroneBattery* battery;
	const FDronePropellerBemt* propeller;

	FPropulsionDroneSetup(const FDroneFrame* in_frame, const FDroneMotor* in_motor,
		const FDroneBattery* in_battery, const FDronePropellerBemt* in_propeller);
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class DRONESIMULATORCORE_API UPropulsionModel : public UObject
{
    GENERATED_BODY()

public:

    virtual TOptional<FDynamicsPropellerSetInfo> tick_propulsion(double delta_time, FSubstepBody* substep_body,
    	const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
    	const USimulationWorld* simulation_world);
};
