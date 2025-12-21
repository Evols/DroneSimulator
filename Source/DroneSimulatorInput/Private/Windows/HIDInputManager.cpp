#include "DroneSimulatorInput/Private/Windows/HIDInputManager.h"

#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include "DroneSimulatorInput/Public/DroneInputTypes.h"
#include "DroneSimulatorInput/Public/DroneSimulatorInput.h"

#if PLATFORM_WINDOWS
#include <hidsdi.h>
#include <setupapi.h>
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")
#endif

FHIDInputManager &FHIDInputManager::get() {
	static FHIDInputManager instance;
	return instance;
}

void FHIDInputManager::initialize() {
	if (initialized)
		return;
	initialized = true;
	enumerate_devices();
}

void FHIDInputManager::shutdown() {
	for (auto &device : devices) {
		device->close();
	}
	devices.Empty();
	initialized = false;
}

void FHIDInputManager::poll_devices(UDroneInputSubsystem *input_subsystem) {
	if (!input_subsystem) {
		return;
	}

	constexpr double re_enumerate_delay_seconds = 2.0;

	// Periodically re-enumerate (e.g. every 2 seconds)
	double current_time = FPlatformTime::Seconds();
	if (current_time - this->last_enumeration_time > re_enumerate_delay_seconds) {
		enumerate_devices(input_subsystem);
		this->last_enumeration_time = current_time;
	}

	for (int32 i = devices.Num() - 1; i >= 0; --i) {
		TSharedPtr<FWindowsHIDDevice> device = devices[i];

		// Drain the input queue
		const int32 max_reads = 50;
		int32 reads = 0;

		while (reads < max_reads) {
			FDroneInputDevice device_data;
			EHIDPollStatus status = device->poll(device_data);

			if (status == EHIDPollStatus::Success) {
				reads++;
				if (input_subsystem) {
					input_subsystem->register_device(device_data.device_id,
																					 device_data.device_name,
																					 device_data.device_uid,
																					 true);

					// Forward to Manager
					// Axes
					for (const auto &pair : device_data.raw_axes) {
						FKey axis_key(pair.Key);
						input_subsystem->handle_raw_analog_input(device_data.device_id,
																										 axis_key, pair.Value);
					}

					// Raw Ints
					for (const auto &pair : device_data.raw_int_axes) {
						input_subsystem->update_raw_int_axis_value(device_data.device_id,
																											 pair.Key, pair.Value);
					}
				}
			} else if (status == EHIDPollStatus::Error) {
				// Device Disconnected
				UE_LOG(LogDroneSimulatorInput, Warning,
							 TEXT("Device %d Disconnected (Poll Error)"),
							 device->get_internal_id());
				input_subsystem->handle_device_disconnected(device->get_internal_id());
				device->close();
				devices.RemoveAt(i);
				break;
			} else {
				// NoData
				break;
			}
		}
	}
}

void FHIDInputManager::enumerate_devices(
		UDroneInputSubsystem *input_subsystem) {
#if PLATFORM_WINDOWS
	GUID hid_guid;
	HidD_GetHidGuid(&hid_guid);

	HDEVINFO device_info_set = SetupDiGetClassDevs(
			&hid_guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (device_info_set == INVALID_HANDLE_VALUE)
		return;

	SP_DEVICE_INTERFACE_DATA device_interface_data;
	device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	TSet<FString> found_device_paths;

	int32 device_index = 0;
	while (SetupDiEnumDeviceInterfaces(device_info_set, nullptr, &hid_guid,
																		 device_index++, &device_interface_data)) {
		DWORD required_size = 0;
		SetupDiGetDeviceInterfaceDetail(device_info_set, &device_interface_data,
																		nullptr, 0, &required_size, nullptr);

		if (required_size > 0) {
			PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data =
					(PSP_DEVICE_INTERFACE_DETAIL_DATA)FMemory::Malloc(required_size);
			detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (SetupDiGetDeviceInterfaceDetail(device_info_set,
																					&device_interface_data, detail_data,
																					required_size, nullptr, nullptr)) {
				FString device_path = detail_data->DevicePath;
				found_device_paths.Add(device_path);

				// Check if already exists
				bool exists = false;
				for (const auto &dev : devices) {
					if (dev->get_device_path() == device_path) {
						exists = true;
						break;
					}
				}

				if (!exists) {
					// Create new device
					int32 new_id = 1000 + devices.Num();
					// Note: This ID generation is not stable if devices are removed.
					// Ideally we should find a free ID or use a hash.
					// But for now, let's keep it simple.

					TSharedPtr<FWindowsHIDDevice> new_device =
							MakeShared<FWindowsHIDDevice>(new_id, device_path);
					if (new_device->open()) {
						devices.Add(new_device);
					}
				}
			}
			FMemory::Free(detail_data);
		}
	}

	SetupDiDestroyDeviceInfoList(device_info_set);

	// Reconciliation: Remove devices that are no longer present
	for (int32 i = devices.Num() - 1; i >= 0; --i) {
		if (!found_device_paths.Contains(devices[i]->get_device_path())) {
			UE_LOG(LogDroneSimulatorInput, Warning,
						 TEXT("Device %d Disconnected (Enumeration)"),
						 devices[i]->get_internal_id());
			if (input_subsystem) {
				input_subsystem->handle_device_disconnected(
						devices[i]->get_internal_id());
			}
			devices[i]->close();
			devices.RemoveAt(i);
		}
	}
#endif
}
