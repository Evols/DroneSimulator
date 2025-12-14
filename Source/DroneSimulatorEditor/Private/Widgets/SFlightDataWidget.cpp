#include "DroneSimulatorEditor/Private/Widgets/SFlightDataWidget.h"
#include "DroneSimulatorEditor/Private/Playback/FlightPlaybackManager.h"
#include "DroneSimulatorGame/Gameplay/Recording/FlightRecord.h"

#include "Runtime/Slate/Public/Widgets/Layout/SBox.h"
#include "Runtime/Slate/Public/Widgets/Text/STextBlock.h"
#include "Runtime/Slate/Public/Widgets/Input/SVectorInputBox.h"
#include "Runtime/Slate/Public/Widgets/Input/SRotatorInputBox.h"
#include "Runtime/Slate/Public/Widgets/Input/SNumericEntryBox.h"
#include "Runtime/SlateCore/Public/Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FlightDataWidget"

void SFlightDataWidget::Construct(const FArguments& in_args, FFlightPlaybackManager* in_playback_manager)
{
	playback_manager = in_playback_manager;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Location
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LocationLabel", "Location (uu)"))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(SVectorInputBox)
				.X(this, &SFlightDataWidget::get_location_x)
				.Y(this, &SFlightDataWidget::get_location_y)
				.Z(this, &SFlightDataWidget::get_location_z)
				.bColorAxisLabels(true)
			]
		]

		// Rotation
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f, 8.0f, 2.0f, 2.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RotationLabel", "Rotation (deg)"))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(SRotatorInputBox)
				.Roll(this, &SFlightDataWidget::get_rotation_roll)
				.Pitch(this, &SFlightDataWidget::get_rotation_pitch)
				.Yaw(this, &SFlightDataWidget::get_rotation_yaw)
				.bColorAxisLabels(true)
			]
		]

		// Velocity
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f, 8.0f, 2.0f, 2.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VelocityLabel", "Velocity (m/s)"))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(SVectorInputBox)
				.X(this, &SFlightDataWidget::get_velocity_x)
				.Y(this, &SFlightDataWidget::get_velocity_y)
				.Z(this, &SFlightDataWidget::get_velocity_z)
				.bColorAxisLabels(true)
			]
		]

		// Angular Velocity
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f, 8.0f, 2.0f, 2.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AngularVelocityLabel", "Angular Velocity (deg/s)"))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(SVectorInputBox)
				.X(this, &SFlightDataWidget::get_angular_velocity_x)
				.Y(this, &SFlightDataWidget::get_angular_velocity_y)
				.Z(this, &SFlightDataWidget::get_angular_velocity_z)
				.bColorAxisLabels(true)
			]
		]

		// Controller Input header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f, 16.0f, 2.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ControllerHeader", "Controller Input"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]

		// Controller inputs
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2.0f)
		[
			SNew(SHorizontalBox)

			// Throttle
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ThrottleLabel", "Throttle"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SNumericEntryBox<double>)
					.Value(this, &SFlightDataWidget::get_controller_throttle)
					.AllowSpin(false)
				]
			]

			// Yaw
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("YawLabel", "Yaw"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SNumericEntryBox<double>)
					.Value(this, &SFlightDataWidget::get_controller_yaw)
					.AllowSpin(false)
				]
			]

			// Pitch
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PitchLabel", "Pitch"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SNumericEntryBox<double>)
					.Value(this, &SFlightDataWidget::get_controller_pitch)
					.AllowSpin(false)
				]
			]

			// Roll
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RollLabel", "Roll"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SNumericEntryBox<double>)
					.Value(this, &SFlightDataWidget::get_controller_roll)
					.AllowSpin(false)
				]
			]
		]
	];
}

TOptional<float> SFlightDataWidget::get_location_x() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->location.X) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_location_y() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->location.Y) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_location_z() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->location.Z) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_rotation_roll() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->rotation.Roll) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_rotation_pitch() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->rotation.Pitch) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_rotation_yaw() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->rotation.Yaw) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_velocity_x() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->velocity.X) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_velocity_y() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->velocity.Y) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_velocity_z() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(event_data->velocity.Z) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_angular_velocity_x() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(FMath::RadiansToDegrees(event_data->angular_velocity.X)) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_angular_velocity_y() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(FMath::RadiansToDegrees(event_data->angular_velocity.Y)) : TOptional<float>();
}

TOptional<float> SFlightDataWidget::get_angular_velocity_z() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<float>(FMath::RadiansToDegrees(event_data->angular_velocity.Z)) : TOptional<float>();
}

TOptional<double> SFlightDataWidget::get_controller_throttle() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<double>(event_data->controller_input.throttle) : TOptional<double>();
}

TOptional<double> SFlightDataWidget::get_controller_yaw() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<double>(event_data->controller_input.yaw) : TOptional<double>();
}

TOptional<double> SFlightDataWidget::get_controller_pitch() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<double>(event_data->controller_input.pitch) : TOptional<double>();
}

TOptional<double> SFlightDataWidget::get_controller_roll() const
{
	const FFlightRecordEventData* event_data = playback_manager->get_current_event_data();
	return event_data ? TOptional<double>(event_data->controller_input.roll) : TOptional<double>();
}

#undef LOCTEXT_NAMESPACE

