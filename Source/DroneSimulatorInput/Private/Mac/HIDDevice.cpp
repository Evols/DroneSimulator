#include "Mac/HIDDevice.h"

#if PLATFORM_MAC

#include "DroneSimulatorInput/Public/DroneSimulatorInput.h"

FMacHIDDevice::FMacHIDDevice(int32 InInternalId, IOHIDDeviceRef InDeviceRef)
		: internal_id(InInternalId), device_ref(InDeviceRef) {
	if (device_ref) {
		CFRetain(device_ref);

		// Get Basic Info
		CFStringRef Product = (CFStringRef)IOHIDDeviceGetProperty(
				device_ref, CFSTR(kIOHIDProductKey));
		if (Product) {
			char Buffer[256];
			if (CFStringGetCString(Product, Buffer, sizeof(Buffer),
														 kCFStringEncodingUTF8)) {
				product_name = FString(UTF8_TO_TCHAR(Buffer));
			}
		}

		// Path (Transport usually, or LocationID)
		CFNumberRef LocationID = (CFNumberRef)IOHIDDeviceGetProperty(
				device_ref, CFSTR(kIOHIDLocationIDKey));
		if (LocationID) {
			int32 Loc = 0;
			CFNumberGetValue(LocationID, kCFNumberSInt32Type, &Loc);
			device_path = FString::Printf(TEXT("Location_%d"), Loc);
		}

		// Serial
		CFStringRef Serial = (CFStringRef)IOHIDDeviceGetProperty(
				device_ref, CFSTR(kIOHIDSerialNumberKey));
		if (Serial) {
			char Buffer[256];
			if (CFStringGetCString(Serial, Buffer, sizeof(Buffer),
														 kCFStringEncodingUTF8)) {
				serial_number = FString(UTF8_TO_TCHAR(Buffer));
			}
		}
	}

	current_state.device_id = internal_id;
	current_state.device_name = product_name;
	current_state.device_uid = serial_number;
}

FMacHIDDevice::~FMacHIDDevice() {
	close();
	if (device_ref) {
		CFRelease(device_ref);
		device_ref = nullptr;
	}
}

bool FMacHIDDevice::open() {
	if (!device_ref)
		return false;

	if (is_open)
		return true;

	IOReturn Ret = IOHIDDeviceOpen(device_ref, kIOHIDOptionsTypeNone);
	if (Ret != kIOReturnSuccess) {
		UE_LOG(LogDroneSimulatorInput, Error, TEXT("Failed to open HID device: %x"),
					 Ret);
		return false;
	}

	parse_elements();

	IOHIDDeviceRegisterInputValueCallback(
			device_ref, &FMacHIDDevice::on_value_callback, this);
	IOHIDDeviceScheduleWithRunLoop(device_ref, CFRunLoopGetMain(),
																 kCFRunLoopDefaultMode);

	is_open = true;
	return true;
}

void FMacHIDDevice::close() {
	if (is_open && device_ref) {
		IOHIDDeviceUnscheduleFromRunLoop(device_ref, CFRunLoopGetMain(),
																		 kCFRunLoopDefaultMode);
		IOHIDDeviceClose(device_ref, kIOHIDOptionsTypeNone);
		is_open = false;
	}
}

void FMacHIDDevice::parse_elements() {
	if (!device_ref)
		return;

	CFArrayRef Elements = IOHIDDeviceCopyMatchingElements(device_ref, nullptr,
																												kIOHIDOptionsTypeNone);
	if (!Elements)
		return;

	CFIndex Count = CFArrayGetCount(Elements);
	for (CFIndex i = 0; i < Count; ++i) {
		IOHIDElementRef Element =
				(IOHIDElementRef)CFArrayGetValueAtIndex(Elements, i);
		IOHIDElementType Type = IOHIDElementGetType(Element);
		IOHIDElementCookie Cookie = IOHIDElementGetCookie(Element);
		uint32 UsagePage = IOHIDElementGetUsagePage(Element);
		uint32 Usage = IOHIDElementGetUsage(Element);

		// Simple mapping based on usage
		FName MappedName;

		if (UsagePage == kHIDPage_GenericDesktop) {
			switch (Usage) {
			case kHIDUsage_GD_X:
				MappedName = FName("Axis_X");
				break;
			case kHIDUsage_GD_Y:
				MappedName = FName("Axis_Y");
				break;
			case kHIDUsage_GD_Z:
				MappedName = FName("Axis_Z");
				break;
			case kHIDUsage_GD_Rx:
				MappedName = FName("Axis_Rx");
				break;
			case kHIDUsage_GD_Ry:
				MappedName = FName("Axis_Ry");
				break;
			case kHIDUsage_GD_Rz:
				MappedName = FName("Axis_Rz");
				break;
			case kHIDUsage_GD_Slider:
				MappedName = FName("Axis_Slider");
				break;
			case kHIDUsage_GD_Dial:
				MappedName = FName("Axis_Dial");
				break;
			case kHIDUsage_GD_Wheel:
				MappedName = FName("Axis_Wheel");
				break;
			default:
				break;
			}
		} else if (UsagePage == kHIDPage_Button) {
			MappedName = FName(*FString::Printf(TEXT("Button_%d"), Usage));
		}

		if (!MappedName.IsNone()) {
			element_map.Add(Cookie, MappedName);
		}
	}

	CFRelease(Elements);
}

void FMacHIDDevice::on_value_callback(void *context, IOReturn result,
																			void *sender, IOHIDValueRef value) {
	FMacHIDDevice *Self = static_cast<FMacHIDDevice *>(context);
	if (Self) {
		Self->handle_value_change(value);
	}
}

