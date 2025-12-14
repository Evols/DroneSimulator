#include "DroneSimulatorCore/Public/Controller/PidDroneController.h"
#include "DroneSimulatorCore/Public/Controller/Setpoint.h"


FPropellerSetThrottle UPidDroneController::tick_controller(float delta_time, const FDroneSetpoint& setpoint, const FVector& current_angular_velocity)
{
	// Unreal has yaw the opposite sign from Euler angles for yaw, and the same ones for pitch and roll

	constexpr auto angular_velocity_clamp_deg = 360.0;
	constexpr auto angular_velocity_clamp = angular_velocity_clamp_deg * (PI / 180.0);
	const auto angular_velocity_error = (setpoint.angular_velocity_radians - current_angular_velocity).BoundToCube(angular_velocity_clamp);

	const auto derivative_angular_velocity_error = (angular_velocity_error - last_angular_velocity_error) / delta_time;
	last_angular_velocity_error = FMath::VInterpTo(last_angular_velocity_error, angular_velocity_error, delta_time, 500.f);

	integrated_angular_velocity_error += angular_velocity_error * delta_time;

	const FVector proportional_pid = FVector(this->roll_pid.proportional, this->pitch_pid.proportional, this->yaw_pid.proportional);
	const FVector derivative_pid = FVector(this->roll_pid.derivative, this->pitch_pid.derivative, this->yaw_pid.derivative);
	const FVector integral_pid = FVector(this->roll_pid.integral, this->pitch_pid.integral, this->yaw_pid.integral);

	const auto delta_throttle_angular = -(angular_velocity_error * proportional_pid + derivative_angular_velocity_error * derivative_pid + integrated_angular_velocity_error * integral_pid);

	const auto delta_throttle_pitch = delta_throttle_angular.Y;
	const auto delta_throttle_yaw = delta_throttle_angular.Z;
	const auto delta_throttle_roll = delta_throttle_angular.X;

	// The min throttle is applied as a clamp of the input, instead of 
	const auto global_throttle = FMath::Clamp(setpoint.throttle, this->min_throttle, 1.0);

	// Throttle values are absolute, in the 0..1 range

	const auto delta_throttle = FPropellerSetThrottle(
		delta_throttle_pitch + delta_throttle_roll + delta_throttle_yaw,
		delta_throttle_pitch - delta_throttle_roll - delta_throttle_yaw,
		-delta_throttle_pitch + delta_throttle_roll - delta_throttle_yaw,
		-delta_throttle_pitch - delta_throttle_roll + delta_throttle_yaw);

	// The ideal throttle is the throttle we would want to apply if negative throttle was possible, and props could spin
	// at very low speeds without consequences
	const auto ideal_throttle = FPropellerSetThrottle::from_real(global_throttle) + delta_throttle;

	// If any axis of the throttle is below the min dynamic throttle, then we try to "fix" it 
	const auto ideal_throttle_min_axis = FMath::Min(ideal_throttle.front_left, ideal_throttle.front_right, ideal_throttle.rear_left, ideal_throttle.rear_right);
	if (ideal_throttle_min_axis < this->min_dynamic_throttle)
	{
		constexpr auto max_throttle_boost = 0.2f;
		const auto desired_throttle_boost = this->min_dynamic_throttle - ideal_throttle_min_axis;

		// If the necessary throttle boost is smaller than the max allowed boost, apply it directly
		if (desired_throttle_boost <= max_throttle_boost)
		{
			const auto throttle_boost = desired_throttle_boost;
			return FPropellerSetThrottle::from_real(global_throttle + throttle_boost) + delta_throttle;
		}

		// If the necessary throttle boost is greater than the max-allowed boost, scale it down to the max allowed,
		// and scale down the delta as well
		constexpr auto throttle_boost = max_throttle_boost;
		const auto delta_throttle_scale = max_throttle_boost / desired_throttle_boost;
		return FPropellerSetThrottle::from_real(global_throttle + throttle_boost) + delta_throttle * delta_throttle_scale;
	}

	// If any axis of the throttle is above the max dynamic throttle, then we try to fix it
	const auto ideal_throttle_max_axis = FMath::Max(ideal_throttle.front_left, ideal_throttle.front_right, ideal_throttle.rear_left, ideal_throttle.rear_right);
	if (ideal_throttle_max_axis > 1.0)
	{
		constexpr auto max_throttle_shrink = 0.1f;
		const auto desired_throttle_shrink = ideal_throttle_max_axis - 1.0;

		// If the necessary throttle shrink is smaller than the max allowed shrink, apply it directly
		if (desired_throttle_shrink < max_throttle_shrink)
		{
			const auto throttle_shink = desired_throttle_shrink;
			return FPropellerSetThrottle::from_real(global_throttle - throttle_shink) + delta_throttle;
		}

		constexpr auto throttle_shink = max_throttle_shrink;
		const auto delta_throttle_scale = max_throttle_shrink / desired_throttle_shrink;
		return FPropellerSetThrottle::from_real(global_throttle - throttle_shink) + delta_throttle * delta_throttle_scale;
	}

	// If the ideal throttle is in the range, no correction to make, it can be returned as-is
	return ideal_throttle;
}
