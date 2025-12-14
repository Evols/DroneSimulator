#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorGame/Assets/DroneAirfoilAsset.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

struct FParsedAirfoilEntry
{
	FString pol_file_name;
	double reynolds_number;	
};

struct FParsedAirfoilDescriptor
{
	FString name;
	TArray<FParsedAirfoilEntry> airfoil_entries;
};

inline TOptional<FParsedAirfoilDescriptor> get_airfoil_descriptor(const FString& file_path, FFeedbackContext* warn)
{
	// Read the airfoil_descriptor.json file
	FString json_content;
	if (!FFileHelper::LoadFileToString(json_content, *file_path))
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("Failed to load airfoil descriptor file: %s"), *file_path);
		return TOptional<FParsedAirfoilDescriptor> {};
	}

	// Parse the JSON
	TSharedPtr<FJsonObject> json_object;
	TSharedRef<TJsonReader<>> json_reader = TJsonReaderFactory<>::Create(json_content);

	if (!FJsonSerializer::Deserialize(json_reader, json_object) || !json_object.IsValid())
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("Failed to parse JSON from airfoil descriptor file: %s"), *file_path);
		return TOptional<FParsedAirfoilDescriptor> {};
	}

	FParsedAirfoilDescriptor descriptor;

	// Get "name" field
	if (!json_object->TryGetStringField(TEXT("name"), descriptor.name))
	{
		if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("No 'name' found in airfoil descriptor"));
		return TOptional<FParsedAirfoilDescriptor> {};
	}

	// Get the "files" array
	const TArray<TSharedPtr<FJsonValue>>* files_array;
	if (!json_object->TryGetArrayField(TEXT("files"), files_array))
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("No 'files' array found in descriptor file"));
		return TOptional<FParsedAirfoilDescriptor> {};
	}

	// Process each file entry
	for (const TSharedPtr<FJsonValue>& file_value : *files_array)
	{
		const TSharedPtr<FJsonObject>* file_object;
		if (!file_value->TryGetObject(file_object))
		{
			if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Invalid file entry in 'files' array"));
			return TOptional<FParsedAirfoilDescriptor> {};
		}

		FParsedAirfoilEntry entry;

		if (!(*file_object)->TryGetStringField(TEXT("pol_file_name"), entry.pol_file_name))
		{
			if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Missing 'pol_file_name' in file entry"));
			return TOptional<FParsedAirfoilDescriptor> {};
		}

		if (!(*file_object)->TryGetNumberField(TEXT("re"), entry.reynolds_number))
		{
			if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Missing 're' in file entry for %s"), *entry.pol_file_name);
			return TOptional<FParsedAirfoilDescriptor> {};
		}

		descriptor.airfoil_entries.Add(entry);
	}

	return descriptor;
}

inline TOptional<FReynoldsXfoilData> parse_pol_file(const FString& file_path, double reynolds_number, FFeedbackContext* warn)
{
	// Read the .pol file
	FString pol_content;
	if (!FFileHelper::LoadFileToString(pol_content, *file_path))
	{
		if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Failed to load .pol file: %s"), *file_path);
		return TOptional<FReynoldsXfoilData>();
	}

	// Parse the .pol file
	TArray<FString> lines;
	pol_content.ParseIntoArrayLines(lines);

	// Find the data section (after the header)
	int32 data_start_index = -1;
	for (int32 i = 0; i < lines.Num(); ++i)
	{
		// Look for the header line that contains "alpha    CL        CD"
		if (lines[i].Contains(TEXT("alpha")) && lines[i].Contains(TEXT("CL")) && lines[i].Contains(TEXT("CD")))
		{
			// Data starts after the dashed separator line
			data_start_index = i + 2;
			break;
		}
	}

	if (data_start_index < 0 || data_start_index >= lines.Num())
	{
		if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Could not find data section in .pol file: %s"), *file_path);
		return TOptional<FReynoldsXfoilData>();
	}

	// Create a Reynolds data entry
	FReynoldsXfoilData reynolds_data;
	reynolds_data.reynolds_number = static_cast<float>(reynolds_number);

	// Parse each data line
	for (int32 i = data_start_index; i < lines.Num(); ++i)
	{
		FString line = lines[i].TrimStartAndEnd();
		
		// Skip empty lines
		if (line.IsEmpty())
		{
			continue;
		}

		// Parse the line: alpha CL CD CDp CM Top_Xtr Bot_Xtr
		TArray<FString> tokens;
		line.ParseIntoArrayWS(tokens);

		// We need at least 3 values: alpha, CL, CD
		if (tokens.Num() < 3)
		{
			continue;
		}

		FAngleOfAttackXfoilData aoa_data;
		aoa_data.angle_of_attack = FCString::Atof(*tokens[0]); // Keep in degrees
		aoa_data.lift_coefficient = FCString::Atof(*tokens[1]);
		aoa_data.drag_coefficient = FCString::Atof(*tokens[2]);

		reynolds_data.angle_of_attack_data.Add(aoa_data);
	}

	if (reynolds_data.angle_of_attack_data.Num() == 0)
	{
		if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("No valid data found in .pol file: %s"), *file_path);
		return TOptional<FReynoldsXfoilData>();
	}

	return reynolds_data;
}