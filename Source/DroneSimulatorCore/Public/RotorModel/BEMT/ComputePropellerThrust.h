#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/Simulation/LogDebug.h"

struct FDronePropellerBemt;

struct FPropThrustResult
{
    double thrust = 0.0; // In Newtons
    double torque = 0.0; // In N·m
    double angle_of_attack = 0.0; // In radians
    TArray<double> reynolds = {};
    double v_induced = 0.0; // In m/s
    double v_axial = 0.0; // In m/s

    FPropThrustResult() = default;

    FPropThrustResult(double in_thrust, double in_torque, double in_angle_of_attack, const TArray<double>& in_reynolds,
        double in_v_induced, double in_v_axial)
        : thrust(in_thrust)
        , torque(in_torque)
        , angle_of_attack(in_angle_of_attack)
        , reynolds(in_reynolds)
        , v_induced(in_v_induced)
        , v_axial(in_v_axial)
    {
    }
};

namespace simulation_bemt
{
    /**
     * Computes the axial velocity of the air for a given propeller
     * @param thrust_axis Unit vector, which direction is the up axis of the propeller
     * @param wind_velocity Velocity of the global wind, in the surrounding of the propeller, in m/s
     * @param propeller_velocity Velocity of the propeller object (not the air), in m/s
     * @return Axial velocity of the freestream, in m/s
     */
    double compute_axial_velocity(const FVector& thrust_axis, const FVector& wind_velocity,
        const FVector& propeller_velocity);

    /**
     *
     * @param propeller_angular_speed Angular speed of the propeller, in rad/s
     * @param thrust_axis Axis of the propeller
     * @param wind_velocity Velocity of the wind, in m/s
     * @param propeller_velocity Velocity of the propeller object, in m/s
     * @param air_density Density of the air
     * @param propeller Propeller info
     * @return Result of the simulation of thrust and torque. Adds additional info for displaying
     */
    TTuple<FPropThrustResult, FDebugLog> compute_thrust_and_torque(double propeller_angular_speed, const FVector& thrust_axis,
        const FVector& wind_velocity, const FVector& propeller_velocity, double air_density,
        const FDronePropellerBemt* propeller);
}
