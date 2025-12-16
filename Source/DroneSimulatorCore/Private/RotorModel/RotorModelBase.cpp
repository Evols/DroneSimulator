#include "DroneSimulatorCore/Public/RotorModel/RotorModelBase.h"

FRotorSimulationResult URotorModelBase::simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
    const TDronePropeller* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
    const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world)
{
    return FRotorSimulationResult(
        FThrustSimValue(),
        {},
        FDebugLog()
    );
}
