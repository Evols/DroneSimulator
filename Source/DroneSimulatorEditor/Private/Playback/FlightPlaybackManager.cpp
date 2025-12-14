#include "DroneSimulatorEditor/Private/Playback/FlightPlaybackManager.h"
#include "DroneSimulatorEditor/Private/Playback/FlightPlaybackVisualizationActor.h"
#include "DroneSimulatorGame/Assets/FlightRecordAsset.h"

#include "Editor.h"
#include "EditorViewportClient.h"
#include "UnrealClient.h"
#include "Engine/World.h"

FFlightPlaybackManager* FFlightPlaybackManager::instance = nullptr;

FFlightPlaybackManager::FFlightPlaybackManager()
	: playback_state(EPlaybackState::Stopped)
	, current_time(0.0f)
	, min_time(0.0f)
	, max_time(10.0f)
	, playback_speed(1.0f)
	, is_pie_active(false)
{
	// Register for world cleanup to destroy actor before worlds are torn down
	FWorldDelegates::OnWorldCleanup.AddRaw(this, &FFlightPlaybackManager::on_world_cleanup);
	FWorldDelegates::OnPreWorldFinishDestroy.AddRaw(this, &FFlightPlaybackManager::on_pre_world_destroy);
}

FFlightPlaybackManager::~FFlightPlaybackManager()
{
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	FWorldDelegates::OnPreWorldFinishDestroy.RemoveAll(this);
	destroy_visualization_actor();
}

FFlightPlaybackManager& FFlightPlaybackManager::get()
{
	if (!instance)
	{
		instance = new FFlightPlaybackManager();
	}
	return *instance;
}

void FFlightPlaybackManager::Tick(float delta_time)
{
	// Don't update during PIE
	if (is_pie_active)
	{
		// Deselect and destroy visualization actor during PIE
		if (GEditor && visualization_actor.IsValid())
		{
			AActor* actor_ptr = visualization_actor.Get();
			if (actor_ptr && IsValid(actor_ptr))
			{
				GEditor->SelectActor(actor_ptr, false, true);
			}
		}
		destroy_visualization_actor();
		return;
	}

	// Clean up invalid actor references
	if (visualization_actor.IsValid())
	{
		AActor* actor_ptr = visualization_actor.Get();
		if (!actor_ptr || !IsValid(actor_ptr) || actor_ptr->IsPendingKillPending())
		{
			visualization_actor.Reset();
		}
	}

	// Update playback time
	if (playback_state == EPlaybackState::Playing && current_flight_record.IsValid())
	{
		current_time += delta_time * playback_speed;

		// Stop at the end
		if (current_time >= max_time)
		{
			current_time = max_time;
			playback_state = EPlaybackState::Stopped;
			destroy_visualization_actor();
		}
		else if (current_time < min_time)
		{
			current_time = min_time;
		}
	}

	// Update visualization if we have a flight record
	if (current_flight_record.IsValid())
	{
		// Update the visualization actor
		this->update_visualization_actor();
	}
}

TStatId FFlightPlaybackManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FDroneFlightPlaybackManager, STATGROUP_Tickables);
}

void FFlightPlaybackManager::load_flight_record(UFlightRecordAsset* flight_record)
{
	current_flight_record = flight_record;
	
	// Reset playback state and destroy visualization actor
	playback_state = EPlaybackState::Stopped;
	current_time = 0.0f;
	destroy_visualization_actor();
	
	// Update time bounds
	update_time_bounds();
	
	// Seek to start
	if (current_flight_record.IsValid())
	{
		current_time = min_time;
	}

	// Spawn and select the visualization actor
	this->spawn_visualization_actor();
}

void FFlightPlaybackManager::play()
{
	// Don't allow playback during PIE
	if (is_pie_active)
	{
		return;
	}

	if (!current_flight_record.IsValid())
	{
		return;
	}

	// If at the end, restart from beginning
	if (playback_state == EPlaybackState::Stopped && current_time >= max_time)
	{
		current_time = min_time;
	}

	playback_state = EPlaybackState::Playing;
}

void FFlightPlaybackManager::pause()
{
	playback_state = EPlaybackState::Paused;
}

