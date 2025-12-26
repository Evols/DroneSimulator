#include "DroneSimulatorCore/Public/RotorModel/Bemt/RotorModelBemt.h"
#include "DroneSimulatorCore/Public/RotorModel/Bemt/PropellerThrust.h"


FRotorSimulationResult URotorModelBemt::simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
    const TDronePropeller* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
    const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world)
{
	if (!propeller->IsType<FDronePropellerBemt>())
	{
	    return FRotorSimulationResult(FThrustSimValue(), {}, FDebugLog());
	}

	const auto& propeller_bemt = propeller->Get<FDronePropellerBemt>();

    const auto [simulation_output, debug_log] = simulation_bemt::simulate_propeller_thrust(substep_body, throttle, &propeller_bemt, motor, battery,
        propeller_location_local, is_clockwise, simulation_world);

    const auto simulation_value = FThrustSimValue(simulation_output.thrust, simulation_output.torque);

    return FRotorSimulationResult(simulation_value, {}, debug_log);
}
