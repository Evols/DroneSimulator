#pragma once

#include "CoreMinimal.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"
#include "Runtime/SlateCore/Public/Widgets/SCompoundWidget.h"

class FFlightPlaybackManager;

/**
 * Widget that displays flight data (location, rotation, velocity, controller input)
 */
class SFlightDataWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFlightDataWidget)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& in_args, FFlightPlaybackManager* in_playback_manager);

private:
	/** Reference to the playback manager */
	FFlightPlaybackManager* playback_manager;

	/** Get individual property values for widgets */
	TOptional<float> get_location_x() const;
	TOptional<float> get_location_y() const;
	TOptional<float> get_location_z() const;
	TOptional<float> get_rotation_roll() const;
	TOptional<float> get_rotation_pitch() const;
	TOptional<float> get_rotation_yaw() const;
	TOptional<float> get_velocity_x() const;
	TOptional<float> get_velocity_y() const;
	TOptional<float> get_velocity_z() const;
	TOptional<float> get_angular_velocity_x() const;
	TOptional<float> get_angular_velocity_y() const;
	TOptional<float> get_angular_velocity_z() const;
	TOptional<double> get_controller_throttle() const;
	TOptional<double> get_controller_yaw() const;
	TOptional<double> get_controller_pitch() const;
	TOptional<double> get_controller_roll() const;
};
