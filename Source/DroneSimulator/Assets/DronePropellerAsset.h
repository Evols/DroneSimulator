#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"

#include "DronePropellerAsset.generated.h"

class UDroneAirfoilAssetBase;

UCLASS(BlueprintType)
class DRONESIMULATOR_API UDronePropellerAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
};

UCLASS(BlueprintType)
class DRONESIMULATOR_API UDronePropellerBemtAsset : public UDronePropellerAsset
{
	GENERATED_BODY()

public:

	/** Number of blades */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(ClampMin="2", ClampMax="6", DisplayName="Number of blades"))
	int32 num_blades = 3;

	/** Tip radius, in inches (blade length from hub center). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(ClampMin="1.2", ClampMax="12", DisplayName="Radius (in inches)"))
	double diameter_inch = 5.0; // 5 inch

	/** Hub (root) radius, in cm – no lift inside this radius. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(ClampMin="0.0", DisplayName="Hub radius (in cm)"))
	double hub_radius_cm = 1.5;

	/** Chord, in cm, assumed constant along span (keep it simple). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(ClampMin="0.005", DisplayName="Chord (in cm)"))
	double chord_cm = 2.0;

	/** Linear twist, in inches. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(ClampMin="0.0", ClampMax="10.0", DisplayName="Pitch (in inches)"))
	double pitch_inch = 3.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Airfoil"))
	UDroneAirfoilAssetBase* airfoil = nullptr;
};

UCLASS(BlueprintType)
class DRONESIMULATOR_API UDronePropellerSimplifiedAsset : public UDronePropellerAsset
{
	GENERATED_BODY()

public:

};
