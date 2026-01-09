#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"

namespace inertia
{
	DRONESIMULATORCORE_API FVector compute_frame_inertia(const TOptional<FDroneFrame>& frame_opt);
	DRONESIMULATORCORE_API FVector compute_motors_inertia(const TOptional<FDroneMotor>& motor_opt, const TOptional<FDroneFrame>& frame_opt);
	DRONESIMULATORCORE_API FVector compute_battery_inertia(const TOptional<FDroneBattery>& battery_opt);

	DRONESIMULATORCORE_API FVector compute_inertia_si(const TOptional<FDroneFrame>& frame_opt,
		const TOptional<FDroneMotor>& motor_opt, const TOptional<FDroneBattery>& battery_opt);

	// Convenience: UE units (kg·cm²)
	DRONESIMULATORCORE_API FVector compute_inertia_uu(const TOptional<FDroneFrame>& frame_opt,
		const TOptional<FDroneMotor>& motor_opt, const TOptional<FDroneBattery>& battery_opt);
}
