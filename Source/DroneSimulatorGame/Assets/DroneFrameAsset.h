#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"

#include "DroneFrameAsset.generated.h"

UCLASS(BlueprintType)
class DRONESIMULATORGAME_API UDroneFrameAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// Distance in each axis from the center of the drone.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Front propellers extent (cm)"))
	FVector props_extent_front_cm = FVector(10.0, 10.0, 0.0);

	// Distance in each axis from the center of the drone.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Rear propellers extent (cm)"))
	FVector props_extent_back_cm = FVector(-10.0, 10.0, 0.0);

	/**
	 * Drag coefficient of the frame.
	 * Dimension-less property, often referred as "Cd".
	 * X axis is forward-backward, Y axis is right-left, Z axis is top-down.
	 *
	 * See https://en.wikipedia.org/wiki/Drag_coefficient
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Drag coefficient (Cd)"))
	FVector drag_coefficient = FVector(0.95, 0.95, 0.95);

	/**
	 * Area of the frame in m², in each axis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Frame area (m²)"))
	FVector area_m2 = FVector(0.02, 0.02, 0.05);

	/**
	 * Mass of the frame in kg.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Frame mass (kg)"))
	double mass_kg = 0.5;

};
