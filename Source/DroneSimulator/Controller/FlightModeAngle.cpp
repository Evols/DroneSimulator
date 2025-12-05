#include "DroneSimulator/Controller/FlightModeAngle.h"

ECameraTiltMode UFlightModeAngle::get_camera_tilt_mode() const
{
    return ECameraTiltMode::Stabilized;
}

EThrottleCalibrationSpace UFlightModeAngle::get_throttle_calibration_space() const
{
    return EThrottleCalibrationSpace::PositiveNegative;
}

FDroneSetpoint UFlightModeAngle::compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state)
{
    // Step 1: Convert player input to target angles
    const double target_roll_deg = player_input.roll * max_roll_angle_deg;
    const double target_pitch_deg = player_input.pitch * max_pitch_angle_deg;

    // Step 2: Get current angles from the flight state
    // Note: Unreal Engine uses Roll-Pitch-Yaw order
    const double current_roll_deg = flight_state.rotation.Roll;
    const double current_pitch_deg = flight_state.rotation.Pitch;

    // Step 3: Calculate angle errors (in degrees)
    const double roll_error_deg = target_roll_deg - current_roll_deg;
    const double pitch_error_deg = target_pitch_deg - current_pitch_deg;

    // Step 4: Apply P controller to convert angle error to desired angular velocity
    // This is a chained controller: angle error -> angular velocity setpoint
    const double desired_roll_rate_deg_s = roll_error_deg * angle_roll_p;
    const double desired_pitch_rate_deg_s = pitch_error_deg * angle_pitch_p;

    // Step 5: Convert to radians for the setpoint
    const FVector desired_angular_velocity = FVector(
        -FMath::DegreesToRadians(desired_roll_rate_deg_s),  // Negative because of Unreal's coordinate system
        -FMath::DegreesToRadians(desired_pitch_rate_deg_s), // Negative because of Unreal's coordinate system
        FMath::DegreesToRadians(player_input.yaw * yaw_rate_deg_per_second) // Yaw is still rate-controlled
    );

    return FDroneSetpoint(player_input.throttle, desired_angular_velocity);
}
