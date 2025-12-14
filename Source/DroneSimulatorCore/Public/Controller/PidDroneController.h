#pragma once

#include "DroneSimulatorCore/Public/Controller/DroneController.h"

#include "PidDroneController.generated.h"

struct FDronePlayerInput;

USTRUCT(BlueprintType)
struct FPidConfig
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	double proportional = 0.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	double integral = 0.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	double derivative = 0.0;
};

UCLASS()
class UPidDroneController : public UDroneController
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates")
	double min_throttle = 0.05;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates")
	double min_dynamic_throttle = 0.02;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID", meta=(DisplayName="Pitch PID"))
	FPidConfig pitch_pid = FPidConfig { 0.1, 0.0, 0.0 };

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID", meta=(DisplayName="Yaw PID"))
	FPidConfig yaw_pid = FPidConfig { 0.3, 0.0, 0.0	 };

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PID", meta=(DisplayName="Roll PID"))
	FPidConfig roll_pid = FPidConfig { 0.1, 0.0, 0.0 };

protected:

	UPROPERTY()
	FVector last_angular_velocity_error;

	UPROPERTY()
	FVector integrated_angular_velocity_error;

public:

	virtual FPropellerSetThrottle tick_controller(float delta_time, const FDroneSetpoint& setpoint, const FVector& current_angular_velocity) override;

};
