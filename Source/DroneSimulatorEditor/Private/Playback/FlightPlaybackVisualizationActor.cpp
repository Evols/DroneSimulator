#include "FlightPlaybackVisualizationActor.h"
#include "DroneSimulatorGame/Assets/FlightRecordAsset.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AFlightPlaybackVisualizationActor::AFlightPlaybackVisualizationActor()
{
	// Disable ticking - we'll update manually
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Create a dummy root component to keep the actor at zero position
	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = root;

	// Lock actor position and rotation to zero
	SetActorLocation(FVector::ZeroVector);
	SetActorRotation(FRotator::ZeroRotator);

	// Load the drone mesh for later use
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMeshAsset(TEXT("/Script/Engine.StaticMesh'/DroneSimulator/Meshes/SM_ReplayDrone.SM_ReplayDrone'"));
	if (DroneMeshAsset.Succeeded())
	{
		drone_mesh = DroneMeshAsset.Object;
	}

	// Make actor visible in outliner but not selectable by clicking in viewport
	bIsEditorOnlyActor = true;

#if WITH_EDITORONLY_DATA
	bListedInSceneOutliner = true;
#endif
}

void AFlightPlaybackVisualizationActor::update_playback(UFlightRecordAsset* flight_record, float current_time, bool is_playing)
{
	if (!flight_record || flight_record->events.Num() == 0)
	{
		return;
	}

	// Cache the flight record for debug drawing
	cached_flight_record = flight_record;

	// Ensure actor stays at zero position
	SetActorLocation(FVector::ZeroVector);
	SetActorRotation(FRotator::ZeroRotator);

	// Collect all unique drone names
	TSet<FName> drone_names;
	for (const FFlightRecordEvent& event : flight_record->events)
	{
		drone_names.Add(event.pawn_name);
	}

	// Update each drone
	for (const FName& drone_name : drone_names)
	{
		// Get or create component for this drone
		UStaticMeshComponent* mesh_component = get_or_create_drone_component(drone_name);
		
		if (!mesh_component)
		{
			continue;
		}

		// Get interpolated event for this drone at current time
		FFlightRecordEvent interpolated_event;
		get_interpolated_event(flight_record->events, drone_name, current_time, interpolated_event);

		// Store in map
		if (FDroneVisualizationComponent* comp = drone_components.Find(drone_name))
		{
			comp->current_event = interpolated_event;
		}

		// Update mesh component position
		mesh_component->SetWorldLocationAndRotation(
			interpolated_event.event_data.location,
			interpolated_event.event_data.rotation
		);
	}

	// Remove components for drones that are no longer in the record
	TArray<FName> to_remove;
	for (const auto& pair : drone_components)
	{
		if (!drone_names.Contains(pair.Key))
		{
			if (pair.Value.mesh_component)
			{
				pair.Value.mesh_component->DestroyComponent();
			}
			to_remove.Add(pair.Key);
		}
	}
	for (const FName& name : to_remove)
	{
		drone_components.Remove(name);
	}

	MarkComponentsRenderStateDirty();
}

