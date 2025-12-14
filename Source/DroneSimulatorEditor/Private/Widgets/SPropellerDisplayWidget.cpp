#include "DroneSimulatorEditor/Private/Widgets/SPropellerDisplayWidget.h"
#include "DroneSimulatorEditor/Private/Widgets/SSinglePropellerWidget.h"
#include "DroneSimulatorEditor/Private/Playback/FlightPlaybackManager.h"
#include "DroneSimulatorGame/Gameplay/Recording/FlightRecord.h"

#include "Runtime/Slate/Public/Widgets/Layout/SBorder.h"
#include "Runtime/Slate/Public/Widgets/Layout/SBox.h"
#include "Runtime/Slate/Public/Widgets/Text/STextBlock.h"
#include "Runtime/SlateCore/Public/Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PropellerDisplayWidget"

void SPropellerDisplayWidget::Construct(const FArguments& in_args, FFlightPlaybackManager* in_playback_manager)
{
	playback_manager = in_playback_manager;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Top row: Front Left and Front Right
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			// Front Left
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SSinglePropellerWidget)
				.propeller_name(LOCTEXT("FrontLeft", "Front Left ◢"))
				.border_color(FLinearColor(0.1f, 0.3f, 0.1f, 0.5f))
				.propeller_data_Lambda([this]() { return get_propeller_fl_data(); })
			]

			// Center spacer
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.0f, 0.0f)
			[
				SNew(SSpacer)
				.Size(FVector2D(1.0f, 1.0f))
			]

			// Front Right
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SSinglePropellerWidget)
				.propeller_name(LOCTEXT("FrontRight", "◣ Front Right"))
				.border_color(FLinearColor(0.1f, 0.3f, 0.1f, 0.5f))
				.propeller_data_Lambda([this]() { return get_propeller_fr_data(); })
			]
		]

		// Middle spacer
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNullWidget::NullWidget
		]

		// Bottom row: Rear Left and Rear Right
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			// Rear Left
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SSinglePropellerWidget)
				.propeller_name(LOCTEXT("RearLeft", "Rear Left ◥"))
				.border_color(FLinearColor(0.3f, 0.1f, 0.1f, 0.5f))
				.propeller_data_Lambda([this]() { return get_propeller_rl_data(); })
			]

			// Center spacer
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.0f, 0.0f)
			[
				SNew(SSpacer)
				.Size(FVector2D(1.0f, 1.0f))
			]

			// Rear Right
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SSinglePropellerWidget)
				.propeller_name(LOCTEXT("RearRight", "◤ Rear Right"))
				.border_color(FLinearColor(0.3f, 0.1f, 0.1f, 0.5f))
				.propeller_data_Lambda([this]() { return get_propeller_rr_data(); })
			]
		]
	];
}

const FPropellerPropulsionInfo* SPropellerDisplayWidget::get_propeller_fl_data() const
{
	if (!playback_manager)
	{
		return nullptr;
	}

	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	if (!event_data || !event_data->propulsion_info.IsSet())
	{
		return nullptr;
	}

	return &event_data->propulsion_info->front_left;
}

const FPropellerPropulsionInfo* SPropellerDisplayWidget::get_propeller_fr_data() const
{
	if (!playback_manager)
	{
		return nullptr;
	}

	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	if (!event_data || !event_data->propulsion_info.IsSet())
	{
		return nullptr;
	}

	return &event_data->propulsion_info->front_right;
}

const FPropellerPropulsionInfo* SPropellerDisplayWidget::get_propeller_rl_data() const
{
	if (!playback_manager)
	{
		return nullptr;
	}

	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	if (!event_data || !event_data->propulsion_info.IsSet())
	{
		return nullptr;
	}

	return &event_data->propulsion_info->rear_left;
}

const FPropellerPropulsionInfo* SPropellerDisplayWidget::get_propeller_rr_data() const
{
	if (!playback_manager)
	{
		return nullptr;
	}

	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	if (!event_data || !event_data->propulsion_info.IsSet())
	{
		return nullptr;
	}

	return &event_data->propulsion_info->rear_right;
}

#undef LOCTEXT_NAMESPACE
