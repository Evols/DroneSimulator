#pragma once

#include "CoreMinimal.h"
#include "Editor/UnrealEd/Public/TickableEditorObject.h"

struct FFlightRecordEventData;
class UFlightRecordAsset;
struct FFlightRecordEvent;

/**
 * Global manager for flight record playback in the editor
 * Handles playback state, timing, and visualization
 */
class FFlightPlaybackManager : public FTickableEditorObject
{
public:
	/** Playback states */
	enum class EPlaybackState : uint8
	{
		Stopped,
		Playing,
		Paused
	};

	/** Get singleton instance */
	static FFlightPlaybackManager& get();

	/** FTickableEditorObject interface */
	virtual void Tick(float delta_time) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }

	/** Load a flight record for playback */
	void load_flight_record(UFlightRecordAsset* flight_record);

	/** Get the currently loaded flight record */
	UFlightRecordAsset* get_flight_record() const { return current_flight_record.Get(); }

	/** Playback control */
	void play();
	void pause();
	void stop();
	void set_playback_speed(float speed);
	void seek_to_time(float time);
	void step_to_next_event();
	void step_to_previous_event();

	/** Get playback state */
	EPlaybackState get_playback_state() const { return playback_state; }
	float get_current_time() const { return current_time; }
	float get_min_time() const { return min_time; }
	float get_max_time() const { return max_time; }
	float get_playback_speed() const { return playback_speed; }

	/** Check if a flight record is loaded */
	bool has_flight_record() const { return current_flight_record.IsValid(); }

	/** Set whether PIE is active (called by editor module) */
	void set_pie_active(bool is_active) { is_pie_active = is_active; }

	/** Check if PIE is currently active */
	bool is_pie_running() const { return is_pie_active; }

	/** Get current event data for a specific drone (or first drone if none specified) */
	const FFlightRecordEventData* get_current_event_data(const FName& drone_name = NAME_None) const;

private:
	/** Private constructor for singleton */
	FFlightPlaybackManager();
	~FFlightPlaybackManager();

	/** Update time bounds from current flight record */
	void update_time_bounds();

	/** Singleton instance */
	static FFlightPlaybackManager* instance;

	/** Current flight record */
	TWeakObjectPtr<UFlightRecordAsset> current_flight_record;

	/** Playback state */
	EPlaybackState playback_state;

	/** Current playback time */
	float current_time;

	/** Time bounds */
	float min_time;
	float max_time;

	/** Playback speed multiplier */
	float playback_speed;

	/** Whether PIE is currently active */
	bool is_pie_active;

	/** Visualization actor for displaying flight data in Details panel */
	TWeakObjectPtr<class AFlightPlaybackVisualizationActor> visualization_actor;

	/** Spawn the visualization actor */
	void spawn_visualization_actor();

	/** Update the visualization actor with current data */
	void update_visualization_actor();

	/** Destroy the visualization actor */
	void destroy_visualization_actor();

	/** Handle world cleanup to destroy actor */
	void on_world_cleanup(UWorld* world, bool session_ended, bool cleanup_resources);

	/** Handle pre-world destroy */
	void on_pre_world_destroy(UWorld* world);
};
