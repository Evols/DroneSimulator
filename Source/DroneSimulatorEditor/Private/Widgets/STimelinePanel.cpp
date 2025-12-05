#include "DroneSimulatorEditor/Private/Widgets/STimelinePanel.h"
#include "DroneSimulatorEditor/Private/Widgets/SFlightDataWidget.h"
#include "DroneSimulatorEditor/Private/Widgets/SPropellerDisplayWidget.h"
#include "DroneSimulatorEditor/Private/Widgets/STimelineTrack.h"
#include "DroneSimulator/Assets/FlightRecordAsset.h"
#include "DroneSimulatorEditor/Private/Playback/FlightPlaybackManager.h"

#include "Runtime/Slate/Public/Widgets/Layout/SBox.h"
#include "Runtime/Slate/Public/Widgets/Layout/SBorder.h"
#include "Runtime/Slate/Public/Widgets/Layout/SSplitter.h"
#include "Runtime/Slate/Public/Widgets/Layout/SScrollBox.h"
#include "Runtime/Slate/Public/Widgets/Layout/SSeparator.h"
#include "Runtime/SlateCore/Public/Widgets/Images/SImage.h"
#include "Runtime/Slate/Public/Widgets/Input/SButton.h"
#include "Runtime/Slate/Public/Widgets/Input/SSlider.h"
#include "Runtime/Slate/Public/Widgets/Text/STextBlock.h"
#include "Runtime/SlateCore/Public/Styling/AppStyle.h"
#include "PropertyCustomizationHelpers.h"
#include "AssetRegistry/AssetData.h"

#define LOCTEXT_NAMESPACE "TimelinePanel"

