#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"

namespace inertia
{
	double uu_to_meters(double uu);
	FVector kgm2_to_kgcm2(const FVector& inertia_si);
	FVector compute_box_inertia(double mass, const FVector& extent);
	FVector compute_point_mass_inertia(double mass, const FVector& offset);
	FVector compute_rotated_box_inertia(double mass, const FVector& extent, const FRotator& rotation);

	// Primitive part-specific inertia functions
	FVector compute_frame_inertia_primitive(double mass_frame, const FVector& front_extent, const FVector& back_extent);
	FVector compute_motors_inertia_primitive(double mass_motors, const FVector& front_extent, const FVector& back_extent);
	FVector compute_battery_inertia_primitive(double mass_battery);
}