void AFlightPlaybackVisualizationActor::draw_debug_visualization(UWorld* world, float current_time, bool is_playing)
{
	if (!world || !cached_flight_record.IsValid())
	{
		return;
	}

	UFlightRecordAsset* flight_record = cached_flight_record.Get();
	if (!flight_record || flight_record->events.Num() == 0)
	{
		return;
	}

	const float drone_size = 50.0f;
	const FColor drone_color = FColor::Cyan;
	const FColor active_color = is_playing ? FColor::Green : FColor::Orange;

	// Draw visualization for each drone
	for (const auto& pair : drone_components)
	{
		const FName& drone_name = pair.Key;
		const FFlightRecordEvent& event = pair.Value.current_event;
		const FVector& location = event.event_data.location;
		const FRotator& rotation = event.event_data.rotation;

		// Draw the drone as a combination of shapes
		// Draw central body (box)
		FVector box_extent(drone_size * 0.3f, drone_size * 0.3f, drone_size * 0.15f);
		DrawDebugBox(world, location, box_extent, rotation.Quaternion(), drone_color, false, -1.0f, 0, 2.0f);

		// Draw arms (4 lines extending from center)
		FVector forward = rotation.RotateVector(FVector::ForwardVector);
		FVector right = rotation.RotateVector(FVector::RightVector);
		FVector arm1_end = location + (forward + right).GetSafeNormal() * drone_size;
		FVector arm2_end = location + (forward - right).GetSafeNormal() * drone_size;
		FVector arm3_end = location + (-forward + right).GetSafeNormal() * drone_size;
		FVector arm4_end = location + (-forward - right).GetSafeNormal() * drone_size;

		DrawDebugLine(world, location, arm1_end, drone_color, false, -1.0f, 0, 3.0f);
		DrawDebugLine(world, location, arm2_end, drone_color, false, -1.0f, 0, 3.0f);
		DrawDebugLine(world, location, arm3_end, drone_color, false, -1.0f, 0, 3.0f);
		DrawDebugLine(world, location, arm4_end, drone_color, false, -1.0f, 0, 3.0f);

		// Draw propellers (circles at arm ends)
		DrawDebugSphere(world, arm1_end, drone_size * 0.2f, 8, active_color, false, -1.0f, 0, 2.0f);
		DrawDebugSphere(world, arm2_end, drone_size * 0.2f, 8, active_color, false, -1.0f, 0, 2.0f);
		DrawDebugSphere(world, arm3_end, drone_size * 0.2f, 8, active_color, false, -1.0f, 0, 2.0f);
		DrawDebugSphere(world, arm4_end, drone_size * 0.2f, 8, active_color, false, -1.0f, 0, 2.0f);

		// Draw forward direction indicator
		FVector forward_indicator = location + forward * drone_size * 1.5f;
		DrawDebugDirectionalArrow(world, location, forward_indicator, drone_size * 0.5f, FColor::Red, false, -1.0f, 0, 3.0f);

		// Draw coordinate system
		FVector up = rotation.RotateVector(FVector::UpVector);
		DrawDebugLine(world, location, location + right * drone_size * 0.5f, FColor::Green, false, -1.0f, 0, 2.0f);
		DrawDebugLine(world, location, location + up * drone_size * 0.5f, FColor::Blue, false, -1.0f, 0, 2.0f);

		// Draw velocity vector
		const FVector& velocity = event.event_data.velocity;
		// Scale velocity for visualization (convert from m/s to Unreal units: 1m = 100 units)
		const float velocity_scale = 0.15f;
		FVector velocity_visual = velocity * 100.f * velocity_scale;

		// Only draw if velocity is significant
		if (velocity_visual.SizeSquared() > 1.f)
		{
			FVector velocity_end = location + velocity_visual;

			// Draw velocity arrow in yellow
			DrawDebugDirectionalArrow(world, location, velocity_end, drone_size * 0.3f, FColor::Yellow, false, -1.0f, 0, 3.0f);
			
			// Draw velocity magnitude as text
			float velocity_magnitude = velocity.Size();
			FString velocity_text = FString::Printf(TEXT("%.2f m/s"), velocity_magnitude);
			DrawDebugString(world, location + FVector(0, 0, drone_size), velocity_text, nullptr, FColor::Yellow, -1.0f, true);
		}
	}

	// Draw flight path (trail) for each drone
	TMap<FName, TArray<const FFlightRecordEvent*>> events_by_drone;
	for (const FFlightRecordEvent& event : flight_record->events)
	{
		events_by_drone.FindOrAdd(event.pawn_name).Add(&event);
	}

	for (const auto& pair : events_by_drone)
	{
		const TArray<const FFlightRecordEvent*>& events = pair.Value;
		
		if (events.Num() > 1)
		{
			for (int32 i = 0; i < events.Num() - 1; ++i)
			{
				const FFlightRecordEvent* event1 = events[i];
				const FFlightRecordEvent* event2 = events[i + 1];
				
				// Determine color based on whether we've passed this segment
				FColor trail_color = event1->event_time <= current_time ? FColor::White : FColor(128, 128, 128, 128);
				
				DrawDebugLine(world, event1->event_data.location, event2->event_data.location, trail_color, false, -1.0f, 0, 1.0f);
			}
		}
	}
}

