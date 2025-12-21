#pragma once

#include "InputCoreTypes.h"
#include "DroneInputTypes.h"

struct DRONESIMULATORINPUT_API FDroneInputKeys
{
	static const FKey throttle;
	static const FKey yaw;
	static const FKey pitch;
	static const FKey roll;
	static const FKey aux_1;
	static const FKey aux_2;
	static const FKey aux_3;
	static const FKey aux_4;
	static const FKey aux_5;
	static const FKey aux_6;
	static const FKey aux_7;
	static const FKey aux_8;
	static const FKey aux_9;
	static const FKey aux_10;
	static const FKey aux_11;
	static const FKey aux_12;

	static constexpr int32 aux_axis_count = drone_input::aux_axis_count;

	static bool is_drone_input_key(const FKey& key);
	static void register_keys();
	static const FKey& get_aux_key(int32 index);
};
