#pragma once

#include "CoreMinimal.h"

#include "HIDDevice.h"

class FHIDInputManager {
public:
  static FHIDInputManager &get();

  void initialize();
  void shutdown();
  void poll_devices(class UDroneInputSubsystem *input_subsystem);

private:
  FHIDInputManager() = default;

  void enumerate_devices(class UDroneInputSubsystem *input_subsystem = nullptr);

  TArray<TSharedPtr<FWindowsHIDDevice>> devices;
  bool initialized = false;
  double last_enumeration_time = 0.0;
};