void STimelinePanel::Construct(const FArguments& InArgs)
{
	// Get reference to the global playback manager
	playback_manager = &FFlightPlaybackManager::get();
	
	// If a flight record is provided, load it into the manager
	if (InArgs._flight_record)
	{
		playback_manager->load_flight_record(InArgs._flight_record);
	}

	// Initialize zoom state
	zoom_level = 1.0f;
	update_visible_time_range();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(4.0f))
		[
			SNew(SVerticalBox)

			// Flight Record Selector
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(SHorizontalBox)

				// Label
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FlightRecordLabel", "Flight Record:"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]

				// Asset picker
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SObjectPropertyEntryBox)
					.ObjectPath(this, &STimelinePanel::get_current_flight_record_path)
					.AllowedClass(UFlightRecordAsset::StaticClass())
					.OnObjectChanged(this, &STimelinePanel::on_flight_record_selected)
					.AllowClear(true)
					.DisplayUseSelected(true)
					.DisplayBrowse(true)
					.DisplayThumbnail(false)
				]
			]

			// Message when no asset is selected
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(20.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(40.0f)
				.Visibility(this, &STimelinePanel::get_no_asset_message_visibility)
				[
					SNew(SVerticalBox)
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NoAssetTitle", "No Flight Record Selected"))
						.Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
						.Justification(ETextJustify::Center)
					]
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NoAssetMessage", "Please select a Flight Record asset from the dropdown above to view playback controls and flight data."))
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
					]
				]
			]

			// Playback controls and timeline section
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(SBox)
				.Visibility(this, &STimelinePanel::get_content_visibility)
				[
					SNew(SHorizontalBox)

					// Transport controls (left side)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4.0f, 2.0f)
					[
						SNew(SBox)
						.WidthOverride(40.0f)
						.HeightOverride(32.0f)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "Button")
							.OnClicked(this, &STimelinePanel::on_step_backward_clicked)
							.IsEnabled(this, &STimelinePanel::is_stop_enabled)
							.ToolTipText(LOCTEXT("StepBackward", "Step Backward"))
							.ContentPadding(0.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(FAppStyle::GetBrush("Animation.Backward_Step"))
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4.0f, 2.0f)
					[
						SNew(SBox)
						.WidthOverride(40.0f)
						.HeightOverride(32.0f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "Button")
								.OnClicked(this, &STimelinePanel::on_play_clicked)
								.IsEnabled(this, &STimelinePanel::is_play_enabled)
								.Visibility(this, &STimelinePanel::get_play_button_visibility)
								.ToolTipText(LOCTEXT("Play", "Play"))
								.ContentPadding(0.0f)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SImage)
									.Image(FAppStyle::GetBrush("Animation.Forward"))
									.ColorAndOpacity(FSlateColor::UseForeground())
								]
							]
							+ SOverlay::Slot()
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "Button")
								.OnClicked(this, &STimelinePanel::on_pause_clicked)
								.IsEnabled(this, &STimelinePanel::is_pause_enabled)
								.Visibility(this, &STimelinePanel::get_pause_button_visibility)
								.ToolTipText(LOCTEXT("Pause", "Pause"))
								.ContentPadding(0.0f)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(SImage)
									.Image(FAppStyle::GetBrush("Animation.Pause"))
									.ColorAndOpacity(FSlateColor::UseForeground())
								]
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4.0f, 2.0f)
					[
						SNew(SBox)
						.WidthOverride(40.0f)
						.HeightOverride(32.0f)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "Button")
							.OnClicked(this, &STimelinePanel::on_step_forward_clicked)
							.IsEnabled(this, &STimelinePanel::is_stop_enabled)
							.ToolTipText(LOCTEXT("StepForward", "Step Forward"))
							.ContentPadding(0.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(FAppStyle::GetBrush("Animation.Forward_Step"))
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
						]
					]

					// Spacer
					+ SHorizontalBox::Slot()
					.Padding(16.0f, 0.0f)
					.AutoWidth()
					[
						SNew(SSeparator)
						.Orientation(Orient_Vertical)
					]

					// Zoom controls
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4.0f, 2.0f)
					[
						SNew(SBox)
						.WidthOverride(32.0f)
						.HeightOverride(32.0f)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "Button")
							.OnClicked(this, &STimelinePanel::on_zoom_out_clicked)
							.IsEnabled(this, &STimelinePanel::is_zoom_out_enabled)
							.ToolTipText(LOCTEXT("ZoomOut", "Zoom Out"))
							.ContentPadding(0.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ZoomOutIcon", "-"))
								.Font(FAppStyle::GetFontStyle("BoldFont"))
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4.0f, 2.0f)
					[
						SNew(SBox)
						.WidthOverride(32.0f)
						.HeightOverride(32.0f)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "Button")
							.OnClicked(this, &STimelinePanel::on_zoom_reset_clicked)
							.IsEnabled(this, &STimelinePanel::is_zoom_out_enabled)
							.ToolTipText(LOCTEXT("ZoomReset", "Reset Zoom"))
							.ContentPadding(0.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ZoomResetIcon", "1:1"))
								.Font(FAppStyle::GetFontStyle("SmallFont"))
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(4.0f, 2.0f)
					[
						SNew(SBox)
						.WidthOverride(32.0f)
						.HeightOverride(32.0f)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "Button")
							.OnClicked(this, &STimelinePanel::on_zoom_in_clicked)
							.IsEnabled(this, &STimelinePanel::is_zoom_in_enabled)
							.ToolTipText(LOCTEXT("ZoomIn", "Zoom In"))
							.ContentPadding(0.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ZoomInIcon", "+"))
								.Font(FAppStyle::GetFontStyle("BoldFont"))
							]
						]
					]

					// Spacer
					+ SHorizontalBox::Slot()
					.Padding(16.0f, 0.0f)
					.AutoWidth()
					[
						SNew(SSeparator)
						.Orientation(Orient_Vertical)
					]

					// Timeline section (with time display and slider)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)

						// Time display
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						[
							SNew(SHorizontalBox)
							
							// Visible start time
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text_Lambda([this]() 
								{
									return FText::AsNumber(get_visible_start_time(), &FNumberFormattingOptions::DefaultNoGrouping());
								})
								.Font(FAppStyle::GetFontStyle("SmallFont"))
							]

							// Spacer
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(SHorizontalBox)
								
								// Time display (current)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(4.0f, 0.0f)
								[
									SNew(STextBlock)
									.Text(this, &STimelinePanel::get_current_time_text)
									.Font(FAppStyle::GetFontStyle("Bold"))
								]

								// Separator
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(2.0f, 0.0f)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("TimeSeparator", "/"))
								]

								// Max time display
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(4.0f, 0.0f)
								[
									SNew(STextBlock)
									.Text(this, &STimelinePanel::get_max_time_text)
								]
							]

							// Visible end time
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4.0f, 0.0f)
							[
								SNew(STextBlock)
								.Text_Lambda([this]() 
								{
									return FText::AsNumber(get_visible_end_time(), &FNumberFormattingOptions::DefaultNoGrouping());
								})
								.Font(FAppStyle::GetFontStyle("SmallFont"))
							]
						]

						// Timeline track with markers
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f)
						[
							SAssignNew(timeline_track, STimelineTrack)
							.start_time(visible_start_time)
							.end_time(visible_end_time)
							.is_zoomed_Lambda([this]() { return zoom_level > 1.0f; })
							.event_times_Lambda([this]() { return get_event_times(); })
							.OnPan(this, &STimelinePanel::on_timeline_pan)
							.OnZoom(this, &STimelinePanel::on_timeline_zoom)
						]

						// Timeline slider
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f)
						[
							SNew(SBox)
							.HeightOverride(24.0f)
							[
								SAssignNew(timeline_slider, SSlider)
								.Value(this, &STimelinePanel::get_timeline_value)
								.OnValueChanged(this, &STimelinePanel::on_timeline_value_changed)
								.SliderBarColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
								.SliderHandleColor(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f))
							]
						]
					]
				]
			]

			// Separator line
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f)
			[
				SNew(SBox)
				.Visibility(this, &STimelinePanel::get_content_visibility)
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
				]
			]

			// Properties display section
			+ SVerticalBox::Slot()
			.Padding(4.0f)
			[
				SNew(SBox)
				.Visibility(this, &STimelinePanel::get_content_visibility)
				[
					SNew(SScrollBox)
					.Orientation(Orient_Vertical)

					+ SScrollBox::Slot()
					.Padding(0.0f)
					[
						SNew(SHorizontalBox)

						// Left side: Flight data (60% width)
						+ SHorizontalBox::Slot()
						.FillWidth(0.25f)
						.Padding(4.0f, 2.0f)
						[
							SNew(SFlightDataWidget, playback_manager)
						]

						// Right side: Propellers (40% width)
						+ SHorizontalBox::Slot()
						.FillWidth(0.7f)
						.Padding(8.0f, 2.0f, 4.0f, 2.0f)
						[
							SNew(SVerticalBox)

							// Propeller header
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("PropellerHeader", "Propellers"))
								.Font(FAppStyle::GetFontStyle("BoldFont"))
							]

							// Propeller display widget
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f, 4.0f, 2.0f, 2.0f)
							[
								SNew(SPropellerDisplayWidget, playback_manager)
							]
						]
					]
				]
			]
		]
	];
}

void STimelinePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// The playback manager handles all timing updates via its own tick
	
	// Update timeline track time range if zoomed
	if (timeline_track.IsValid())
	{
		timeline_track->set_time_range(visible_start_time, visible_end_time);
	}
}

FReply STimelinePanel::on_play_clicked()
{
	playback_manager->play();
	return FReply::Handled();
}

FReply STimelinePanel::on_pause_clicked()
{
	playback_manager->pause();
	return FReply::Handled();
}

FReply STimelinePanel::on_step_forward_clicked()
{
	playback_manager->pause();
	playback_manager->step_to_next_event();
	return FReply::Handled();
}

FReply STimelinePanel::on_step_backward_clicked()
{
	playback_manager->pause();
	playback_manager->step_to_previous_event();
	return FReply::Handled();
}

void STimelinePanel::on_timeline_value_changed(float new_value)
{
	// Map the normalized value to the visible time range
	float new_time = FMath::Lerp(visible_start_time, visible_end_time, new_value);
	
	// Snap to nearest event if close enough
	float snapped_time = snap_to_nearest_event(new_time);
	playback_manager->seek_to_time(snapped_time);
	
	// Auto-pan if necessary when playing
	if (playback_manager->get_playback_state() == FFlightPlaybackManager::EPlaybackState::Playing)
	{
		update_visible_time_range();
	}
}

float STimelinePanel::get_timeline_value() const
{
	float current_time = playback_manager->get_current_time();
	
	// Map current time to the visible range
	float visible_duration = visible_end_time - visible_start_time;
	if (visible_duration > 0.0f)
	{
		return FMath::Clamp((current_time - visible_start_time) / visible_duration, 0.0f, 1.0f);
	}
	return 0.0f;
}

void STimelinePanel::on_timeline_value_committed(float new_value)
{
	// When user releases the timeline scrubber, pause playback
	if (playback_manager->get_playback_state() == FFlightPlaybackManager::EPlaybackState::Playing)
	{
		playback_manager->pause();
	}
}

FText STimelinePanel::get_current_time_text() const
{
	float current_time = playback_manager->get_current_time();
	return FText::AsNumber(current_time, &FNumberFormattingOptions::DefaultNoGrouping());
}

FText STimelinePanel::get_max_time_text() const
{
	float max_time = playback_manager->get_max_time();
	return FText::AsNumber(max_time, &FNumberFormattingOptions::DefaultNoGrouping());
}