void FFlightPlaybackManager::stop()
{
	playback_state = EPlaybackState::Stopped;
	current_time = min_time;
	
	// Deselect before destroying
	if (GEditor && visualization_actor.IsValid())
	{
		AActor* actor_ptr = visualization_actor.Get();
		if (actor_ptr && IsValid(actor_ptr))
		{
			GEditor->SelectActor(actor_ptr, false, true);
		}
	}

	destroy_visualization_actor();
}

void FFlightPlaybackManager::set_playback_speed(float speed)
{
	playback_speed = FMath::Clamp(speed, 0.1f, 10.0f);
}

void FFlightPlaybackManager::seek_to_time(float time)
{
	current_time = FMath::Clamp(time, min_time, max_time);
	
	// Immediately update visualization when seeking
	update_visualization_actor();
}

void FFlightPlaybackManager::step_to_next_event()
{
	if (!current_flight_record.IsValid() || current_flight_record->events.Num() == 0)
	{
		return;
	}

	// Find the next event after current time
	const TArray<FFlightRecordEvent>& events = current_flight_record->events;
	
	for (int32 i = 0; i < events.Num(); ++i)
	{
		if (events[i].event_time > current_time + KINDA_SMALL_NUMBER)
		{
			current_time = events[i].event_time;
			return;
		}
	}

	// If no next event found, go to the last event
	if (events.Num() > 0)
	{
		current_time = events.Last().event_time;
	}
}

void FFlightPlaybackManager::step_to_previous_event()
{
	if (!current_flight_record.IsValid() || current_flight_record->events.Num() == 0)
	{
		return;
	}

	// Find the previous event before current time
	const TArray<FFlightRecordEvent>& events = current_flight_record->events;
	
	for (int32 i = events.Num() - 1; i >= 0; --i)
	{
		if (events[i].event_time < current_time - KINDA_SMALL_NUMBER)
		{
			current_time = events[i].event_time;
			return;
		}
	}

	// If no previous event found, go to the first event
	if (events.Num() > 0)
	{
		current_time = events[0].event_time;
	}
}

void FFlightPlaybackManager::update_time_bounds()
{
	if (!current_flight_record.IsValid() || current_flight_record->events.Num() == 0)
	{
		min_time = 0.0f;
		max_time = 10.0f;
		return;
	}

	// Find min and max times from events
	min_time = current_flight_record->events[0].event_time;
	max_time = current_flight_record->events.Last().event_time;

	for (const FFlightRecordEvent& event : current_flight_record->events)
	{
		min_time = FMath::Min(min_time, static_cast<float>(event.event_time));
		max_time = FMath::Max(max_time, static_cast<float>(event.event_time));
	}

	// Ensure we have at least some time range
	if (FMath::IsNearlyEqual(min_time, max_time))
	{
		max_time = min_time + 1.0f;
	}
}

const FFlightRecordEventData* FFlightPlaybackManager::get_current_event_data(const FName& drone_name) const
{
	if (!current_flight_record.IsValid() || current_flight_record->events.Num() == 0)
	{
		return nullptr;
	}

	const TArray<FFlightRecordEvent>& events = current_flight_record->events;

	// Determine which drone to get data for
	FName target_drone = drone_name;
	if (target_drone == NAME_None)
	{
		// Find the first drone in the events
		if (events.Num() > 0)
		{
			target_drone = events[0].pawn_name;
		}
		else
		{
			return nullptr;
		}
	}

	// Filter events for the target drone and find the closest one to current_time
	int32 closest_index = -1;
	float closest_time_diff = FLT_MAX;

	for (int32 i = 0; i < events.Num(); ++i)
	{
		if (events[i].pawn_name == target_drone)
		{
			float time_diff = FMath::Abs(events[i].event_time - current_time);
			if (time_diff < closest_time_diff)
			{
				closest_time_diff = time_diff;
				closest_index = i;
			}
		}
	}

	if (closest_index == -1)
	{
		return nullptr;
	}

	return &events[closest_index].event_data;
}

