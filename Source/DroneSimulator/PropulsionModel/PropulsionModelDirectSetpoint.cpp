#include "DroneSimulator/PropulsionModel/PropulsionModelDirectSetpoint.h"

#include "DroneSimulator/Controller/Setpoint.h"
#include "DroneSimulator/Simulation/SubstepBody.h"

TOptional<FDynamicsPropellerSetInfo> UPropulsionModelDirectSetpoint::tick_propulsion(double delta_time,
    FSubstepBody* substep_body, const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
    const USimulationWorld* simulation_world)
{
    substep_body->angular_velocity_radians_world = substep_body->transform_world.TransformVectorNoScale(drone_setpoint.angular_velocity_radians);

    const auto drone_up_axis = substep_body->transform_world.GetUnitAxis(EAxis::Z);
    const auto force_to_apply = drone_up_axis * drone_setpoint.throttle * this->max_throttle_force;
    substep_body->add_force(force_to_apply);

    return {};
}
