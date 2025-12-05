#pragma once

#include "DroneSimulator/Controller/DroneController.h"

#include "BasicDroneController.generated.h"

struct FDroneSetpoint;

UCLASS(Blueprintable)
class UBasicDroneController : public UDroneController
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Global throttle rate"))
	double global_throttle_rate = 0.6;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Roll throttle rate"))
	double roll_throttle_rate = 0.01;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Pitch throttle rate"))
	double pitch_throttle_rate = 0.01;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Yaw throttle rate"))
	double yaw_throttle_rate = 0.08;

	virtual FPropellerSetThrottle tick_controller(float delta_time, const FDroneSetpoint& setpoint, const FVector& current_angular_velocity) override;

};
