#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"

struct FDronePropellerBemt;
struct FSubstepBody;
struct FDroneFrame;
class USimulationWorld;
class UPrimitiveComponent;

namespace simulation
{
	FVector calculate_props_cda(const TDronePropeller& propeller);

	void DRONESIMULATORCORE_API calculate_linear_drag(FSubstepBody* substep_body, const FDroneFrame& frame, const TDronePropeller& propeller, const USimulationWorld* simulation_world);
}
