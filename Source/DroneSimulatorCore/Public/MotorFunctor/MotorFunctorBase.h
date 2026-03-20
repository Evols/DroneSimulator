#pragma once

class USimulationWorld;
struct FSubstepBody;

class DRONESIMULATORCORE_API MotorFunctorBase
{
public:
	virtual ~MotorFunctorBase() = default;
	MotorFunctorBase();

	/**
	 * Outputs the new motor angular velocity (rad / s)
	 * @param previous_angular_velocity
	 * @param throttle 
	 * @return Motor angular velocity (rad / s)
	 */
	virtual double update(double previous_angular_velocity, double throttle) = 0;
};
