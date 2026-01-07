#pragma once

#include "DroneSimulatorGame/Gameplay/DroneMovementComponent.h"

#include "FlightRecord.generated.h"

/**
 * Single frame for a single pawn
 */
USTRUCT(BlueprintType)
struct FFlightRecordEventData
{
	GENERATED_BODY()

public:

	// In Unreal units
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FVector location = FVector::ZeroVector;

	// In unreal units
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FRotator rotation = FRotator::ZeroRotator;

	// In m/s
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FVector velocity = FVector::ZeroVector;

	// In rad/s
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FVector angular_velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FDronePlayerInput controller_input;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TOptional<FPropulsionInfo> propulsion_info;

	FFlightRecordEventData() = default;

	FFlightRecordEventData(const FVector& in_location, const FRotator& in_rotation, const FVector& in_velocity,
		const FVector& in_angular_velocity, const FDronePlayerInput& in_controller_input,
		const TOptional<FPropulsionInfo>& in_propulsion_info)
		: location(in_location), rotation(in_rotation), velocity(in_velocity), angular_velocity(in_angular_velocity),
		  controller_input(in_controller_input), propulsion_info(in_propulsion_info)
	{
	}
};
