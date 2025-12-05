#pragma once

#include "CoreMinimal.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"
#include "Runtime/SlateCore/Public/Widgets/SCompoundWidget.h"
#include "Runtime/SlateCore/Public/Styling/SlateTypes.h"
#include "Runtime/Slate/Public/Widgets/Input/SButton.h"
#include "Runtime/Slate/Public/Widgets/Input/SSlider.h"
#include "Runtime/Slate/Public/Widgets/Text/STextBlock.h"

class FFlightPlaybackManager;
class UFlightRecordAsset;

/**
 * Slate widget that provides a timeline with playback controls
 * for use in the Unreal Editor as a dockable panel
 */
class STimelinePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STimelinePanel)
		: _min_time(0.0f)
		, _max_time(10.0f)
		, _flight_record(nullptr)
		{}
		SLATE_ARGUMENT(float, min_time)
		SLATE_ARGUMENT(float, max_time)
		SLATE_ARGUMENT(UFlightRecordAsset*, flight_record)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Tick function for updating playback */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Mouse input for panning and zooming */
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	/** Reference to the playback manager */
	FFlightPlaybackManager* playback_manager;

	/** Handle flight record selection change */
	void on_flight_record_selected(const FAssetData& asset_data);

	/** Get the currently selected flight record as asset data */
	FString get_current_flight_record_path() const;

	/** Button handlers */
	FReply on_play_clicked();
	FReply on_pause_clicked();
	FReply on_step_forward_clicked();
	FReply on_step_backward_clicked();

	/** Zoom handlers */
	FReply on_zoom_in_clicked();
	FReply on_zoom_out_clicked();
	FReply on_zoom_reset_clicked();

	/** Timeline interaction handlers */
	void on_timeline_pan(float time_offset);
	void on_timeline_zoom(float wheel_delta);

	/** Timeline scrubber handlers */
	void on_timeline_value_changed(float new_value);
	float get_timeline_value() const;
	void on_timeline_value_committed(float new_value);

	/** Get formatted time string */
	FText get_current_time_text() const;
	FText get_max_time_text() const;

	/** Get button visibility */
	EVisibility get_play_button_visibility() const;
	EVisibility get_pause_button_visibility() const;

	/** Get button enabled state */
	bool is_play_enabled() const;
	bool is_pause_enabled() const;
	bool is_stop_enabled() const;

	/** Get visibility for content sections */
	EVisibility get_content_visibility() const;
	EVisibility get_no_asset_message_visibility() const;

	/** Zoom state */
	float zoom_level = 1.0f;  // 1.0 = no zoom, 2.0 = 2x zoom, etc.
	float visible_start_time = 0.0f;
	float visible_end_time = 0.0f;

	/** Pan state */
	bool is_panning = false;
	FVector2D last_mouse_position;
	float pan_anchor_start_time = 0.0f;

	/** Helper methods for zoom */
	void update_visible_time_range();
	void update_visible_time_range_with_center(float center_time);
	void pan_by_offset(float time_offset);
	float get_visible_start_time() const { return visible_start_time; }
	float get_visible_end_time() const { return visible_end_time; }
	bool is_zoom_in_enabled() const;
	bool is_zoom_out_enabled() const;
	
	/** Helper methods for event snapping */
	float snap_to_nearest_event(float time) const;
	TArray<float> get_event_times() const;

	/** Reference to the timeline slider widget */
	TSharedPtr<SSlider> timeline_slider;

	/** Reference to the timeline track widget */
	TSharedPtr<class STimelineTrack> timeline_track;
};