void FFlightPlaybackManager::spawn_visualization_actor()
{
	// Don't spawn during PIE
	if (is_pie_active)
	{
		return;
	}

	if (!GEditor)
	{
		return;
	}

	// Get the current editor world
	UWorld* editor_world = GEditor->GetEditorWorldContext().World();
	if (!editor_world || editor_world->WorldType != EWorldType::Editor)
	{
		return;
	}

	// Destroy old actor if it exists
	destroy_visualization_actor();

	// Spawn new visualization actor
	FActorSpawnParameters spawn_params;
	spawn_params.Name = FName(TEXT("FlightPlaybackVisualization"));
	spawn_params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	spawn_params.bNoFail = false;
	spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawn_params.ObjectFlags = RF_Transient; // Don't save this actor
	
	visualization_actor = editor_world->SpawnActor<AFlightPlaybackVisualizationActor>(
		AFlightPlaybackVisualizationActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		spawn_params
	);

	if (visualization_actor.IsValid())
	{
		visualization_actor->SetActorLabel(TEXT("Flight Playback Data"));
		visualization_actor->SetFlags(RF_Transient); // Mark as transient
		visualization_actor->bIsEditorOnlyActor = true; // Editor only
	}
}

void FFlightPlaybackManager::update_visualization_actor()
{
	// Don't update during PIE
	if (is_pie_active)
	{
		return;
	}

	// Multiple validity checks to prevent crashes
	if (!visualization_actor.IsValid())
	{
		return;
	}

	AActor* actor_ptr = visualization_actor.Get();
	if (!actor_ptr || !IsValid(actor_ptr) || actor_ptr->IsPendingKillPending())
	{
		visualization_actor.Reset();
		return;
	}

	// Check if the actor's world is valid
	UWorld* actor_world = actor_ptr->GetWorld();
	if (!actor_world || actor_world->WorldType != EWorldType::Editor)
	{
		visualization_actor.Reset();
		return;
	}

	// Update the actor with the current flight record and time
	if (current_flight_record.IsValid())
	{
		bool is_playing = playback_state == EPlaybackState::Playing;
		visualization_actor->update_playback(current_flight_record.Get(), current_time, is_playing);
		
		// Draw debug visualization
		visualization_actor->draw_debug_visualization(actor_world, current_time, is_playing);
	}
}

void FFlightPlaybackManager::destroy_visualization_actor()
{
	if (visualization_actor.IsValid())
	{
		AActor* actor_ptr = visualization_actor.Get();
		if (actor_ptr)
		{
			// CRITICAL: Deselect the actor first to prevent crashes during selection cleanup
			if (GEditor)
			{
				GEditor->SelectActor(actor_ptr, false, true);
			}

			// Get world before any operations
			UWorld* actor_world = actor_ptr->GetWorld();
			
			// Only try to destroy if the world is still valid
			if (actor_world && !actor_world->bIsTearingDown && IsValid(actor_ptr) && !actor_ptr->IsPendingKillPending())
			{
				actor_ptr->Destroy();
			}
		}
	}
	// Always reset the pointer
	visualization_actor.Reset();
}

void FFlightPlaybackManager::on_world_cleanup(UWorld* world, bool session_ended, bool cleanup_resources)
{
	// Destroy visualization actor when any world is being cleaned up
	if (!world)
	{
		return;
	}

	if (visualization_actor.IsValid())
	{
		AActor* actor_ptr = visualization_actor.Get();
		if (actor_ptr && IsValid(actor_ptr))
		{
			UWorld* actor_world = actor_ptr->GetWorld();
			if (actor_world == world)
			{
				// Deselect first to prevent selection crashes
				if (GEditor)
				{
					GEditor->SelectActor(actor_ptr, false, true);
				}
				
				// This world is being cleaned up, destroy our actor now
				destroy_visualization_actor();
			}
		}
		else
		{
			// Actor is invalid, clear the reference
			visualization_actor.Reset();
		}
	}
}

void FFlightPlaybackManager::on_pre_world_destroy(UWorld* world)
{
	// Destroy visualization actor before any world is destroyed
	if (!world)
	{
		return;
	}

	// If the world being destroyed is the editor world, just clear our reference
	// Don't try to access the actor at all, it's too late
	if (world->WorldType == EWorldType::Editor)
	{
		visualization_actor.Reset();
	}
}
