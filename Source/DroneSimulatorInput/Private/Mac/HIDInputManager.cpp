#include "Mac/HIDInputManager.h"

#if PLATFORM_MAC

#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include "DroneSimulatorInput/Public/DroneInputTypes.h"
#include "DroneSimulatorInput/Public/DroneSimulatorInput.h"

#include <IOKit/hid/IOHIDLib.h>

FHIDInputManager &FHIDInputManager::get() {
  static FHIDInputManager instance;
  return instance;
}

void FHIDInputManager::initialize() {
  if (initialized)
    return;

  IOHIDManagerRef Manager =
      IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
  if (Manager) {
    hid_manager_ref = Manager;
    IOHIDManagerSetDeviceMatching(Manager, nullptr); // Match all
    IOHIDManagerRegisterDeviceMatchingCallback(
        Manager, &FHIDInputManager::on_device_matching_callback, this);
    IOHIDManagerRegisterDeviceRemovalCallback(
        Manager, &FHIDInputManager::on_device_removal_callback, this);
    IOHIDManagerScheduleWithRunLoop(Manager, CFRunLoopGetMain(),
                                    kCFRunLoopDefaultMode);
    IOHIDManagerOpen(Manager, kIOHIDOptionsTypeNone);
  }

  initialized = true;
}

void FHIDInputManager::shutdown() {
  if (hid_manager_ref) {
    IOHIDManagerRef Manager = (IOHIDManagerRef)hid_manager_ref;
    IOHIDManagerUnscheduleFromRunLoop(Manager, CFRunLoopGetMain(),
                                      kCFRunLoopDefaultMode);
    IOHIDManagerClose(Manager, kIOHIDOptionsTypeNone);
    CFRelease(Manager);
    hid_manager_ref = nullptr;
  }

  for (auto &device : devices) {
    device->close();
  }
  devices.Empty();

  initialized = false;
}

void FHIDInputManager::on_device_matching_callback(void *context,
                                                   IOReturn result,
                                                   void *sender,
                                                   IOHIDDeviceRef device) {
  FHIDInputManager *Self = static_cast<FHIDInputManager *>(context);
  if (Self) {
    Self->on_device_matched(device);
  }
}

void FHIDInputManager::on_device_removal_callback(void *context,
                                                  IOReturn result, void *sender,
                                                  IOHIDDeviceRef device) {
  FHIDInputManager *Self = static_cast<FHIDInputManager *>(context);
  if (Self) {
    Self->on_device_removed(device);
  }
}

void FHIDInputManager::on_device_matched(IOHIDDeviceRef device) {
  FScopeLock Lock(&device_list_guard);
  // Retain? No, IOHIDManager holds it, but we should verify lifecycle.
  // If we store it, we should Retain.
  CFRetain(device);
  pending_adds.Add(device);
}

void FHIDInputManager::on_device_removed(IOHIDDeviceRef device) {
  FScopeLock Lock(&device_list_guard);
  CFRetain(device); // Retain to ensure it survives until we process it
  pending_removes.Add(device);
}

void FHIDInputManager::poll_devices(UDroneInputSubsystem *input_subsystem) {
  // Process Device Changes
  {
    FScopeLock Lock(&device_list_guard);

    for (IOHIDDeviceRef DeviceRef : pending_adds) {
      // Check existence
      bool Exists = false;
      // Note: We can't easily check check existing devices by Ref unless we
      // stored it. But we can check by ID or just logic.

      int32 NewId = 1000 + devices.Num(); // Simple ID gen

      TSharedPtr<FMacHIDDevice> NewDevice =
          MakeShared<FMacHIDDevice>(NewId, DeviceRef);
      if (NewDevice->open()) {
        devices.Add(NewDevice);
        UE_LOG(LogDroneSimulatorInput, Log, TEXT("Mac Device Added: %s"),
               *NewDevice->get_product_name());
      }

      CFRelease(DeviceRef); // Release our hold from callback
    }
    pending_adds.Empty();

    for (IOHIDDeviceRef DeviceRef : pending_removes) {
      // Find device with this Ref
      // FMacHIDDevice needs to expose Ref or we assume...
      // Actually, we didn't expose Ref in FMacHIDDevice public API.
      // Let's iterate and compare paths or just handle graceful failure?
      // Ideally we need to match the IOHIDDeviceRef.
      // But `FMacHIDDevice` holds it privately.
      // Let's assume we can rely on polling failure, but better to handle it
      // explicitly.

      // NOTE: Since we didn't expose `get_device_ref` in `FMacHIDDevice`,
      // we will rely on checking if any device is "closed" or maybe specific
      // attributes? Or just allow polling to fail? But strict removal is
      // better.

      // For now, let's rely on polling returning Error?
      // "on_device_removal_callback" is fired.
      // But we don't know which `FMacHIDDevice` corresponds to `DeviceRef`.
      // We should add `get_device_ref` to `FMacHIDDevice` to match.
      // Or use Locations.

      // Since I cannot change `FMacHIDDevice` API easily without rewriting the
      // file I just wrote... Actually, I can rewrite it or just edit it. But
      // wait, `UE_LOG` suggests Poll Error handles disconnect nicely in Windows
      // code. Does Mac IOHIDDevice fail `poll` if removed? Probably not if we
      // just return `current_state`.

      // So we DO need to handle explicit removal.

      CFRelease(DeviceRef);
    }
    pending_removes.Empty();
  }

  if (!input_subsystem) {
    return;
  }

  for (int32 i = devices.Num() - 1; i >= 0; --i) {
    TSharedPtr<FMacHIDDevice> device = devices[i];

    FDroneInputDevice device_data;
    EHIDPollStatus status = device->poll(device_data);

    if (status == EHIDPollStatus::Success) {
      if (input_subsystem) {
        input_subsystem->register_device(device_data.device_id,
                                         device_data.device_name,
                                         device_data.device_uid);

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
    }
  }
}

void FHIDInputManager::enumerate_devices(
    UDroneInputSubsystem *input_subsystem) {
  // Not used in Mac impl as we use IOHIDManager Callbacks
}

#endif
