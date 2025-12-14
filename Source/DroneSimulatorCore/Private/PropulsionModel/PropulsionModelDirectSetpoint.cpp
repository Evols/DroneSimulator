#include "DroneSimulatorCore/Public/PropulsionModel/PropulsionModelDirectSetpoint.h"

#include "DroneSimulatorCore/Public/Controller/Setpoint.h"
#include "DroneSimulatorCore/Public/Simulation/SubstepBody.h"

TOptional<FDynamicsPropellerSetInfo> UPropulsionModelDirectSetpoint::tick_propulsion(double delta_time,
    FSubstepBody* substep_body, const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
    const USimulationWorld* simulation_world)
{
    substep_body->angular_velocity_radians_world = substep_body->transform_world.TransformVectorNoScale(drone_setpoint.angular_velocity_radians);

    const auto drone_up_axis = substep_body->transform_world.GetUnitAxis(EAxis::Z);

    const auto drone_vertical_velocity = FVector::DotProduct(drone_up_axis, substep_body->linear_velocity_world);
    const auto max_speed_factor = FMath::Max(0.0, 1.0 - drone_vertical_velocity / this->max_vertical_speed);

    const auto force_to_apply = drone_up_axis * drone_setpoint.throttle * max_speed_factor * this->max_throttle_force;
    substep_body->add_force(force_to_apply);

    return {};
}
