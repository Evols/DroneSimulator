#pragma once

#include "CoreMinimal.h"
#include "DroneInputTypes.h"

struct FDroneInputProfile
{
	TMap<FName, FInputAxisMapping> mappings;
	TMap<FName, FAxisCalibrationData> calibrations;
};

/**
 * Handles persistence of input profiles (mappings + calibrations) to disk.
 * Keeps serialization concerns out of the input subsystem.
 */
class FDroneInputProfileManager
{
public:
	FDroneInputProfileManager() = default;
	~FDroneInputProfileManager() = default;

	void load_profiles();
	void save_profiles() const;

	void apply_saved_profile_if_any(const FDroneInputDevice& device, int32 device_id, TMap<int32, TMap<FName, FInputAxisMapping>>& device_mappings, TMap<int32, TMap<FName, FAxisCalibrationData>>& calibration_data) const;
	void update_saved_profile_for_device(const FDroneInputDevice& device, const TMap<FName, FInputAxisMapping>* mappings, const TMap<FName, FAxisCalibrationData>* calibrations);
	void remove_profile_for_device(const FDroneInputDevice& device);

private:
	FString get_profile_file_path() const;
	FString get_device_uid(const FDroneInputDevice& device) const;

	TMap<FString, FDroneInputProfile> saved_profiles;
};
