#pragma once

#include "CoreMinimal.h"
#include "DroneInputTypes.h"

class UDroneInputSubsystem;
class FDroneInputProfileManager;

/**
 * Handles the interactive calibration process for drone input devices.
 * Manages state for mapping axes and measuring limits.
 */
class DRONESIMULATORINPUT_API FDroneInputCalibrator
{
public:
    FDroneInputCalibrator(UDroneInputSubsystem* InSubsystem, TSharedPtr<FDroneInputProfileManager> InProfileManager);
    virtual ~FDroneInputCalibrator() = default;

    void start_calibration(int32 device_id);
    void stop_calibration();
    void start_mapping_axis(EDroneInputAxis axis_to_map);
    void start_measuring_limits();

    // Returns true if input was consumed by calibration logic
    bool handle_raw_input(int32 device_id, FKey key, float value);

    void on_device_disconnected(int32 device_id);

    int32 get_calibrating_device_id() const { return calibrating_device_id; }
    EDroneCalibrationPhase get_calibration_phase() const { return calibration_phase; }

private:
    UDroneInputSubsystem* subsystem;
    TSharedPtr<FDroneInputProfileManager> profile_manager;

    int32 calibrating_device_id = -1;
    EDroneCalibrationPhase calibration_phase = EDroneCalibrationPhase::None;
    EDroneInputAxis axis_being_mapped = EDroneInputAxis::None;

    // Snapshot of values when calibration started, to detect movement delta
    TMap<int32, TMap<FName, float>> initial_axes_values;
};
