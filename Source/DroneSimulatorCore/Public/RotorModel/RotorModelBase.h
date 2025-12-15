#pragma once

#include "Containers/Union.h"
#include "CoreMinimal.h"

#include "DroneSimulatorCore/Public/Simulation/Structural.h"
#include "DroneSimulatorCore/Public/Simulation/LogDebug.h"

#include "RotorModelBase.generated.h"

class USimulationWorld;
struct FSubstepBody;
struct FDronePropellerBemt;

struct FThrustSimValue
{
	double thrust = 0.0; // In Newtons
	double torque = 0.0; // In N·m

	FThrustSimValue() = default;

	FThrustSimValue(double in_thrust, double in_torque)
		: thrust(in_thrust), torque(in_torque) {}
};

struct FThrustSimBemtData {
	double angle_of_attack;
	const TArray<double> &reynolds;
	double v_induced;
	double v_axial;

	FThrustSimBemtData(double in_angle_of_attack, const TArray<double> &in_reynolds, double in_v_induced,
		double in_v_axial)
		: angle_of_attack(in_angle_of_attack), reynolds(in_reynolds), v_induced(in_v_induced), v_axial(in_v_axial) {}
};

// Structured additional data
using FThrustSimAdditionalData = TUnion<FThrustSimBemtData>;

struct FRotorSimulationResult
{
	FThrustSimValue value;
	TOptional<FThrustSimAdditionalData> additional_data;
	FDebugLog debug_log;

	FRotorSimulationResult(const FThrustSimValue& in_value,
		const TOptional<FThrustSimAdditionalData> &in_additional_data, const FDebugLog &in_debug_log)
		: value(in_value), additional_data(in_additional_data), debug_log(in_debug_log)
	{}
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class DRONESIMULATORCORE_API URotorModelBase : public UObject
{
	GENERATED_BODY()

public:

	virtual FRotorSimulationResult simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
		const FDronePropellerBemt* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
		const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world);
};
