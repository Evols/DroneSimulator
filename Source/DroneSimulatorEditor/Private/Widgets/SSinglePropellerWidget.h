#pragma once

#include "CoreMinimal.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"
#include "Runtime/SlateCore/Public/Widgets/SCompoundWidget.h"

#include "DroneSimulator/Gameplay/DroneMovementComponent.h"

/**
 * Widget that displays a single propeller's information
 */
class SSinglePropellerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSinglePropellerWidget)
		: _propeller_name(FText::GetEmpty())
		, _border_color(FLinearColor::White)
		{}
		SLATE_ARGUMENT(FText, propeller_name)
		SLATE_ARGUMENT(FLinearColor, border_color)
		SLATE_ATTRIBUTE(const FPropellerPropulsionInfo*, propeller_data)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& in_args);

private:
	/** Attribute that provides the propeller data */
	TAttribute<const FPropellerPropulsionInfo*> propeller_data;

	/** Get individual property values as text */
	FText get_throttle_text() const;
	FText get_thrust_text() const;
	FText get_angular_speed_text() const;
	FText get_torque_text() const;
	FText get_aoa_text() const;
	FText get_reynolds_text() const;
	FText get_axial_vel_text() const;
	FText get_log_text() const;
};
