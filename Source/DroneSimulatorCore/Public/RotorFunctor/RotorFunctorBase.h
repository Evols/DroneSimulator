#pragma once

class USimulationWorld;
struct FSubstepBody;

class DRONESIMULATORCORE_API RotorFunctorBase
{
public:
	virtual ~RotorFunctorBase() = default;
	RotorFunctorBase();

	virtual void update(FSubstepBody* substep_body, double angular_velocity, const USimulationWorld* simulation_world) = 0;
};
