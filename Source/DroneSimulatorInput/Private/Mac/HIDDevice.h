#pragma once

#include "CoreMinimal.h"

#if PLATFORM_MAC

#include "DroneSimulatorInput/Public/DroneInputTypes.h"
#include <IOKit/hid/IOHIDLib.h>

enum class EHIDPollStatus { Success, NoData, Error };

class FMacHIDDevice {
public:
	FMacHIDDevice(int32 InInternalId, IOHIDDeviceRef InDeviceRef);
	~FMacHIDDevice();

	bool open();
	void close();
	EHIDPollStatus poll(FDroneInputDevice &out_device_data);

	int32 get_internal_id() const { return internal_id; }
	FString get_device_path() const { return device_path; }
	FString get_product_name() const { return product_name; }

private:
	int32 internal_id;
	FString device_path;
	FString product_name;
	FString serial_number;

	IOHIDDeviceRef device_ref;
	bool is_open = false;

	// We'll queue up events here since IOHIDManager is usually callback based,
	// but the Windows implementation is polling based.
	// For simplicity in this port, we can use IOHIDDeviceGetValue if we want
	// strict polling, or register a callback that pushes to a queue. Given the
	// architecture, let's try to query current state or use a small queue.

	// Actually, IOHIDDevice performs best with callbacks.
	// Let's implement a simple queue or just read current element values if
	// possible. However, for typical Game Input, we want events.

	// To keep it simple and consistent with the polling architecture:
	// We can use IOHIDDeviceCopyMatchingElements and read values? No, that's
	// slow. Better: Register input report callback, and queue data.

	struct FPendingInputReport {
		TMap<FName, float> axes;
		TMap<FName, int32> int_axes;
	};

	mutable FCriticalSection data_guard;
	TArray<FPendingInputReport> pending_reports;

	static void on_input_report_callback(void *context, IOReturn result,
																			 void *sender, IOHIDReportType type,
																			 uint32_t reportID, uint8_t *report,
																			 CFIndex reportLength);
	void handle_input_report(uint8_t *report, CFIndex reportLength);

	// Helper to extract values from report is hard without parsing HID descriptor
	// manually if we just get raw bytes. A better way for Mac:
	// IOHIDValueCallback.
	static void on_value_callback(void *context, IOReturn result, void *sender,
																IOHIDValueRef value);
	void handle_value_change(IOHIDValueRef value);

	// Current state buffer to assemble a "frame" for polling
	FDroneInputDevice current_state;
	bool has_new_data = false;

	// Store element cookies to names
	TMap<IOHIDElementCookie, FName> element_map;

	void parse_elements();
};

#endif
