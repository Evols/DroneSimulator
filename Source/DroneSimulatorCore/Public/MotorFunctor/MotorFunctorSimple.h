#pragma once

#include "DroneSimulatorCore/Public/MotorFunctor/MotorFunctorBase.h"
#include "Simulation/Structural.h"

class MotorFunctorSimple : public MotorFunctorBase
{
private:
	const FDroneBattery battery;
	const FDroneMotor motor;
public:
	virtual ~MotorFunctorSimple() override = default;

	MotorFunctorSimple(const FDroneBattery& drone_battery, const FDroneMotor drone_motor);
	
	virtual double update(double previous_angular_velocity, double throttle) override;
};
