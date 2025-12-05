#pragma once

#include "DroneSimulator/Controller/FlightMode.h"

#include "FlightModeAngle.generated.h"

/**
 * Angle flight mode (aka Stabilized mode)
 * The pilot controls the pitch and roll angles of the drone.
 * The controller will automatically stabilize the drone at the target angle.
 * This is more stable and easier to fly than Air mode (acro/rate mode).
 */
UCLASS(BlueprintType)
class UFlightModeAngle : public UFlightModeBase
{
    GENERATED_BODY()

public:

    // Maximum angle in degrees that the drone can tilt when the stick is fully deflected
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Angle Limits", meta=(DisplayName="Max Roll Angle (degrees)"))
    double max_roll_angle_deg = 45.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Angle Limits", meta=(DisplayName="Max Pitch Angle (degrees)"))
    double max_pitch_angle_deg = 45.0;

    // Yaw rate when the stick is fully deflected (angle mode doesn't control yaw angle, only yaw rate)
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Yaw Rate (degrees per second)"))
    double yaw_rate_deg_per_second = 200.0;

    // PID gains for the angle-to-angular-velocity controller
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Angle to Angular Velocity", meta=(DisplayName="Roll P gain"))
    double angle_roll_p = 6.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Angle to Angular Velocity", meta=(DisplayName="Pitch P gain"))
    double angle_pitch_p = 6.0;

    virtual ECameraTiltMode get_camera_tilt_mode() const override;

    virtual EThrottleCalibrationSpace get_throttle_calibration_space() const override;

    virtual FDroneSetpoint compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state) override;

};
