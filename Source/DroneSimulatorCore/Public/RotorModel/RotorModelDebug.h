#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/RotorModel/RotorModelBase.h"

#include "RotorModelDebug.generated.h"

UCLASS(EditInlineNew, DefaultToInstanced)
class DRONESIMULATORCORE_API URotorModelDebug : public URotorModelBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DisplayName="Thrust at max throttle (Netwons)"))
	double max_thrust;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DisplayName="Torque at max throttle (N·m)"))
	double max_torque;

public:

	URotorModelDebug();

	virtual FRotorSimulationResult simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
		const TDronePropeller* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
		const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world) override;
};