void FMacHIDDevice::handle_value_change(IOHIDValueRef value) {
	IOHIDElementRef Element = IOHIDValueGetElement(value);
	IOHIDElementCookie Cookie = IOHIDElementGetCookie(Element);

	if (FName *Name = element_map.Find(Cookie)) {
		CFIndex IntValue = IOHIDValueGetIntegerValue(value);
		double ScaledValue = IOHIDValueGetScaledValue(
				value, kIOHIDValueScaleTypePhysical); // Physical gives float roughly

		FScopeLock Lock(&data_guard);

		// Heuristic: If it's a button, put into raw_int_axes (0 or 1 usually)
		if (Name->ToString().Contains("Button")) {
			current_state.raw_int_axes.Add(*Name, (int32)IntValue);
		} else {
			// For axes, usually we want normalized or raw.
			// Windows impl seemed to pass raw integers but maybe interpreted as
			// float. Let's pass the integer value as float for now, as calibrator
			// expects raw range. Or ScaledValue if it's -1..1. Looking at Windows
			// impl: it just passes raw value.
			current_state.raw_axes.Add(*Name, (float)IntValue);
		}

		has_new_data = true;
	}
}

EHIDPollStatus FMacHIDDevice::poll(FDroneInputDevice &out_device_data) {
	FScopeLock Lock(&data_guard);

	if (has_new_data) {
		out_device_data = current_state;

		// Clear for next poll? Or keep state?
		// Poll usually implies "get events since last time" or "get current state".
		// Code in HIDInputManager/Subsystem seems to process Axes every frame.
		// If we clear, we might miss holding a button.
		// BUT, Subsystem Tick clears its own state:
		// "device_pair.Value.raw_axes.Empty();"
		// So we should just provide current state.

		// Wait, if subsystem clears it, we should provide the latest state we have.
		// But if no new data came in, `has_new_data` is false.
		// If we return NoData, the subsystem does NOTHING for this device this
		// frame. That means Axes will be empty in Subsystem.

		// Subsystem logic:
		// 1. Tick clears subsystem's view of raw axes.
		// 2. HIDManager calls Poll.
		// 3. If Poll Success, it registers device and forwards axes.

		// So we MUST return Success if we are connected, even if values haven't
		// changed? Windows impl: `device->poll(device_data)` Windows impl drains a
		// queue. "read" from "input_report_buffer". It seems Windows impl is event
		// based (GetOverlappedResult).

		// If I use callbacks, I am updating `current_state`.
		// If I return `current_state` every frame, I am emulating "Continuous
		// State".

		// HACK: To match Windows behavior of "reporting inputs when they change or
		// just happen", we can just return Success with current state. However, if
		// we return Success every time, we might flood. But Subsystem clears every
		// tick. So we NEED to refill every tick if we want the axis to be valid.
		// Actually, if we use `handle_raw_analog_input` it ADDS to the map.

		// If we just return `current_state` that accumulates updates, it should be
		// fine.

		has_new_data = false; // Reset flag ??
		// If we reset flag, next poll returns NoData (if we implement it that way).
		// If we return NoData, subsystem has empty axes for that device.
		// Does subsystem remember previous values?
		// `device_pair.Value.raw_axes.Empty();` -> NO, it clears it.

		// So we MUST provide the axis value EVERY FRAME if we want it to persist.
		// But Windows code:
		// `while (reads < max_reads)` -> it drains EVENTS.
		// If no events, it breaks.
		// So if no events, Subsystem gets NOTHING.
		// So Subsystem assumes 0 if nothing provided?

		// Let's check Subsystem again.
		// `Reset raw_axes`.
		// If nothing added, `get_raw_axis_value` returns 0.
		// So if I hold a stick at 50%, and stop moving, no new reports come in (on
		// some devices). On Windows HID, usually Joysticks spam reports even if not
		// moving? Or only on change? Usually only on change. If so, Subsystem logic
		// seems to imply "if no report, value is 0". That sounds wrong for a
		// Throttle.

		// Wait, `devices` is a map in Subsystem.
		// `device.raw_axes` is TMap.
		// Tick :: `device_pair.Value.raw_axes.Empty()`

		// This strongly suggests that if we don't send data, it resets to 0.
		// So we MUST send data every frame if we want to hold a value.
		// Windows `issue_read` -> `ReadFile`.
		// If device sends reports continuously, this works.
		// If device only sends on change, the drone would crash if you stop moving
		// stick.

		// Most Gamepads send periodic reports.
		// Mac IOHIDDevice behaves similarly.

		// So: return Success if we have data.
		// But do we clear `current_state`?
		// If I received 5 updates in between polls, `current_state` holds the
		// LATEST. I should return that.

		// If I return it, I should likely NOT clear it, just ensure `has_new_data`
		// logic is right. BUT, if I return it, and then next frame there is NO new
		// data, what happens? If I return NoData, drone crashes. So I must return
		// Success and the LAST KNOWN state.

		out_device_data = current_state;
		return EHIDPollStatus::Success;
	}

	// If no new data but we are open, maybe we should still return the state?
	// Let's try returning Success with last state.
	// NOTE: This might be redundant but safer for the "Clear every frame" logic.
	if (!current_state.raw_axes.IsEmpty()) {
		out_device_data = current_state;
		return EHIDPollStatus::Success;
	}

	return EHIDPollStatus::NoData;
}

#endif
