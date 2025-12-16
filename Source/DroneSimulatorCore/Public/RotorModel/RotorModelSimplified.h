#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/RotorModel/RotorModelBase.h"

#include "RotorModelSimplified.generated.h"

UCLASS(EditInlineNew, DefaultToInstanced)
class DRONESIMULATORCORE_API URotorModelSimplified : public URotorModelBase
{
	GENERATED_BODY()

public:

	virtual FRotorSimulationResult simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
		const TDronePropeller* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
		const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world) override;
};
