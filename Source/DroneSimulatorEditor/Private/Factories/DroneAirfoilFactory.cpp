#include "DroneSimulatorEditor/Private/Factories/DroneAirfoilFactory.h"
#include "DroneSimulatorGame/Assets/DroneAirfoilAsset.h"
#include "DroneSimulatorEditor/Private/Factories/AirfoilParsing.h"
#include "DroneSimulatorEditor/Private/Factories/AeroTableCSVParsing.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Math/UnrealMathUtility.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "DroneAirfoilFactory"

UDroneAirfoilFactory::UDroneAirfoilFactory()
{
	bCreateNew = false;
	bEditAfterNew = false;
	bEditorImport = true;
	SupportedClass = UDroneAirfoilAssetTable::StaticClass();
	
	Formats.Add(TEXT("json;Airfoil Descriptor File (Legacy)"));
	Formats.Add(TEXT("csv;Aero Table CSV"));
}

bool UDroneAirfoilFactory::FactoryCanImport(const FString& filename)
{
	FString extension = FPaths::GetExtension(filename);
	FString base_filename = FPaths::GetCleanFilename(filename);
	
	// Accept airfoil_descriptor.json files
	if (base_filename.Equals(TEXT("airfoil_descriptor.json"), ESearchCase::IgnoreCase))
	{
		return true;
	}
	
	// Accept CSV files (aero table format)
	if (extension.Equals(TEXT("csv"), ESearchCase::IgnoreCase))
	{
		return true;
	}
	
	return false;
}

UObject* UDroneAirfoilFactory::FactoryCreateFile(UClass* in_class, UObject* in_parent, FName in_name, EObjectFlags flags, const FString& filename, const TCHAR* parms, FFeedbackContext* warn, bool& out_cancel_operation)
{
	out_cancel_operation = false;

	FString extension = FPaths::GetExtension(filename);
	
	// Create the asset
	UDroneAirfoilAssetTable* new_asset = NewObject<UDroneAirfoilAssetTable>(in_parent, in_class, in_name, flags);

	// Handle CSV import (aero table with Viterna post-stall correction from Python)
	if (extension.Equals(TEXT("csv"), ESearchCase::IgnoreCase))
	{
		if (warn) warn->Logf(ELogVerbosity::Display, TEXT("Importing aero table CSV: %s"), *filename);

		TOptional<FParsedAeroTableCSV> csv_data = parse_aero_table_csv(filename, warn);
		if (!csv_data.IsSet())
		{
			if (warn) warn->Logf(ELogVerbosity::Error, TEXT("Failed to parse CSV file: %s"), *filename);
			return nullptr;
		}

		new_asset->imported_name = csv_data->airfoil_name;
		new_asset->imported_xfoil_data.reynolds_data = csv_data->reynolds_data;

		if (warn) warn->Logf(ELogVerbosity::Display, 
			TEXT("✓ Successfully imported CSV with %d Reynolds datasets (includes post-stall correction to ±90° from Python pipeline)"), 
			csv_data->reynolds_data.Num());

		if (csv_data->reynolds_data.Num() > 0)
		{
			int32 num_aoa = csv_data->reynolds_data[0].angle_of_attack_data.Num();
			float min_aoa = csv_data->reynolds_data[0].angle_of_attack_data[0].angle_of_attack;
			float max_aoa = csv_data->reynolds_data[0].angle_of_attack_data.Last().angle_of_attack;
			
			if (warn) warn->Logf(ELogVerbosity::Display, 
				TEXT("  Coverage: %d angles from %.1f° to %.1f°"), 
				num_aoa, min_aoa, max_aoa);
			
			if (warn) warn->Logf(ELogVerbosity::Display, 
				TEXT("  Reynolds range: %.0f to %.0f"), 
				csv_data->reynolds_data[0].reynolds_number,
				csv_data->reynolds_data.Last().reynolds_number);
		}
	}
	// Handle JSON import (legacy .pol file format - limited angle range, no post-stall correction)
	else
	{
		if (warn) warn->Logf(ELogVerbosity::Display, TEXT("Importing legacy JSON descriptor: %s"), *filename);
		if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("⚠ Legacy format does not include post-stall correction. Use CSV format generated from Python pipeline for full ±90° coverage."));

		// Parse the airfoil descriptor JSON file
		TOptional<FParsedAirfoilDescriptor> descriptor = get_airfoil_descriptor(filename, warn);
		if (!descriptor.IsSet())
		{
			return nullptr;
		}

		// Get the directory containing the airfoil_descriptor.json
		FString base_directory = FPaths::GetPath(filename);

		new_asset->imported_name = descriptor->name;
		
		// Process each file entry from the parsed descriptor
		for (const FParsedAirfoilEntry& entry : descriptor->airfoil_entries)
		{
			// Construct the full path to the .pol file
			FString pol_file_path = FPaths::Combine(base_directory, entry.pol_file_name);

			// Parse the pol file
			TOptional<FReynoldsXfoilData> reynolds_data = parse_pol_file(pol_file_path, entry.reynolds_number, warn);

			if (reynolds_data.IsSet())
			{
				new_asset->imported_xfoil_data.reynolds_data.Add(reynolds_data.GetValue());
			}
		}

		if (new_asset->imported_xfoil_data.reynolds_data.Num() == 0)
		{
			if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("No valid data was imported from descriptor file: %s"), *filename);
		}
		else
		{
			if (warn) warn->Logf(ELogVerbosity::Display, TEXT("Successfully imported %d Reynolds number datasets (raw data, no post-stall correction)"), 
				new_asset->imported_xfoil_data.reynolds_data.Num());
		}
	}

	return new_asset;
}

uint32 UDroneAirfoilFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDroneAirfoilFactory::GetDisplayName() const
{
	return LOCTEXT("DroneAirfoilFactoryDisplayName", "Airfoil (Table)");
}

#undef LOCTEXT_NAMESPACE
