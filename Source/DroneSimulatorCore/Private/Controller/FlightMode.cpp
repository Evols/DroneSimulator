#include "DroneSimulatorCore/Public/Controller/FlightMode.h"


#ifdef WITH_DRONE_INPUT

EThrottleCalibrationSpace UFlightModeBase::get_throttle_calibration_space() const
{
    return EThrottleCalibrationSpace::PositiveOnly;
}

#endif

ECameraTiltMode UFlightModeBase::get_camera_tilt_mode() const
{
    return ECameraTiltMode::Zero;
}

FDroneSetpoint UFlightModeBase::compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state)
{
    return FDroneSetpoint(0.0, FVector::ZeroVector);
}
