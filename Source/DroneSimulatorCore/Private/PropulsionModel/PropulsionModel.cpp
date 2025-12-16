#include "DroneSimulatorCore/Public/PropulsionModel/PropulsionModel.h"


FPropulsionDroneSetup::FPropulsionDroneSetup(const FDroneFrame* in_frame, const FDroneMotor* in_motor,
    const FDroneBattery* in_battery, const TDronePropeller* in_propeller)
    : frame(in_frame), motor(in_motor), battery(in_battery), propeller(in_propeller)
{
}

TOptional<FDynamicsPropellerSetInfo> UPropulsionModel::tick_propulsion(double delta_time, FSubstepBody* substep_body,
    const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
    const USimulationWorld* simulation_world)
{
    return {};
}
