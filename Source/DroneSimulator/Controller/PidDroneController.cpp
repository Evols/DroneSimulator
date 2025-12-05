#include "DroneSimulator/Controller/PidDroneController.h"
#include "DroneSimulator/Controller/Setpoint.h"


FPropellerSetThrottle UPidDroneController::tick_controller(float delta_time, const FDroneSetpoint& setpoint, const FVector& current_angular_velocity)
{
	// Unreal has yaw the opposite sign from Euler angles for yaw, and the same ones for pitch and roll

	constexpr auto angular_velocity_clamp_deg = 360.0;
	constexpr auto angular_velocity_clamp = angular_velocity_clamp_deg * (PI / 180.0);
	const auto angular_velocity_error = (setpoint.angular_velocity_radians - current_angular_velocity).BoundToCube(angular_velocity_clamp);

	const auto derivative_angular_velocity_error = (angular_velocity_error - last_angular_velocity_error) / delta_time;
	last_angular_velocity_error = FMath::VInterpTo(last_angular_velocity_error, angular_velocity_error, delta_time, 200.f);

	integrated_angular_velocity_error += angular_velocity_error * delta_time;

	const FVector proportional_pid = FVector(this->roll_pid.proportional, this->pitch_pid.proportional, this->yaw_pid.proportional);
	const FVector derivative_pid = FVector(this->roll_pid.derivative, this->pitch_pid.derivative, this->yaw_pid.derivative);
	const FVector integral_pid = FVector(this->roll_pid.integral, this->pitch_pid.integral, this->yaw_pid.integral);

	const auto delta_throttle = -(angular_velocity_error * proportional_pid + derivative_angular_velocity_error * derivative_pid + integrated_angular_velocity_error * integral_pid);

	const auto delta_throttle_pitch = delta_throttle.Y;
	const auto delta_throttle_yaw = delta_throttle.Z;
	const auto delta_throttle_roll = delta_throttle.X;
	const auto global_throttle = setpoint.throttle * this->throttle_factor;

	constexpr double delta_throttle_clamp = 0.2f; // This is an absolute delta, so with inverted signs
	double throttle_fl = global_throttle + FMath::Clamp(delta_throttle_pitch + delta_throttle_roll + delta_throttle_yaw, -delta_throttle_clamp, delta_throttle_clamp);
	double throttle_fr = global_throttle + FMath::Clamp(delta_throttle_pitch - delta_throttle_roll - delta_throttle_yaw, -delta_throttle_clamp, delta_throttle_clamp);
	double throttle_rl = global_throttle + FMath::Clamp(-delta_throttle_pitch + delta_throttle_roll - delta_throttle_yaw, -delta_throttle_clamp, delta_throttle_clamp);
	double throttle_rr = global_throttle + FMath::Clamp(-delta_throttle_pitch - delta_throttle_roll + delta_throttle_yaw, -delta_throttle_clamp, delta_throttle_clamp);

	const double throttle_min = FMath::Min(throttle_fl, throttle_fr, throttle_rl, throttle_rr);
	if (throttle_min < 0.0)
	{
		const double add = -throttle_min;
		throttle_fl += add;
		throttle_fr += add;
		throttle_rl += add;
		throttle_rr += add;
	}

	return FPropellerSetThrottle {
		throttle_fl,
		throttle_fr,
		throttle_rl,
		throttle_rr
	};
}
