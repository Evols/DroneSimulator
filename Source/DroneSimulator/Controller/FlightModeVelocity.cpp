#include "DroneSimulator/Controller/FlightModeVelocity.h"


ECameraTiltMode UFlightModeVelocity::get_camera_tilt_mode() const
{
    return ECameraTiltMode::Stabilized;
}

EThrottleCalibrationSpace UFlightModeVelocity::get_throttle_calibration_space() const
{
    return EThrottleCalibrationSpace::PositiveNegative;
}

FDroneSetpoint UFlightModeVelocity::compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state)
{
    // ============================================================================
    // LEVEL 1: Player Input -> Target Velocity (in world frame)
    // ============================================================================

    // Get the drone's current yaw to transform input to world frame
    const double current_yaw_rad = FMath::DegreesToRadians(flight_state.rotation.Yaw);
    const double cos_yaw = FMath::Cos(current_yaw_rad);
    const double sin_yaw = FMath::Sin(current_yaw_rad);

    const auto player_input_vector = FVector2D(-player_input.pitch, player_input.roll);

    // Transform player input from body frame to world frame
    // Player input: X = forward/back, Y = left/right
    // We rotate by the drone's yaw to get world frame velocities
    const auto target_velocity_world = player_input_vector.GetRotated(flight_state.rotation.Yaw) * max_horizontal_velocity_m_s;

    // ============================================================================
    // LEVEL 2: Velocity Error -> Target Angle (via PID controller)
    // ============================================================================

    // Get current velocity in world frame
    const auto current_velocity_world = FVector2D(flight_state.linear_velocity_world);

    // Calculate velocity errors
    const auto velocity_error = target_velocity_world - current_velocity_world;

    // Update integral terms with anti-windup clamping
    // Using PI controller (no D term) - I term helps eliminate steady-state errors from wind/drift
    const double dt = flight_state.delta_time;
    if (dt > 0.0)
    {
        velocity_x_integral += velocity_error.X * dt;
        velocity_y_integral += velocity_error.Y * dt;

        // Anti-windup: clamp integral to prevent excessive buildup
        velocity_x_integral = FMath::Clamp(velocity_x_integral, -velocity_i_max_deg / velocity_i, velocity_i_max_deg / velocity_i);
        velocity_y_integral = FMath::Clamp(velocity_y_integral, -velocity_i_max_deg / velocity_i, velocity_i_max_deg / velocity_i);
    }

    // PI output: desired angles in world frame (no D term - adds noise and instability)
    const double target_angle_x_deg =
        -(velocity_p * velocity_error.X) -
        (velocity_i * velocity_x_integral);

    const double target_angle_y_deg =
        (velocity_p * velocity_error.Y) +
        (velocity_i * velocity_y_integral);

    // Clamp angles to max tilt
    const double clamped_angle_x_deg = FMath::Clamp(target_angle_x_deg, -max_tilt_angle_deg, max_tilt_angle_deg);
    const double clamped_angle_y_deg = FMath::Clamp(target_angle_y_deg, -max_tilt_angle_deg, max_tilt_angle_deg);

    // Transform target angles from world frame to body frame (roll/pitch)
    // In world frame: X angle tilts North/South, Y angle tilts East/West
    // In body frame: Roll tilts left/right, Pitch tilts forward/back
    // Use INVERSE rotation (transpose of rotation matrix) to go from world -> body
    const double target_pitch_deg =
        (clamped_angle_x_deg * cos_yaw) -
        (clamped_angle_y_deg * sin_yaw);

    const double target_roll_deg =
        (clamped_angle_x_deg * sin_yaw) +
        (clamped_angle_y_deg * cos_yaw);

    // ============================================================================
    // LEVEL 3: Angle Error -> Angular Velocity (via P controller)
    // ============================================================================

    // Get current angles
    const double current_roll_deg = flight_state.rotation.Roll;
    const double current_pitch_deg = flight_state.rotation.Pitch;

    // Calculate angle errors
    const double roll_error_deg = target_roll_deg - current_roll_deg;
    const double pitch_error_deg = target_pitch_deg - current_pitch_deg;

    // Apply P controller to convert angle error to desired angular velocity
    const double desired_roll_rate_deg_s = roll_error_deg * angle_roll_p;
    const double desired_pitch_rate_deg_s = pitch_error_deg * angle_pitch_p;

    // Convert to radians for the setpoint
    const FVector desired_angular_velocity = FVector(
        -FMath::DegreesToRadians(desired_roll_rate_deg_s),  // Negative because of Unreal's coordinate system
        -FMath::DegreesToRadians(desired_pitch_rate_deg_s), // Negative because of Unreal's coordinate system
        FMath::DegreesToRadians(player_input.yaw * yaw_rate_deg_per_second) // Yaw is still rate-controlled
    );

    // ============================================================================
    // LEVEL 4: Vertical Velocity Control (Throttle stick -> Vertical velocity -> Throttle)
    // ============================================================================

    // Throttle input is -1 to +1, controlling vertical velocity
    const double throttle_stick = player_input.throttle;

    // Target vertical velocity (positive = up, negative = down)
    const double target_vertical_velocity = throttle_stick * max_vertical_velocity_m_s;

    // Current vertical velocity (Z component)
    const double current_vertical_velocity = flight_state.linear_velocity_world.Z;

    // Calculate vertical velocity error
    const double vertical_velocity_error = target_vertical_velocity - current_vertical_velocity;

    // Update integral with anti-windup
    if (dt > 0.0)
    {
        vertical_velocity_integral += vertical_velocity_error * dt;
        vertical_velocity_integral = FMath::Clamp(vertical_velocity_integral,
            -vertical_velocity_i_max / vertical_velocity_i,
            vertical_velocity_i_max / vertical_velocity_i);
    }

    // Calculate derivative
    double vertical_velocity_derivative = 0.0;
    if (dt > 0.0)
    {
        vertical_velocity_derivative = (vertical_velocity_error - prev_vertical_velocity_error) / dt;
    }
    prev_vertical_velocity_error = vertical_velocity_error;

    // PID output for throttle adjustment
    const double throttle_adjustment =
        (vertical_velocity_p * vertical_velocity_error) +
        (vertical_velocity_i * vertical_velocity_integral) +
        (vertical_velocity_d * vertical_velocity_derivative);

    // Final throttle = hover throttle + PID adjustment
    const double final_throttle = FMath::Clamp(throttle_adjustment, 0.0, 1.0);

    // ============================================================================
    // Output: Throttle + Angular Velocity Setpoint
    // ============================================================================

    return FDroneSetpoint(final_throttle, desired_angular_velocity);
}
