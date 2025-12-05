#include "DroneInputCalibrator.h"
#include "DroneSimulatorInput.h"
#include "DroneInputProfileManager.h"
#include "DroneInputSubsystem.h"

FDroneInputCalibrator::FDroneInputCalibrator(UDroneInputSubsystem* InSubsystem, TSharedPtr<FDroneInputProfileManager> InProfileManager)
    : subsystem(InSubsystem)
    , profile_manager(InProfileManager)
{
}

void FDroneInputCalibrator::start_calibration(int32 device_id)
{
    if (calibrating_device_id != -1 && calibrating_device_id != device_id)
    {
        UE_LOG(LogDroneSimulatorInput, Warning, TEXT("FDroneInputCalibrator::start_calibration - Already calibrating DeviceId=%d, ignoring request for DeviceId=%d"), calibrating_device_id, device_id);
        return;
    }

    calibrating_device_id = device_id;
    calibration_phase = EDroneCalibrationPhase::None;
    axis_being_mapped = EDroneInputAxis::None;

    // Clear existing mappings and calibration data to start fresh
    if (subsystem)
    {
        subsystem->device_mappings.Remove(device_id);
        subsystem->calibration_data.Remove(device_id);
    }
    initial_axes_values.Remove(device_id);

    UE_LOG(LogDroneSimulatorInput, Log, TEXT("FDroneInputCalibrator::start_calibration - Started Calibration for DeviceId=%d (Mappings Cleared)"), device_id);
}

void FDroneInputCalibrator::stop_calibration()
{
    const int32 completed_device_id = calibrating_device_id;
    calibrating_device_id = -1;
    calibration_phase = EDroneCalibrationPhase::None;
    UE_LOG(LogDroneSimulatorInput, Log, TEXT("FDroneInputCalibrator::stop_calibration - Stopped Calibration"));

    if (subsystem && profile_manager.IsValid() && completed_device_id != -1)
    {
        if (const FDroneInputDevice* device = subsystem->devices.Find(completed_device_id))
        {
            const TMap<FName, FInputAxisMapping>* mappings = subsystem->device_mappings.Find(completed_device_id);
            const TMap<FName, FAxisCalibrationData>* calibrations = subsystem->calibration_data.Find(completed_device_id);
            profile_manager->update_saved_profile_for_device(*device, mappings, calibrations);
            profile_manager->save_profiles();
        }
    }
}

void FDroneInputCalibrator::start_mapping_axis(EDroneInputAxis axis_to_map)
{
    axis_being_mapped = axis_to_map;
    calibration_phase = EDroneCalibrationPhase::WaitingForInput;

    // Capture current values to detect delta
    initial_axes_values.Empty();
    if (subsystem)
    {
        for (const auto& device_pair : subsystem->devices)
        {
            initial_axes_values.Add(device_pair.Key, device_pair.Value.raw_axes);
        }
    }
    UE_LOG(LogDroneSimulatorInput, Log, TEXT("FDroneInputCalibrator::start_mapping_axis - Waiting for Input for Axis: %d"), (int32)axis_to_map);
}

void FDroneInputCalibrator::start_measuring_limits()
{
    calibration_phase = EDroneCalibrationPhase::MeasuringLimits;
    UE_LOG(LogDroneSimulatorInput, Log, TEXT("FDroneInputCalibrator::start_measuring_limits - Started Measuring Limits"));

    // Initialize limits to current values
    if (subsystem && subsystem->devices.Contains(calibrating_device_id))
    {
        const FDroneInputDevice& device = subsystem->devices[calibrating_device_id];
        if (subsystem->device_mappings.Contains(calibrating_device_id))
        {
            for (const auto& pair : subsystem->device_mappings[calibrating_device_id])
            {
                FName axis_name = pair.Key;
                if (device.raw_axes.Contains(axis_name))
                {
                    float current_val = device.raw_axes[axis_name];
                    FAxisCalibrationData& data = subsystem->calibration_data.FindOrAdd(calibrating_device_id).FindOrAdd(axis_name);
                    data.min_value = current_val;
                    data.max_value = current_val;
                    data.zero_value = current_val;
                }
            }
        }
    }
}

