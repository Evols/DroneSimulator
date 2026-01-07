

#include "DroneSimulatorCore/Public/PropulsionModel/PropulsionModelDynamics.h"
#include "DroneSimulatorCore/Public/Controller/DroneController.h"
#include "DroneSimulatorCore/Public/RotorModel/RotorModelBase.h"
#include "DroneSimulatorCore/Public/Simulation/SubstepBody.h"

TOptional<FDynamicsPropellerSetInfo> UPropulsionModelDynamics::tick_propulsion(double delta_time,
    FSubstepBody* substep_body, const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
    const USimulationWorld* simulation_world)
{
    if (!drone_controller || !rotor_model)
    {
        return {};
    }

    const auto component_angular_velocity = substep_body->transform_world.GetRotation().Inverse().RotateVector(substep_body->angular_velocity_radians_world);

    const auto propeller_set_throttle = drone_controller->tick_controller(delta_time, drone_setpoint, component_angular_velocity);

    const auto* propeller = drone_setup.propeller;
    const auto* motor = drone_setup.motor;
    const auto* battery = drone_setup.battery;
    const auto* frame = drone_setup.frame;

    const auto front_left_throttle = propeller_set_throttle.front_left;
    const auto front_right_throttle = propeller_set_throttle.front_right;
    const auto rear_left_throttle = propeller_set_throttle.rear_left;
    const auto rear_right_throttle = propeller_set_throttle.rear_right;

    // Extents are distances; normalize signs so front is +X and back is -X. In unreal units
    const FVector props_extent_front = frame->props_extent_front.GetAbs();
    const FVector props_extent_back = frame->props_extent_back.GetAbs();

	// In unreal units
    const FVector front_left_location(props_extent_front.X, -props_extent_front.Y, props_extent_front.Z);
    const FVector front_right_location(props_extent_front.X, props_extent_front.Y, props_extent_front.Z);
    const FVector rear_left_location(-props_extent_back.X, -props_extent_back.Y, props_extent_back.Z);
    const FVector rear_right_location(-props_extent_back.X, props_extent_back.Y, props_extent_back.Z);

    const auto result_front_left = rotor_model->simulate_propeller_rotor(substep_body, front_left_throttle, propeller,
        motor, battery, front_left_location, true, simulation_world);
    const auto result_front_right = rotor_model->simulate_propeller_rotor(substep_body, front_right_throttle, propeller,
        motor, battery, front_right_location, false, simulation_world);
    const auto result_rear_left = rotor_model->simulate_propeller_rotor(substep_body, rear_left_throttle, propeller,
        motor, battery, rear_left_location, false, simulation_world);
    const auto result_rear_right = rotor_model->simulate_propeller_rotor(substep_body, rear_right_throttle, propeller,
        motor, battery, rear_right_location, true, simulation_world);

    TOptional<FDynamicsPropellerSetInfo> return_data = {};
    if (result_front_left.additional_data.IsSet() && result_front_right.additional_data.IsSet()
        && result_rear_left.additional_data.IsSet() && result_rear_right.additional_data.IsSet())
    {
        // TODO
    }

    return return_data;
}
