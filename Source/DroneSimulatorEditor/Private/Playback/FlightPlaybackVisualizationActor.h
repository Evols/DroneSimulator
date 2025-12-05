#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "DroneSimulator/Assets/FlightRecordAsset.h"

#include "FlightPlaybackVisualizationActor.generated.h"

class UFlightRecordAsset;

/**
 * Per-drone component data
 */
USTRUCT()
struct FDroneVisualizationComponent
{
	GENERATED_BODY()

	/** Mesh component for this drone */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> mesh_component;

	/** Current interpolated event data for this drone */
	FFlightRecordEvent current_event;

	FDroneVisualizationComponent()
		: mesh_component(nullptr)
		, current_event()
	{
	}
};

/**
 * Temporary actor spawned during flight playback to display properties in the Details panel
 * and visualize drone flight paths. Actor position is always zero.
 */
UCLASS(NotBlueprintable, NotPlaceable)
class AFlightPlaybackVisualizationActor : public AActor
{
	GENERATED_BODY()

public:
	AFlightPlaybackVisualizationActor();

	/** Update the actor with current flight record and playback time */
	void update_playback(class UFlightRecordAsset* flight_record, float current_time, bool is_playing);

	/** Draw debug visualization for all drones */
	void draw_debug_visualization(UWorld* world, float current_time, bool is_playing);

private:
	/** Get interpolated event data at a specific time for a specific drone */
	void get_interpolated_event(const TArray<FFlightRecordEvent>& events, const FName& pawn_name, float time, FFlightRecordEvent& out_event) const;

	/** Ensure we have a component for the given drone */
	UStaticMeshComponent* get_or_create_drone_component(const FName& pawn_name);

	/** Map of drone name to visualization component */
	UPROPERTY()
	TMap<FName, FDroneVisualizationComponent> drone_components;

	/** Cached flight record for drawing paths */
	UPROPERTY()
	TWeakObjectPtr<UFlightRecordAsset> cached_flight_record;

	/** Static mesh reference for drone */
	UPROPERTY()
	TObjectPtr<UStaticMesh> drone_mesh;
};
