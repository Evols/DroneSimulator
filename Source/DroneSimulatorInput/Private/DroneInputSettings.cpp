#include "DroneSimulatorInput/Public/DroneInputSettings.h"

#include "DroneSimulatorInput.h"
#include "DroneSimulatorInput/Public/KnownHid.h"

#include "RawInput/Public/Windows/RawInputWindows.h"
#include "RawInput/Public/RawInput.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Windows/WindowsApplication.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

UDroneInputSettings* UDroneInputSettings::get_instance()
{
	return GetMutableDefault<UDroneInputSettings>();
}

TArray<FRawInputDeviceInfo> UDroneInputSettings::get_hid_devices()
{
	TArray<FRawInputDeviceInfo> all_devices = query_devices();
	TArray<FRawInputDeviceInfo> hid_devices;

	for (const FRawInputDeviceInfo& device : all_devices)
	{
		if (device.device_type == ERawInputDeviceType::HID)
		{
			hid_devices.Add(device);
		}
	}

	return hid_devices;
}

TArray<FRawInputDeviceInfo> UDroneInputSettings::find_devices_by_vendor_id(int32 vendor_id)
{
	TArray<FRawInputDeviceInfo> devices = get_hid_devices();
	TArray<FRawInputDeviceInfo> found_devices;

	for (const FRawInputDeviceInfo& device : devices)
	{
		if (device.vendor_id == vendor_id)
		{
			found_devices.Add(device);
		}
	}

	return found_devices;
}

bool UDroneInputSettings::register_device(int32 usage_page, int32 usage)
{
#if PLATFORM_WINDOWS
	if (!FRawInputPlugin::IsAvailable())
	{
		return false;
	}

	FRawInputWindows* raw_input = static_cast<FRawInputWindows*>(
		static_cast<FRawInputPlugin*>(&FRawInputPlugin::Get())->GetRawInputDevice().Get());

	if (raw_input == nullptr)
	{
		return false;
	}

	int32 flags = RIDEV_INPUTSINK;
	int32 handle = raw_input->RegisterInputDevice(
		RIM_TYPEHID,
		flags,
		static_cast<uint16>(usage),
		static_cast<int16>(usage_page),
		nullptr
	);

	return handle != INDEX_NONE;
#else
	return false;
#endif
}

void UDroneInputSettings::register_all_hid_devices()
{
	TArray<FRawInputDeviceInfo> devices = get_hid_devices();
	TSet<TPair<int32, int32>> registered;

	for (const FRawInputDeviceInfo& device : devices)
	{
		TPair<int32, int32> usage_pair(device.usage_page, device.usage);

		if (registered.Contains(usage_pair))
		{
			continue;
		}

		registered.Add(usage_pair);

		if (register_device(device.usage_page, device.usage))
		{
			UE_LOG(LogDroneSimulatorInput, Log, TEXT("Registered HID: VID:0x%04X PID:0x%04X (UsagePage:%d Usage:%d)"),
				device.vendor_id, device.product_id, device.usage_page, device.usage);
		}
	}
}

void UDroneInputSettings::log_hid_devices()
{
	TArray<FRawInputDeviceInfo> devices = get_hid_devices();

	UE_LOG(LogDroneSimulatorInput, Warning, TEXT("========== HID Devices =========="));
	UE_LOG(LogDroneSimulatorInput, Warning, TEXT("Total: %d"), devices.Num());

	for (const FRawInputDeviceInfo& device : devices)
	{
		log_device(device);
	}

	UE_LOG(LogDroneSimulatorInput, Warning, TEXT("================================="));
}

void UDroneInputSettings::initialize()
{
#if PLATFORM_WINDOWS
	if (!FRawInputPlugin::IsAvailable())
	{
		return;
	}

	FRawInputWindows* raw_input_windows = static_cast<FRawInputWindows*>(
		FRawInputPlugin::Get().GetRawInputDevice().Get());

	if (raw_input_windows != nullptr)
	{
		raw_input_windows->QueryConnectedDevices();
	}

	log_hid_devices();
	register_all_hid_devices();
#endif
}

FString UDroneInputSettings::get_device_vid(const FRawInputDeviceInfo& device)
{
	return FString::Printf(TEXT("%04X"), device.vendor_id);
}

FString UDroneInputSettings::get_device_pid(const FRawInputDeviceInfo& device)
{
	return FString::Printf(TEXT("%04X"), device.product_id);
}

