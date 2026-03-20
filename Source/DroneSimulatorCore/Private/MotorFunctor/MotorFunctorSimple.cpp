#include "DroneSimulatorCore/Public/MotorFunctor/MotorFunctorSimple.h"

MotorFunctorSimple::MotorFunctorSimple(const FDroneBattery& in_drone_battery, const FDroneMotor in_drone_motor)
	: battery(in_drone_battery), motor(in_drone_motor)
{
}

double MotorFunctorSimple::update(double previous_angular_velocity, double throttle)
{
	// motor.kv is already in rad/s/V
	// max_angular_velocity is in rad/s
	const auto max_angular_velocity = this->battery.voltage * this->motor.kv;

	return throttle * max_angular_velocity;
}
