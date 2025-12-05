#pragma once

#include "CoreMinimal.h"

struct FSubstepBody;
struct FDroneFrame;
class USimulationWorld;
class UPrimitiveComponent;

namespace simulation
{
	void calculate_rotational_drag(FSubstepBody* substep_body, const FDroneFrame& frame, const USimulationWorld* simulation_world);
}