TArray<FRawInputDeviceInfo> UDroneInputSettings::query_devices()
{
	TArray<FRawInputDeviceInfo> devices;

#if PLATFORM_WINDOWS
	UINT device_count = 0;
	if (GetRawInputDeviceList(nullptr, &device_count, sizeof(RAWINPUTDEVICELIST)) == static_cast<UINT>(-1))
	{
		return devices;
	}

	if (device_count == 0)
	{
		return devices;
	}

	TArray<RAWINPUTDEVICELIST> device_list;
	device_list.AddUninitialized(device_count);

	if (GetRawInputDeviceList(device_list.GetData(), &device_count, sizeof(RAWINPUTDEVICELIST)) == static_cast<UINT>(-1))
	{
		return devices;
	}

	TArray<char> name_buffer;

	for (const RAWINPUTDEVICELIST& raw_device : device_list)
	{
		FRawInputDeviceInfo device_info;

		UINT name_len = 0;
		if (GetRawInputDeviceInfoA(raw_device.hDevice, RIDI_DEVICENAME, nullptr, &name_len) == static_cast<UINT>(-1))
		{
			continue;
		}

		name_buffer.SetNumUninitialized(name_len + 1, EAllowShrinking::No);
		if (GetRawInputDeviceInfoA(raw_device.hDevice, RIDI_DEVICENAME, name_buffer.GetData(), &name_len) == static_cast<UINT>(-1))
		{
			continue;
		}

		name_buffer[name_len] = 0;
		device_info.device_name = ANSI_TO_TCHAR(name_buffer.GetData());
		device_info.device_name.ReplaceInline(TEXT("#"), TEXT("\\"), ESearchCase::CaseSensitive);

		RID_DEVICE_INFO device_info_raw = { 0 };
		UINT device_info_len = sizeof(device_info_raw);

		if (GetRawInputDeviceInfo(raw_device.hDevice, RIDI_DEVICEINFO, &device_info_raw, &device_info_len) == static_cast<UINT>(-1))
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				continue;
			}
			continue;
		}

		switch (device_info_raw.dwType)
		{
		case RIM_TYPEMOUSE:
			device_info.device_type = ERawInputDeviceType::Mouse;
			break;
		case RIM_TYPEKEYBOARD:
			device_info.device_type = ERawInputDeviceType::Keyboard;
			break;
		default:
			device_info.device_type = ERawInputDeviceType::Unknown;
			break;
		case RIM_TYPEHID:
			device_info.device_type = ERawInputDeviceType::HID;
			device_info.vendor_id = device_info_raw.hid.dwVendorId;
			device_info.vendor_known = UKnownHidLibrary::parse_vendor_id(device_info_raw.hid.dwVendorId);
			device_info.product_id = device_info_raw.hid.dwProductId;
			device_info.product_known = UKnownHidLibrary::parse_product_id(device_info.vendor_known, device_info_raw.hid.dwProductId);
			device_info.usage_page = device_info_raw.hid.usUsagePage;
			device_info.usage = device_info_raw.hid.usUsage;
			device_info.display_name = FString::Printf(TEXT("HID (VID:0x%04X PID:0x%04X) - %s"),
			                                           device_info.vendor_id, device_info.product_id, *device_info.device_name);
			break;
		}

		devices.Add(device_info);
	}
#endif

	return devices;
}

FString UDroneInputSettings::create_device_id(const FRawInputDeviceInfo& device) const
{
	if (device.device_type == ERawInputDeviceType::HID)
	{
		return FString::Printf(TEXT("HID_%04X_%04X_%d_%d"),
			device.vendor_id, device.product_id, device.usage_page, device.usage);
	}
	return FString::Printf(TEXT("%d_%s"), static_cast<int32>(device.device_type), *device.device_name);
}

void UDroneInputSettings::log_device(const FRawInputDeviceInfo& device) const
{
	UE_LOG(LogDroneSimulatorInput, Warning, TEXT("VID:0x%04X PID:0x%04X | UsagePage:%d Usage:%d"),
		device.vendor_id, device.product_id, device.usage_page, device.usage);
	UE_LOG(LogDroneSimulatorInput, Warning, TEXT("  %s"), *device.display_name);
	UE_LOG(LogDroneSimulatorInput, Warning, TEXT("  %s"), *device.device_name);
}
