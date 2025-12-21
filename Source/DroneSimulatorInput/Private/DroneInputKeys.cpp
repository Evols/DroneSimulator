#include "DroneInputKeys.h"
#include "InputCoreTypes.h"
#include "Internationalization/Text.h"

#define LOCTEXT_NAMESPACE "DroneInputKeys"

const FKey FDroneInputKeys::throttle("Drone_Throttle");
const FKey FDroneInputKeys::yaw("Drone_Yaw");
const FKey FDroneInputKeys::pitch("Drone_Pitch");
const FKey FDroneInputKeys::roll("Drone_Roll");
const FKey FDroneInputKeys::aux_1("Drone_Aux1");
const FKey FDroneInputKeys::aux_2("Drone_Aux2");
const FKey FDroneInputKeys::aux_3("Drone_Aux3");
const FKey FDroneInputKeys::aux_4("Drone_Aux4");
const FKey FDroneInputKeys::aux_5("Drone_Aux5");
const FKey FDroneInputKeys::aux_6("Drone_Aux6");
const FKey FDroneInputKeys::aux_7("Drone_Aux7");
const FKey FDroneInputKeys::aux_8("Drone_Aux8");
const FKey FDroneInputKeys::aux_9("Drone_Aux9");
const FKey FDroneInputKeys::aux_10("Drone_Aux10");
const FKey FDroneInputKeys::aux_11("Drone_Aux11");
const FKey FDroneInputKeys::aux_12("Drone_Aux12");

bool FDroneInputKeys::is_drone_input_key(const FKey& key)
{
	return key.GetFName().ToString().StartsWith(TEXT("Drone_"));
}

void FDroneInputKeys::register_keys()
{
	static bool keys_registered = false;
	if (keys_registered)
	{
		return;
	}
	keys_registered = true;

	const FName category_name("Drone");
	EKeys::AddMenuCategoryDisplayInfo(category_name, LOCTEXT("DroneInputCategory", "Drone"), TEXT("Icons.Input"));

	const uint32 axis_flags = FKeyDetails::Axis1D | FKeyDetails::GamepadKey;

	EKeys::AddKey(FKeyDetails(throttle, LOCTEXT("DroneThrottleKey", "Drone Throttle"), axis_flags, category_name));
	EKeys::AddKey(FKeyDetails(yaw, LOCTEXT("DroneYawKey", "Drone Yaw"), axis_flags, category_name));
	EKeys::AddKey(FKeyDetails(pitch, LOCTEXT("DronePitchKey", "Drone Pitch"), axis_flags, category_name));
	EKeys::AddKey(FKeyDetails(roll, LOCTEXT("DroneRollKey", "Drone Roll"), axis_flags, category_name));

	const FKey aux_keys[] = {
		aux_1, aux_2, aux_3, aux_4, aux_5, aux_6, aux_7, aux_8, aux_9, aux_10, aux_11, aux_12
	};

	for (int32 index = 0; index < aux_axis_count; ++index)
	{
		const FString display_name = FString::Printf(TEXT("Drone Aux %d"), index + 1);
		EKeys::AddKey(FKeyDetails(aux_keys[index], FText::FromString(display_name), axis_flags, category_name));
	}
}

const FKey& FDroneInputKeys::get_aux_key(int32 index)
{
	static const FKey aux_keys[] = {
		aux_1, aux_2, aux_3, aux_4, aux_5, aux_6, aux_7, aux_8, aux_9, aux_10, aux_11, aux_12
	};

	if (index < 0 || index >= aux_axis_count)
	{
		return EKeys::Invalid;
	}

	return aux_keys[index];
}

#undef LOCTEXT_NAMESPACE
