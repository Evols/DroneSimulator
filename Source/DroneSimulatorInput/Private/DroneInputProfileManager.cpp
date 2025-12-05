#include "DroneInputProfileManager.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformFilemanager.h"

void FDroneInputProfileManager::load_profiles()
{
	saved_profiles.Empty();

	const FString path = get_profile_file_path();
	if (!FPaths::FileExists(path))
	{
		return;
	}

	FString json_string;
	if (!FFileHelper::LoadFileToString(json_string, *path))
	{
		return;
	}

	TSharedPtr<FJsonObject> root;
	const TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(json_string);
	if (!FJsonSerializer::Deserialize(reader, root) || !root.IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* profiles_array = nullptr;
	if (!root->TryGetArrayField(TEXT("profiles"), profiles_array))
	{
		return;
	}

	for (const TSharedPtr<FJsonValue>& profile_value : *profiles_array)
	{
		const TSharedPtr<FJsonObject>* profile_object = nullptr;
		if (!profile_value->TryGetObject(profile_object) || !profile_object->IsValid())
		{
			continue;
		}

		FString uid;
		if (!(*profile_object)->TryGetStringField(TEXT("device_uid"), uid) || uid.IsEmpty())
		{
			continue;
		}

		FDroneInputProfile profile;

		const TArray<TSharedPtr<FJsonValue>>* mappings_array = nullptr;
		if ((*profile_object)->TryGetArrayField(TEXT("mappings"), mappings_array))
		{
			for (const TSharedPtr<FJsonValue>& mapping_value : *mappings_array)
			{
				const TSharedPtr<FJsonObject>* mapping_object = nullptr;
				if (!mapping_value->TryGetObject(mapping_object) || !mapping_object->IsValid())
				{
					continue;
				}

				FString axis_name;
				int32 game_axis_int = 0;
				bool is_inverted = false;
				if ((*mapping_object)->TryGetStringField(TEXT("axis"), axis_name)
					&& (*mapping_object)->TryGetNumberField(TEXT("game_axis"), game_axis_int)
					&& (*mapping_object)->TryGetBoolField(TEXT("is_inverted"), is_inverted))
				{
					FInputAxisMapping mapping;
					mapping.input_device_axis_name = FName(*axis_name);
					mapping.game_axis = static_cast<EDroneInputAxis>(game_axis_int);
					mapping.is_inverted = is_inverted;
					profile.mappings.Add(mapping.input_device_axis_name, mapping);
				}
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* calibration_array = nullptr;
		if ((*profile_object)->TryGetArrayField(TEXT("calibration"), calibration_array))
		{
			for (const TSharedPtr<FJsonValue>& calib_value : *calibration_array)
			{
				const TSharedPtr<FJsonObject>* calib_object = nullptr;
				if (!calib_value->TryGetObject(calib_object) || !calib_object->IsValid())
				{
					continue;
				}

				FString axis_name;
				double min_value = 0.0;
				double max_value = 0.0;
				double center_value = 0.0;
				if ((*calib_object)->TryGetStringField(TEXT("axis"), axis_name)
					&& (*calib_object)->TryGetNumberField(TEXT("min"), min_value)
					&& (*calib_object)->TryGetNumberField(TEXT("max"), max_value)
					&& (*calib_object)->TryGetNumberField(TEXT("center"), center_value))
				{
					FAxisCalibrationData calibration;
					calibration.min_value = static_cast<float>(min_value);
					calibration.max_value = static_cast<float>(max_value);
					calibration.zero_value = static_cast<float>(center_value);
					profile.calibrations.Add(FName(*axis_name), calibration);
				}
			}
		}

		saved_profiles.Add(uid, profile);
	}
}

void FDroneInputProfileManager::save_profiles() const
{
	TArray<TSharedPtr<FJsonValue>> profiles_array;
	profiles_array.Reserve(saved_profiles.Num());

	for (const auto& pair : saved_profiles)
	{
		const FString& uid = pair.Key;
		const FDroneInputProfile& profile = pair.Value;

		TSharedPtr<FJsonObject> profile_object = MakeShared<FJsonObject>();
		profile_object->SetStringField(TEXT("device_uid"), uid);

		TArray<TSharedPtr<FJsonValue>> mappings_array;
		mappings_array.Reserve(profile.mappings.Num());
		for (const auto& mapping_pair : profile.mappings)
		{
			TSharedPtr<FJsonObject> mapping_object = MakeShared<FJsonObject>();
			mapping_object->SetStringField(TEXT("axis"), mapping_pair.Key.ToString());
			mapping_object->SetNumberField(TEXT("game_axis"), static_cast<int32>(mapping_pair.Value.game_axis));
			mapping_object->SetBoolField(TEXT("is_inverted"), mapping_pair.Value.is_inverted);
			mappings_array.Add(MakeShared<FJsonValueObject>(mapping_object));
		}
		profile_object->SetArrayField(TEXT("mappings"), mappings_array);

		TArray<TSharedPtr<FJsonValue>> calibration_array;
		calibration_array.Reserve(profile.calibrations.Num());
		for (const auto& calib_pair : profile.calibrations)
		{
			TSharedPtr<FJsonObject> calib_object = MakeShared<FJsonObject>();
			calib_object->SetStringField(TEXT("axis"), calib_pair.Key.ToString());
			calib_object->SetNumberField(TEXT("min"), calib_pair.Value.min_value);
			calib_object->SetNumberField(TEXT("max"), calib_pair.Value.max_value);
			calib_object->SetNumberField(TEXT("center"), calib_pair.Value.zero_value);
			calibration_array.Add(MakeShared<FJsonValueObject>(calib_object));
		}
		profile_object->SetArrayField(TEXT("calibration"), calibration_array);

		profiles_array.Add(MakeShared<FJsonValueObject>(profile_object));
	}

	TSharedPtr<FJsonObject> root = MakeShared<FJsonObject>();
	root->SetArrayField(TEXT("profiles"), profiles_array);

	FString output_string;
	const TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&output_string);
	FJsonSerializer::Serialize(root.ToSharedRef(), writer);

	const FString path = get_profile_file_path();
	const FString directory = FPaths::GetPath(path);
	IPlatformFile& platform_file = FPlatformFileManager::Get().GetPlatformFile();
	platform_file.CreateDirectoryTree(*directory);

	FFileHelper::SaveStringToFile(output_string, *path);
}

void FDroneInputProfileManager::apply_saved_profile_if_any(const FDroneInputDevice& device, int32 device_id, TMap<int32, TMap<FName, FInputAxisMapping>>& device_mappings, TMap<int32, TMap<FName, FAxisCalibrationData>>& calibration_data) const
{
	const FString uid = get_device_uid(device);
	if (uid.IsEmpty())
	{
		return;
	}

	if (const FDroneInputProfile* profile = saved_profiles.Find(uid))
	{
		if (profile->mappings.Num() > 0)
		{
			device_mappings.Add(device_id, profile->mappings);
		}
		if (profile->calibrations.Num() > 0)
		{
			calibration_data.Add(device_id, profile->calibrations);
		}
	}
}

void FDroneInputProfileManager::update_saved_profile_for_device(const FDroneInputDevice& device, const TMap<FName, FInputAxisMapping>* mappings, const TMap<FName, FAxisCalibrationData>* calibrations)
{
	const FString uid = get_device_uid(device);
	if (uid.IsEmpty())
	{
		return;
	}

	FDroneInputProfile& profile = saved_profiles.FindOrAdd(uid);

	if (mappings)
	{
		profile.mappings = *mappings;
	}

	if (calibrations)
	{
		profile.calibrations = *calibrations;
	}
}

void FDroneInputProfileManager::remove_profile_for_device(const FDroneInputDevice& device)
{
	const FString uid = get_device_uid(device);
	if (uid.IsEmpty())
	{
		return;
	}

	if (saved_profiles.Remove(uid) > 0)
	{
		save_profiles();
	}
}

FString FDroneInputProfileManager::get_profile_file_path() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config"), TEXT("DroneInputMappings.json"));
}

FString FDroneInputProfileManager::get_device_uid(const FDroneInputDevice& device) const
{
	if (!device.device_uid.IsEmpty())
	{
		return device.device_uid;
	}
	if (!device.device_name.IsEmpty())
	{
		return device.device_name;
	}
	return FString();
}
