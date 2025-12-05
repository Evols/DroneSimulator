#pragma once

#include "CoreMinimal.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"
#include "Runtime/SlateCore/Public/Widgets/SCompoundWidget.h"

#include "DroneSimulator/Gameplay/DroneMovementComponent.h"

class FFlightPlaybackManager;

/**
 * Widget that displays propeller information in a drone shape layout
 */
class SPropellerDisplayWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPropellerDisplayWidget)
		{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& in_args, FFlightPlaybackManager* in_playback_manager);

private:
	/** Reference to the playback manager */
	FFlightPlaybackManager* playback_manager;

	/** Get propeller data for each motor */
	const FPropellerPropulsionInfo* get_propeller_fl_data() const;
	const FPropellerPropulsionInfo* get_propeller_fr_data() const;
	const FPropellerPropulsionInfo* get_propeller_rl_data() const;
	const FPropellerPropulsionInfo* get_propeller_rr_data() const;
};
