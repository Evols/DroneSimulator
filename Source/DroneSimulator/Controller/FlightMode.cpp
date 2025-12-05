#include "DroneSimulator/Controller/FlightMode.h"


EThrottleCalibrationSpace UFlightModeBase::get_throttle_calibration_space() const
{
    return EThrottleCalibrationSpace::PositiveOnly;
}

ECameraTiltMode UFlightModeBase::get_camera_tilt_mode() const
{
    return ECameraTiltMode::Zero;
}

FDroneSetpoint UFlightModeBase::compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state)
{
    return FDroneSetpoint(0.0, FVector::ZeroVector);
}
