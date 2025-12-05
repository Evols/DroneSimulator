#pragma once

#include "DroneSimulator/RotorModel/RotorModelBase.h"

#include "RotorModelBemt.generated.h"

UCLASS()
class DRONESIMULATOR_API URotorModelBemt : public URotorModelBase
{
    GENERATED_BODY()

public:

    virtual FRotorSimulationResult simulate_propeller_rotor(FSubstepBody* substep_body, double throttle,
        const FDronePropellerBemt* propeller, const FDroneMotor* motor, const FDroneBattery* battery,
        const FVector& propeller_location_local, bool is_clockwise, const USimulationWorld* simulation_world) override;
};
