#pragma once

#include "DroneSimulator/PropulsionModel/PropulsionModelDynamics.h"
#include "DroneSimulator/Controller/DroneController.h"
#include "DroneSimulator/RotorModel/RotorModelBase.h"
#include "DroneSimulator/Simulation/SubstepBody.h"

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

    const auto result_front_left = rotor_model->simulate_propeller_rotor(substep_body, front_left_throttle, propeller,
        motor, battery, frame->props_extent_front * FVector(1.0, -1.0, 1.0), true, simulation_world);
    const auto result_front_right = rotor_model->simulate_propeller_rotor(substep_body, front_right_throttle, propeller,
        motor, battery, frame->props_extent_front * FVector(1.0, 1.0, 1.0), false, simulation_world);
    const auto result_rear_left = rotor_model->simulate_propeller_rotor(substep_body, rear_left_throttle, propeller,
        motor, battery, frame->props_extent_back * FVector(1.0, -1.0, 1.0), false, simulation_world);
    const auto result_rear_right = rotor_model->simulate_propeller_rotor(substep_body, rear_right_throttle, propeller,
        motor, battery, frame->props_extent_back * FVector(1.0, 1.0, 1.0), true, simulation_world);

    TOptional<FDynamicsPropellerSetInfo> return_data = {};
    if (result_front_left.additional_data.IsSet() && result_front_right.additional_data.IsSet()
        && result_rear_left.additional_data.IsSet() && result_rear_right.additional_data.IsSet())
    {
        // TODO
    }

    return return_data;
}
