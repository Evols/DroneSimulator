#include "DroneSimulator/Controller/FlightModeAir.h"


ECameraTiltMode UFlightModeAir::get_camera_tilt_mode() const
{
    return ECameraTiltMode::Tilted;
}

EThrottleCalibrationSpace UFlightModeAir::get_throttle_calibration_space() const
{
    return EThrottleCalibrationSpace::PositiveOnly;
}

FDroneSetpoint UFlightModeAir::compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state)
{
    const auto desired_angular_velocity = FVector(
        player_input.roll * -FMath::DegreesToRadians(this->roll_rate_deg_per_second), // Roll stick right = Input.Roll >0 = drone rotates clockwise = rotator roll >0
        player_input.pitch * -FMath::DegreesToRadians(this->pitch_rate_deg_per_second), // Pitch stick up = Input.Pitch >0 = look down = rotator pitch <0
        player_input.yaw * FMath::DegreesToRadians(this->yaw_rate_deg_per_second) // Yaw stick right = Input.Yaw >0 = look right = rotator yaw >0
    );

    return FDroneSetpoint(player_input.throttle, desired_angular_velocity);
}