void AFlightPlaybackVisualizationActor::get_interpolated_event(const TArray<FFlightRecordEvent>& events, const FName& pawn_name, float time, FFlightRecordEvent& out_event) const
{
	// Filter events for this specific drone
	TArray<const FFlightRecordEvent*> drone_events;
	for (const FFlightRecordEvent& event : events)
	{
		if (event.pawn_name == pawn_name)
		{
			drone_events.Add(&event);
		}
	}

	if (drone_events.Num() == 0)
	{
		// No events for this drone
		out_event = FFlightRecordEvent();
		return;
	}

	// Find the two events to interpolate between
	int32 index_before = -1;
	int32 index_after = -1;

	for (int32 i = 0; i < drone_events.Num(); ++i)
	{
		if (drone_events[i]->event_time <= time)
		{
			index_before = i;
		}
		if (drone_events[i]->event_time >= time)
		{
			index_after = i;
			break;
		}
	}

	// Handle edge cases
	if (index_before == -1)
	{
		// Before first event
		out_event = *drone_events[0];
		return;
	}

	if (index_after == -1)
	{
		// After last event
		out_event = *drone_events.Last();
		return;
	}

	// Same event or very close
	if (index_before == index_after)
	{
		out_event = *drone_events[index_before];
		return;
	}

	// Interpolate between events
	const FFlightRecordEvent* event_before = drone_events[index_before];
	const FFlightRecordEvent* event_after = drone_events[index_after];

	float time_before = event_before->event_time;
	float time_after = event_after->event_time;
	float alpha = (time - time_before) / (time_after - time_before);
	alpha = FMath::Clamp(alpha, 0.0f, 1.0f);

	// Create interpolated event
	out_event.pawn_name = pawn_name;
	out_event.event_time = time;
	out_event.event_data.location = FMath::Lerp(event_before->event_data.location, event_after->event_data.location, alpha);
	out_event.event_data.rotation = FMath::Lerp(event_before->event_data.rotation, event_after->event_data.rotation, alpha);
	out_event.event_data.velocity = FMath::Lerp(event_before->event_data.velocity, event_after->event_data.velocity, alpha);
	out_event.event_data.angular_velocity = FMath::Lerp(event_before->event_data.angular_velocity, event_after->event_data.angular_velocity, alpha);
	
	// For discrete data, use the "before" event's values
	out_event.event_data.controller_input = event_before->event_data.controller_input;
	out_event.event_data.propulsion_info = event_before->event_data.propulsion_info;
}

UStaticMeshComponent* AFlightPlaybackVisualizationActor::get_or_create_drone_component(const FName& pawn_name)
{
	// Check if component already exists
	if (FDroneVisualizationComponent* existing = drone_components.Find(pawn_name))
	{
		return existing->mesh_component;
	}

	// Create new component
	FString component_name = FString::Printf(TEXT("DroneMesh_%s"), *pawn_name.ToString());
	UStaticMeshComponent* new_component = NewObject<UStaticMeshComponent>(this, FName(*component_name));

	if (!new_component)
	{
		return nullptr;
	}

	// Setup the component
	new_component->RegisterComponent();
	new_component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
	if (drone_mesh)
	{
		new_component->SetStaticMesh(drone_mesh);
	}
	
	// Set mesh to not collide
	new_component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	new_component->SetCastShadow(false);

	// Add to map
	FDroneVisualizationComponent& comp = drone_components.Add(pawn_name);
	comp.mesh_component = new_component;
	comp.current_event = FFlightRecordEvent();

	return new_component;
}