bool FDroneInputCalibrator::handle_raw_input(int32 device_id, FKey key, float value)
{
    if (device_id != calibrating_device_id)
    {
        return false;
    }

    // Filter out Vendor Defined Usage Pages (often used for motion sensors, touchpads, etc. which can be noisy)
    if (key.GetFName().ToString().StartsWith(TEXT("HID_")))
    {
        FString key_name = key.GetFName().ToString();
        // Format is HID_PPPP_UUUU
        if (key_name.Len() >= 9)
        {
            FString page_str = key_name.Mid(4, 4);
            int32 page = FParse::HexNumber(*page_str);
            if (page >= 0xFF00)
            {
                return false;
            }
        }
    }

    if (calibration_phase == EDroneCalibrationPhase::WaitingForInput)
    {
        // Check Delta from initial value
        TMap<FName, float>& device_initial_axes = initial_axes_values.FindOrAdd(device_id);
        float initial_val = value;
        if (const float* existing_initial_val = device_initial_axes.Find(key.GetFName()))
        {
            initial_val = *existing_initial_val;
        }
        else
        {
            device_initial_axes.Add(key.GetFName(), value);
        }

        float delta = FMath::Abs(value - initial_val);

        // Check if this physical axis is already assigned to another game axis
        if (subsystem && subsystem->device_mappings.Contains(device_id))
        {
            if (const FInputAxisMapping* existing_mapping = subsystem->device_mappings[device_id].Find(key.GetFName()))
            {
                // If it's mapped to something valid, AND it's not the one we are currently trying to map
                if (existing_mapping->game_axis != EDroneInputAxis::None && existing_mapping->game_axis != axis_being_mapped)
                {
                    // Ignore this input
                    return true; // Consumed, but ignored
                }
            }
        }

        if (delta > 0.5f)
        {
            // Map it!
            EDroneInputAxis mapped_axis = axis_being_mapped;
            if (subsystem)
            {
                subsystem->set_axis_mapping(device_id, key.GetFName(), mapped_axis);

                // Reset phase
                calibration_phase = EDroneCalibrationPhase::None;
                axis_being_mapped = EDroneInputAxis::None;

                UE_LOG(LogDroneSimulatorInput, Log, TEXT("FDroneInputCalibrator::handle_raw_input - Axis Mapped: DeviceId=%d, Key=%s -> GameAxis=%d"), device_id, *key.ToString(), (int32)mapped_axis);

                subsystem->on_axis_mapped.Broadcast(mapped_axis, key.GetFName());
                subsystem->on_axis_mapped_native.Broadcast(mapped_axis, key.GetFName());
            }
        }
        return true; // Consumed
    }
    else if (calibration_phase == EDroneCalibrationPhase::MeasuringLimits)
    {
        if (subsystem)
        {
            if (const TMap<FName, FInputAxisMapping>* mappings = subsystem->device_mappings.Find(device_id))
            {
                if (mappings->Contains(key.GetFName()))
                {
                    TMap<FName, FAxisCalibrationData>& device_calibration = subsystem->calibration_data.FindOrAdd(device_id);
                    const bool had_existing_entry = device_calibration.Contains(key.GetFName());
                    FAxisCalibrationData& data = device_calibration.FindOrAdd(key.GetFName());
                    if (!had_existing_entry)
                    {
                        data.min_value = value;
                        data.max_value = value;
                        data.zero_value = value;
                    }
                    data.min_value = FMath::Min(data.min_value, value);
                    data.max_value = FMath::Max(data.max_value, value);
                    data.zero_value = FMath::Clamp(data.zero_value, data.min_value, data.max_value);
                }
            }
        }
        return true; // Consumed
    }

    return false; // Not calibrating or not relevant
}

void FDroneInputCalibrator::on_device_disconnected(int32 device_id)
{
    if (calibrating_device_id == device_id)
    {
        calibrating_device_id = -1;
        calibration_phase = EDroneCalibrationPhase::None;
        axis_being_mapped = EDroneInputAxis::None;
    }
    initial_axes_values.Remove(device_id);
}
