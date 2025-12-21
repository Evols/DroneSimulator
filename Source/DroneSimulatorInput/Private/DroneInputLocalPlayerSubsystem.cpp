#include "DroneInputLocalPlayerSubsystem.h"

#include "DroneInputKeys.h"
#include "DroneInputSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

void UDroneInputLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GEngine)
	{
		device_subsystem = GEngine->GetEngineSubsystem<UDroneInputSubsystem>();
	}

	if (const ULocalPlayer* local_player = GetLocalPlayer())
	{
		platform_user_id = local_player->GetPlatformUserId();
	}

	if (!platform_user_id.IsValid())
	{
		platform_user_id = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
	}
}

void UDroneInputLocalPlayerSubsystem::Deinitialize()
{
	last_axis_device_ids.Empty();
	device_subsystem.Reset();

	Super::Deinitialize();
}

void UDroneInputLocalPlayerSubsystem::Tick(float DeltaTime)
{
	if (!device_subsystem.IsValid() || !FSlateApplication::IsInitialized())
	{
		return;
	}

	dispatch_axis(EDroneInputAxis::Throttle, FDroneInputKeys::throttle, throttle_calibration_space);
	dispatch_axis(EDroneInputAxis::Yaw, FDroneInputKeys::yaw, EThrottleCalibrationSpace::PositiveOnly);
	dispatch_axis(EDroneInputAxis::Pitch, FDroneInputKeys::pitch, EThrottleCalibrationSpace::PositiveOnly);
	dispatch_axis(EDroneInputAxis::Roll, FDroneInputKeys::roll, EThrottleCalibrationSpace::PositiveOnly);

	for (int32 aux_index = 0; aux_index < drone_input::aux_axis_count; ++aux_index)
	{
		const FKey aux_key = FDroneInputKeys::get_aux_key(aux_index);
		if (!aux_key.IsValid())
		{
			continue;
		}

		const EDroneInputAxis axis = static_cast<EDroneInputAxis>(
			static_cast<int32>(EDroneInputAxis::Aux1) + aux_index);
		dispatch_axis(axis, aux_key, EThrottleCalibrationSpace::PositiveOnly);
	}
}

void UDroneInputLocalPlayerSubsystem::set_precision_mode(EDroneInputPrecisionMode in_precision_mode)
{
	precision_mode = in_precision_mode;
}

void UDroneInputLocalPlayerSubsystem::set_throttle_calibration_space(EThrottleCalibrationSpace in_throttle_calibration_space)
{
	throttle_calibration_space = in_throttle_calibration_space;
}

EDroneInputPrecisionMode UDroneInputLocalPlayerSubsystem::get_precision_mode() const
{
	return precision_mode;
}

void UDroneInputLocalPlayerSubsystem::assign_device_to_local_player(int32 device_id)
{
	if (device_id == -1)
	{
		return;
	}

	if (!device_subsystem.IsValid())
	{
		return;
	}

	if (!platform_user_id.IsValid())
	{
		if (const ULocalPlayer* local_player = GetLocalPlayer())
		{
			platform_user_id = local_player->GetPlatformUserId();
		}

		if (!platform_user_id.IsValid())
		{
			platform_user_id = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
		}
	}

	device_subsystem->set_device_platform_user(device_id, platform_user_id);
}

void UDroneInputLocalPlayerSubsystem::dispatch_axis(EDroneInputAxis axis, const FKey& key,
	EThrottleCalibrationSpace axis_throttle_space)
{
	if (!key.IsValid() || !device_subsystem.IsValid())
	{
		return;
	}

	float axis_value = 0.0f;
	int32 device_id = INDEX_NONE;

	const bool has_value = device_subsystem->try_get_calibrated_axis_value_for_user(
		axis, axis_throttle_space, precision_mode, platform_user_id, axis_value, device_id);

	if (!has_value)
	{
		if (const int32* last_device_id = last_axis_device_ids.Find(axis))
		{
			device_id = *last_device_id;
			const FDroneInputDevice device = device_subsystem->get_device(device_id);
			if (device.device_id == -1)
			{
				last_axis_device_ids.Remove(axis);
				return;
			}
			axis_value = 0.0f;
		}
		else
		{
			return;
		}
	}
	else
	{
		last_axis_device_ids.Add(axis, device_id);
	}

	const bool is_hid_device = device_subsystem->is_hid_device(device_id);
	const FInputDeviceId input_device_id =
		device_subsystem->get_or_create_input_device_id(device_id, is_hid_device);

	if (!input_device_id.IsValid())
	{
		return;
	}

	FPlatformUserId dispatch_user_id = platform_user_id;
	if (!dispatch_user_id.IsValid())
	{
		dispatch_user_id = device_subsystem->get_platform_user_for_device(device_id);
	}

	FSlateApplication::Get().OnControllerAnalog(
		key.GetFName(), dispatch_user_id, input_device_id, axis_value);
}