EVisibility STimelinePanel::get_play_button_visibility() const
{
	auto state = playback_manager->get_playback_state();
	return (state != FFlightPlaybackManager::EPlaybackState::Playing) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility STimelinePanel::get_pause_button_visibility() const
{
	auto state = playback_manager->get_playback_state();
	return (state == FFlightPlaybackManager::EPlaybackState::Playing) ? EVisibility::Visible : EVisibility::Collapsed;
}

bool STimelinePanel::is_play_enabled() const
{
	auto state = playback_manager->get_playback_state();
	return state != FFlightPlaybackManager::EPlaybackState::Playing 
		&& playback_manager->has_flight_record()
		&& !playback_manager->is_pie_running();
}

bool STimelinePanel::is_pause_enabled() const
{
	auto state = playback_manager->get_playback_state();
	return state == FFlightPlaybackManager::EPlaybackState::Playing;
}

bool STimelinePanel::is_stop_enabled() const
{
	auto state = playback_manager->get_playback_state();
	float current_time = playback_manager->get_current_time();
	float min_time = playback_manager->get_min_time();
	return state != FFlightPlaybackManager::EPlaybackState::Stopped || current_time > min_time;
}

void STimelinePanel::on_flight_record_selected(const FAssetData& asset_data)
{
	UFlightRecordAsset* selected_record = Cast<UFlightRecordAsset>(asset_data.GetAsset());
	if (selected_record)
	{
		playback_manager->load_flight_record(selected_record);
		
		// Reset zoom and update visible time range to show full duration
		zoom_level = 1.0f;
		update_visible_time_range();
	}
	else
	{
		// Clear the flight record
		playback_manager->load_flight_record(nullptr);
		
		// Reset zoom and update visible time range
		zoom_level = 1.0f;
		update_visible_time_range();
	}
}

FString STimelinePanel::get_current_flight_record_path() const
{
	UFlightRecordAsset* flight_record = playback_manager->get_flight_record();
	if (flight_record)
	{
		return flight_record->GetPathName();
	}
	return FString();
}

EVisibility STimelinePanel::get_content_visibility() const
{
	return playback_manager->has_flight_record() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility STimelinePanel::get_no_asset_message_visibility() const
{
	return playback_manager->has_flight_record() ? EVisibility::Collapsed : EVisibility::Visible;
}

FReply STimelinePanel::on_zoom_in_clicked()
{
	// Increase zoom by 50%
	zoom_level = FMath::Min(zoom_level * 1.5f, 1000.0f);
	update_visible_time_range();
	return FReply::Handled();
}

FReply STimelinePanel::on_zoom_out_clicked()
{
	// Decrease zoom by 50%
	zoom_level = FMath::Max(zoom_level / 1.5f, 1.0f);
	update_visible_time_range();
	return FReply::Handled();
}

FReply STimelinePanel::on_zoom_reset_clicked()
{
	zoom_level = 1.0f;
	update_visible_time_range();
	return FReply::Handled();
}

void STimelinePanel::update_visible_time_range()
{
	float min_time = playback_manager->get_min_time();
	float max_time = playback_manager->get_max_time();
	float current_time = playback_manager->get_current_time();
	
	float total_duration = max_time - min_time;
	if (total_duration <= 0.0f)
	{
		visible_start_time = min_time;
		visible_end_time = max_time;
		return;
	}
	
	// Calculate visible duration based on zoom level
	float visible_duration = total_duration / zoom_level;
	
	// Center the visible range on current time
	float half_duration = visible_duration * 0.5f;
	visible_start_time = current_time - half_duration;
	visible_end_time = current_time + half_duration;
	
	// Clamp to valid range
	if (visible_start_time < min_time)
	{
		visible_start_time = min_time;
		visible_end_time = FMath::Min(min_time + visible_duration, max_time);
	}
	else if (visible_end_time > max_time)
	{
		visible_end_time = max_time;
		visible_start_time = FMath::Max(max_time - visible_duration, min_time);
	}
}

bool STimelinePanel::is_zoom_in_enabled() const
{
	return playback_manager->has_flight_record() && zoom_level < 1000.0f;
}

bool STimelinePanel::is_zoom_out_enabled() const
{
	return playback_manager->has_flight_record() && zoom_level > 1.0f;
}

void STimelinePanel::update_visible_time_range_with_center(float center_time)
{
	float min_time = playback_manager->get_min_time();
	float max_time = playback_manager->get_max_time();
	
	float total_duration = max_time - min_time;
	if (total_duration <= 0.0f)
	{
		visible_start_time = min_time;
		visible_end_time = max_time;
		return;
	}
	
	// Calculate visible duration based on zoom level
	float visible_duration = total_duration / zoom_level;
	
	// Center the visible range on the specified time
	float half_duration = visible_duration * 0.5f;
	visible_start_time = center_time - half_duration;
	visible_end_time = center_time + half_duration;
	
	// Clamp to valid range
	if (visible_start_time < min_time)
	{
		visible_start_time = min_time;
		visible_end_time = FMath::Min(min_time + visible_duration, max_time);
	}
	else if (visible_end_time > max_time)
	{
		visible_end_time = max_time;
		visible_start_time = FMath::Max(max_time - visible_duration, min_time);
	}
}

void STimelinePanel::pan_by_offset(float time_offset)
{
	float min_time = playback_manager->get_min_time();
	float max_time = playback_manager->get_max_time();
	float visible_duration = visible_end_time - visible_start_time;
	
	// Apply offset
	visible_start_time += time_offset;
	visible_end_time += time_offset;
	
	// Clamp to valid range
	if (visible_start_time < min_time)
	{
		visible_start_time = min_time;
		visible_end_time = min_time + visible_duration;
	}
	else if (visible_end_time > max_time)
	{
		visible_end_time = max_time;
		visible_start_time = max_time - visible_duration;
	}
}

FReply STimelinePanel::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Start panning with middle mouse button or Ctrl+Left click
	if ((MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton || 
		(MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && MouseEvent.IsControlDown())) 
		&& zoom_level > 1.0f)
	{
		is_panning = true;
		last_mouse_position = MouseEvent.GetScreenSpacePosition();
		pan_anchor_start_time = visible_start_time;
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	
	return FReply::Unhandled();
}

FReply STimelinePanel::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (is_panning)
	{
		is_panning = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	
	return FReply::Unhandled();
}

FReply STimelinePanel::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (is_panning)
	{
		FVector2D current_mouse = MouseEvent.GetScreenSpacePosition();
		FVector2D delta = current_mouse - last_mouse_position;
		
		// Convert pixel delta to time delta
		float visible_duration = visible_end_time - visible_start_time;
		float timeline_width = MyGeometry.GetLocalSize().X;
		
		if (timeline_width > 0.0f)
		{
			float time_delta = -(delta.X / timeline_width) * visible_duration;
			pan_by_offset(time_delta);
		}
		
		last_mouse_position = current_mouse;
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FReply STimelinePanel::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Note: Mouse wheel on timeline track is now handled by STimelineTrack widget
	return FReply::Unhandled();
}

void STimelinePanel::on_timeline_pan(float time_offset)
{
	pan_by_offset(time_offset);
}

void STimelinePanel::on_timeline_zoom(float wheel_delta)
{
	if (!playback_manager->has_flight_record())
	{
		return;
	}
	
	if (FMath::Abs(wheel_delta) > 0.0f)
	{
		// Zoom in/out based on wheel direction
		float old_zoom = zoom_level;
		
		if (wheel_delta > 0.0f)
		{
			// Zoom in
			zoom_level = FMath::Min(zoom_level * 1.2f, 1000.0f);
		}
		else
		{
			// Zoom out
			zoom_level = FMath::Max(zoom_level / 1.2f, 1.0f);
		}
		
		// If zoom changed, update visible range centered on current time
		if (zoom_level != old_zoom)
		{
			float current_time = playback_manager->get_current_time();
			update_visible_time_range_with_center(current_time);
		}
	}
}

float STimelinePanel::snap_to_nearest_event(float time) const
{
	UFlightRecordAsset* flight_record = playback_manager->get_flight_record();
	if (!flight_record || flight_record->events.Num() == 0)
	{
		return time;
	}
	
	// Calculate snap threshold based on visible duration and zoom level
	float visible_duration = visible_end_time - visible_start_time;
	float snap_threshold = visible_duration * 0.01f; // 1% of visible duration
	
	// Find nearest event
	float nearest_time = time;
	float nearest_distance = snap_threshold;
	
	for (const FFlightRecordEvent& event : flight_record->events)
	{
		float event_time = static_cast<float>(event.event_time);
		float distance = FMath::Abs(event_time - time);
		
		if (distance < nearest_distance)
		{
			nearest_distance = distance;
			nearest_time = event_time;
		}
	}
	
	return nearest_time;
}

TArray<float> STimelinePanel::get_event_times() const
{
	TArray<float> event_times;
	
	UFlightRecordAsset* flight_record = playback_manager->get_flight_record();
	if (flight_record)
	{
		for (const FFlightRecordEvent& event : flight_record->events)
		{
			event_times.Add(static_cast<float>(event.event_time));
		}
	}
	
	return event_times;
}

#undef LOCTEXT_NAMESPACE
