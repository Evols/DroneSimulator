#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS

#include "Windows/WindowsHWrapper.h"
#include <hidsdi.h>
#include <setupapi.h>

#include "DroneSimulatorInput/Public/DroneInputTypes.h"

enum class EHIDPollStatus { Success, NoData, Error };

class FWindowsHIDDevice {
public:
  FWindowsHIDDevice(int32 InInternalId, const FString &InDevicePath);
  ~FWindowsHIDDevice();

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

  HANDLE device_handle;
  OVERLAPPED overlapped;
  PHIDP_PREPARSED_DATA preparsed_data;
  HIDP_CAPS capabilities;
  TArray<uint8> input_report_buffer;

  // Parsed Info
  TArray<HIDP_VALUE_CAPS> value_capabilities;
  TArray<HIDP_BUTTON_CAPS> button_capabilities;

  void parse_capabilities();
  void issue_read();
};

#endif
