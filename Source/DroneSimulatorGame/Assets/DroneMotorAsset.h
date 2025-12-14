#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"

#include "DroneMotorAsset.generated.h"

UCLASS(BlueprintType)
class DRONESIMULATORGAME_API UDroneMotorAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// RPM produced by the motor per volt
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Motor KV (RPM per volt)"))
	double kv = 2000.0;

	/**
	 * Mass of each motor, in kg.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Motor mass (kg)"))
	double mass_kg = 0.03;

};
