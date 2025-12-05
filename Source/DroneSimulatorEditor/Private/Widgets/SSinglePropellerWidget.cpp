#include "DroneSimulatorEditor/Private/Widgets/SSinglePropellerWidget.h"

#include "Runtime/Slate/Public/Widgets/Layout/SBorder.h"
#include "Runtime/Slate/Public/Widgets/Text/STextBlock.h"
#include "Runtime/SlateCore/Public/Styling/AppStyle.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#define LOCTEXT_NAMESPACE "SinglePropellerWidget"

void SSinglePropellerWidget::Construct(const FArguments& in_args)
{
	propeller_data = in_args._propeller_data;

	constexpr double key_width_override = 80.f;
	constexpr double properties_width_override = 180.f;

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(in_args._border_color)
		.Padding(6.f, 4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(properties_width_override)
				[
					SNew(SVerticalBox)

					// Header
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 2.f, 2.f, 6.f)
					[
						SNew(STextBlock)
						.Text(in_args._propeller_name)
						.Justification(ETextJustify::Left)
					]

					// Throttle
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 1.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(key_width_override)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ThrottleLabel", "Throttle"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SSinglePropellerWidget::get_throttle_text)
						]
					]

					// Angular speed
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 1.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(key_width_override)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SpeedLabel", "Rotation speed"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SSinglePropellerWidget::get_angular_speed_text)
						]
					]

					// Axial velocity
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 1.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(key_width_override)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("AxialVelocityLabel", "Axial Velocity"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SSinglePropellerWidget::get_axial_vel_text)
						]
					]

					// Angle of Attack
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 1.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(key_width_override)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("AoALabel", "AoA"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SSinglePropellerWidget::get_aoa_text)
						]
					]

					// Reynolds
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 1.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(key_width_override)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ReynoldsLabel", "Reynolds"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SSinglePropellerWidget::get_reynolds_text)
						]
					]

					// Thrust
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 1.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(key_width_override)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ThrustLabel", "Thrust"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SSinglePropellerWidget::get_thrust_text)
						]
					]

					// Torque
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2.f, 1.f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 8.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(key_width_override)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("TorqueLabel", "Torque"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(this, &SSinglePropellerWidget::get_torque_text)
						]
					]
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SBox)
				.HeightOverride(150.f)
				[
					SNew(SMultiLineEditableTextBox)
						.IsReadOnly(true)
						.AlwaysShowScrollbars(true)
						.Text(this, &SSinglePropellerWidget::get_log_text)
				]
			]
		]
	];
}

FText SSinglePropellerWidget::get_throttle_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop)
	{
		return LOCTEXT("NoData", "-");
	}
	return FText::FromString(FString::Printf(TEXT("%.1f %%"), prop->throttle * 100.0));
}

FText SSinglePropellerWidget::get_thrust_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop)
	{
		return LOCTEXT("NoData", "-");
	}

	return FText::FromString(FString::Printf(TEXT("%.2f N"), prop->thrust));
}

FText SSinglePropellerWidget::get_angular_speed_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop)
	{
		return LOCTEXT("NoData", "-");
	}
	const auto rpm = prop->angular_speed / (TWO_PI / 60.0);
	return FText::FromString(FString::Printf(TEXT("%.0f rpm"), rpm));
}

FText SSinglePropellerWidget::get_torque_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop)
	{
		return LOCTEXT("NoData", "-");
	}
	return FText::FromString(FString::Printf(TEXT("%.5f N·m"), prop->torque));
}

FText SSinglePropellerWidget::get_aoa_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop)
	{
		return LOCTEXT("NoData", "-");
	}
	return FText::FromString(FString::Printf(TEXT("%.1f°"), FMath::RadiansToDegrees(prop->angle_of_attack)));
}

FString reynolds_to_string(double reynolds)
{
	if (reynolds >= 1000.0)
	{
		const auto k = reynolds / 1000.0;
		return k < 10.0 ? FString::Printf(TEXT("%.1fk"), k) : FString::Printf(TEXT("%dk"), (int32)k);
	}

	return FString::Printf(TEXT("%d"), (int32)reynolds);
}

FText SSinglePropellerWidget::get_reynolds_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop || prop->reynolds.Num() == 0)
	{
		return LOCTEXT("NoData", "-");
	}

	double min_reynolds = prop->reynolds[0];
	double max_reynolds = prop->reynolds[0];
	for (const auto reynolds : prop->reynolds)
	{
		min_reynolds = std::min(min_reynolds, reynolds);
		max_reynolds = std::max(max_reynolds, reynolds);
	}

	return FText::FromString(FString::Printf(TEXT("%s to %s"), *reynolds_to_string(min_reynolds), *reynolds_to_string(max_reynolds)));
}

FText SSinglePropellerWidget::get_axial_vel_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop)
	{
		return LOCTEXT("NoData", "-");
	}
	return FText::FromString(FString::Printf(TEXT("%.1f m/s"), prop->velocity_axial));
}

FText SSinglePropellerWidget::get_log_text() const
{
	const auto* prop = propeller_data.Get();
	if (!prop)
	{
		return FText::FromString(TEXT(""));
	}

	const auto& logs = propeller_data.Get()->debug_log.logs;
	
	FString aggregated_log = "";
	for (int32 i = 0; i < logs.Num(); i++)
	{
		if (i > 0)
		{
			aggregated_log += "\n";
		}
		aggregated_log += logs[i];
	}

	return FText::FromString(aggregated_log);
}

#undef LOCTEXT_NAMESPACE
