#pragma once

#include "DroneSimulator/PropulsionModel/PropulsionModel.h"

#include "PropulsionModelDirectSetpoint.generated.h"


UCLASS()
class UPropulsionModelDirectSetpoint : public UPropulsionModel
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    double max_throttle_force = 50.0;

    UPROPERTY(EditAnywhere, meta=(DisplayName="Max vertical speed (m/s)"))
    double max_vertical_speed = 30.0;

    virtual TOptional<FDynamicsPropellerSetInfo> tick_propulsion(double delta_time, FSubstepBody* substep_body,
        const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
        const USimulationWorld* simulation_world);
};
