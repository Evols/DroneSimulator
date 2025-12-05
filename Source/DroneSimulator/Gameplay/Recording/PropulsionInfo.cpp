#include "DroneSimulator/Gameplay/Recording/PropulsionInfo.h"

#include "DroneSimulator/RotorModel/Bemt/PropellerThrust.h"

FPropellerPropulsionInfo::FPropellerPropulsionInfo(double in_angular_speed, double in_thrust, double in_torque,
                                                   double in_angle_of_attack, const TArray<double>& in_reynolds, double in_throttle, double in_velocity_axial,
                                                   double in_velocity_induced, const FDebugLog& in_debug_log)
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

FPropellerPropulsionInfo FPropellerPropulsionInfo::from_simulation_output(const FPropellerSimInfo& in_sim_info, double in_throttle, const FDebugLog& in_debug_log)
{
    return FPropellerPropulsionInfo(
        in_sim_info.angular_speed,
        in_sim_info.thrust,
        in_sim_info.torque,
        in_sim_info.angle_of_attack,
        in_sim_info.reynolds,
        in_throttle,
        in_sim_info.v_axial,
        in_sim_info.v_induced,
        in_debug_log
    );
}
