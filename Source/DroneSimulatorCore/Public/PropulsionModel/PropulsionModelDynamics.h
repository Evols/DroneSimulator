#pragma once

#include "DroneSimulatorCore/Public/PropulsionModel/PropulsionModel.h"

#include "PropulsionModelDynamics.generated.h"

class URotorModelBase;
class UDroneController;

UCLASS()
class UPropulsionModelDynamics : public UPropulsionModel
{
    GENERATED_BODY()

public:

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Controller"))
    UDroneController* drone_controller;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Rotor model"))
    URotorModelBase* rotor_model;

    virtual TOptional<FDynamicsPropellerSetInfo> tick_propulsion(double delta_time, FSubstepBody* substep_body,
        const FDroneSetpoint& drone_setpoint, const FPropulsionDroneSetup& drone_setup,
        const USimulationWorld* simulation_world) override;
};
