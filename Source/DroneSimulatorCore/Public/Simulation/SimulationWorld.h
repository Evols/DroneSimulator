#pragma once

#include "CoreMinimal.h"

#include "SimulationWorld.generated.h"

UCLASS()
class DRONESIMULATORCORE_API USimulationWorld : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Gets the air density (in kg/m^3) and wind velocity (in m/s)
	 */
	TTuple<double, FVector> get_wind_and_air_density() const;

};
