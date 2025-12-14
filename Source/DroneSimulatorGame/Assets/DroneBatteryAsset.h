#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"

#include "DroneBatteryAsset.generated.h"

UCLASS(BlueprintType)
class DRONESIMULATORGAME_API UDroneBatteryAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(ClampMin="0.0", ClampMax="50.0", DisplayName="Voltage"))
	double voltage = 20.0;

	/**
	 * Mass of the battery in kg.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Battery mass (kg)"))
	double mass_kg = 0.3;

};
