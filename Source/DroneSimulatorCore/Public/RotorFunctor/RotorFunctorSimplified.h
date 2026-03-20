#pragma once

#include "DroneSimulatorCore/Public/RotorFunctor/RotorFunctorBase.h"

class DRONESIMULATORCORE_API RotorFunctorSimplified : public RotorFunctorBase
{
private:

	FVector propeller_location_local;

	bool is_clockwise;

public:

	virtual ~RotorFunctorSimplified() = default;

	RotorFunctorSimplified(const FVector& in_propeller_location_local, bool in_is_clockwise) : 
		propeller_location_local(in_propeller_location_local), is_clockwise(in_is_clockwise)
	{
	}

	virtual void update(FSubstepBody* substep_body, double angular_velocity,
		const USimulationWorld* simulation_world) override;
};
