#pragma once

#include "CoreMinimal.h"

struct FDronePropellerBemt;
struct FSubstepBody;
struct FDroneFrame;
class USimulationWorld;
class UPrimitiveComponent;

namespace simulation
{
	void DRONESIMULATORCORE_API calculate_linear_drag(FSubstepBody* substep_body, const FDroneFrame& frame, const FDronePropellerBemt& propeller, const USimulationWorld* simulation_world);
}
