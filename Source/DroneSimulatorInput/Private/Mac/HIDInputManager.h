#pragma once

#include "CoreMinimal.h"

#if PLATFORM_MAC

#include "Mac/HIDDevice.h"

class FHIDInputManager {
public:
  static FHIDInputManager &get();

  void initialize();
  void shutdown();
  void poll_devices(class UDroneInputSubsystem *input_subsystem);

private:
  FHIDInputManager() = default;

  void enumerate_devices(class UDroneInputSubsystem *input_subsystem = nullptr);

  TArray<TSharedPtr<FMacHIDDevice>> devices;
  bool initialized = false;
  double last_enumeration_time = 0.0;

  // IOHIDManagerRef
  // We can use IOHIDManager to detect devices instead of manual enumeration
  // loop
  void *hid_manager_ref =
      nullptr; // void* to avoid including IOKit in header if possible, but we
               // included in HIDDevice.h so it's fine.

  static void on_device_matching_callback(void *context, IOReturn result,
                                          void *sender, IOHIDDeviceRef device);
  static void on_device_removal_callback(void *context, IOReturn result,
                                         void *sender, IOHIDDeviceRef device);

  void on_device_matched(IOHIDDeviceRef device);
  void on_device_removed(IOHIDDeviceRef device);

  // Queue for device changes to be processed on GameThread poll
  TArray<IOHIDDeviceRef> pending_adds;
  TArray<IOHIDDeviceRef> pending_removes;

  FCriticalSection device_list_guard;
};

#endif
