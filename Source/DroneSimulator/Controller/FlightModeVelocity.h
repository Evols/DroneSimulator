#pragma once

#include "DroneSimulator/Controller/FlightMode.h"

#include "FlightModeVelocity.generated.h"

/**
 * Velocity flight mode (aka Position mode / GPS mode)
 * The pilot controls the horizontal and vertical velocity of the drone.
 * The controller uses a cascaded architecture:
 *
 * Horizontal:
 *   1. Velocity error -> Target angle (via PI controller)
 *   2. Angle error -> Angular velocity (via P controller)
 *   3. Angular velocity -> Motor commands (handled by main PID controller)
 *
 * Vertical:
 *   1. Throttle stick (-1 to +1) -> Target vertical velocity
 *   2. Vertical velocity error -> Throttle adjustment (via PID)
 *   3. Final throttle = Hover throttle + Adjustment
 *
 * This provides very stable, GPS-like flight behavior with altitude hold.
 */
UCLASS(BlueprintType)
class UFlightModeVelocity : public UFlightModeBase
{
    GENERATED_BODY()

public:

    // Maximum velocity in m/s when the stick is fully deflected
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Velocity Limits", meta=(DisplayName="Max Horizontal Velocity (m/s)"))
    double max_horizontal_velocity_m_s = 10.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Velocity Limits", meta=(DisplayName="Max Vertical Velocity (m/s)"))
    double max_vertical_velocity_m_s = 5.0;

    // Maximum angle in degrees that the velocity controller can command
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Angle Limits", meta=(DisplayName="Max Tilt Angle (degrees)"))
    double max_tilt_angle_deg = 30.0;

    // Yaw rate when the stick is fully deflected
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Yaw Rate (degrees per second)"))
    double yaw_rate_deg_per_second = 120.0;

    // PI gains for the horizontal velocity-to-angle controller
    // D term removed - adds noise and is unnecessary for velocity control
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PI - Horizontal Velocity to Angle", meta=(DisplayName="Velocity P gain"))
    double velocity_p = 2.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PI - Horizontal Velocity to Angle", meta=(DisplayName="Velocity I gain"))
    double velocity_i = 0.5;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PI - Horizontal Velocity to Angle", meta=(DisplayName="Velocity I Max (anti-windup)"))
    double velocity_i_max_deg = 10.0;

    // P gains for the angle-to-angular-velocity controller (same as angle mode)
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Angle to Angular Velocity", meta=(DisplayName="Angle Roll P gain"))
    double angle_roll_p = 6.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Angle to Angular Velocity", meta=(DisplayName="Angle Pitch P gain"))
    double angle_pitch_p = 6.0;

    // PID gains for vertical velocity-to-throttle controller
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Vertical Velocity to Throttle", meta=(DisplayName="Vertical Velocity P gain"))
    double vertical_velocity_p = 0.5;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Vertical Velocity to Throttle", meta=(DisplayName="Vertical Velocity I gain"))
    double vertical_velocity_i = 0.5;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Vertical Velocity to Throttle", meta=(DisplayName="Vertical Velocity D gain"))
    double vertical_velocity_d = 0.05;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Vertical Velocity to Throttle", meta=(DisplayName="Vertical Velocity I Max (anti-windup)"))
    double vertical_velocity_i_max = 2.0;

    // Hover throttle - the throttle value needed to maintain altitude (typically ~0.5 for hovering)
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID - Vertical Velocity to Throttle", meta=(DisplayName="Hover Throttle"))
    double hover_throttle = 0.5;

    virtual ECameraTiltMode get_camera_tilt_mode() const override;

    virtual EThrottleCalibrationSpace get_throttle_calibration_space() const override;

    virtual FDroneSetpoint compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state) override;

private:
    // PI state (integral terms) for horizontal velocity
    double velocity_x_integral = 0.0;
    double velocity_y_integral = 0.0;

    // PID state for vertical velocity
    double vertical_velocity_integral = 0.0;
    double prev_vertical_velocity_error = 0.0;
};

