#include "DroneSimulator/Controller/BasicDroneController.h"
#include "DroneSimulator/Controller/Setpoint.h"


struct FDroneSetpoint;

FPropellerSetThrottle UBasicDroneController::tick_controller(float delta_time, const FDroneSetpoint& setpoint, const FVector& current_angular_velocity)
{
	const auto global_throttle = FMath::Clamp(0.1 + 0.8 * setpoint.throttle, 0.0, 1.0);
	const auto roll_throttle = setpoint.angular_velocity_radians.X * -this->roll_throttle_rate;
	const auto pitch_throttle = setpoint.angular_velocity_radians.Y * -this->pitch_throttle_rate;
	const auto yaw_throttle = setpoint.angular_velocity_radians.Z * -this->yaw_throttle_rate;

	double throttle_fl = global_throttle + pitch_throttle + roll_throttle + yaw_throttle;
	double throttle_fr = global_throttle + pitch_throttle - roll_throttle - yaw_throttle;
	double throttle_rl = global_throttle - pitch_throttle + roll_throttle - yaw_throttle;
	double throttle_rr = global_throttle - pitch_throttle - roll_throttle + yaw_throttle;

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
		throttle_fl * this->global_throttle_rate,
		throttle_fr * this->global_throttle_rate,
		throttle_rl * this->global_throttle_rate,
		throttle_rr * this->global_throttle_rate
	};
}
