#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorGame/Assets/DroneAirfoilAsset.h"
#include "Misc/FileHelper.h"

/**
 * Parser for aero table CSV files generated from Python pipeline.
 * Data includes post-stall correction to ±90° from Viterna extrapolation.
 * 
 * Expected format:
 * Reynolds,Coefficient,AOA_-90,AOA_-88,...,AOA_90
 * 40000,CL,-0.823,-0.791,...,0.654
 * 40000,CD,1.234,1.198,...,1.456
 * 40000,CM,-0.023,-0.023,...,-0.023
 * 50119,CL,-0.831,-0.798,...,0.662
 * ...
 */

struct FParsedAeroTableCSV
{
	FString airfoil_name;
	TArray<FReynoldsXfoilData> reynolds_data;
};

inline TOptional<FParsedAeroTableCSV> parse_aero_table_csv(const FString& file_path, FFeedbackContext* warn)
{
	// Read the CSV file
	FString csv_content;
	if (!FFileHelper::LoadFileToString(csv_content, *file_path))
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("Failed to load aero table CSV file: %s"), *file_path);
		return TOptional<FParsedAeroTableCSV>();
	}

	// Parse the CSV
	TArray<FString> lines;
	csv_content.ParseIntoArrayLines(lines);

	if (lines.Num() < 2)
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("CSV file is too short: %s"), *file_path);
		return TOptional<FParsedAeroTableCSV>();
	}

	// Parse header line
	FString header_line = lines[0];
	TArray<FString> header_tokens;
	header_line.ParseIntoArray(header_tokens, TEXT(","), true);

	if (header_tokens.Num() < 3)
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("Invalid CSV header: %s"), *file_path);
		return TOptional<FParsedAeroTableCSV>();
	}

	// Verify expected format: Reynolds,Coefficient,AOA_-20,...
	if (!header_tokens[0].Equals(TEXT("Reynolds"), ESearchCase::IgnoreCase) ||
		!header_tokens[1].Equals(TEXT("Coefficient"), ESearchCase::IgnoreCase))
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("CSV header doesn't match expected format (Reynolds,Coefficient,AOA_...)"));
		return TOptional<FParsedAeroTableCSV>();
	}

	// Extract angles of attack from header (skip first 2 columns)
	TArray<float> aoa_values;
	for (int32 i = 2; i < header_tokens.Num(); ++i)
	{
		FString aoa_token = header_tokens[i].TrimStartAndEnd();
		
		// Remove "AOA_" prefix if present
		if (aoa_token.StartsWith(TEXT("AOA_"), ESearchCase::IgnoreCase))
		{
			aoa_token = aoa_token.RightChop(4); // Remove "AOA_"
		}
		
		float aoa = FCString::Atof(*aoa_token);
		aoa_values.Add(aoa);
	}

	if (warn) warn->Logf(ELogVerbosity::Display, TEXT("Parsing CSV with %d angle of attack values from %.1f° to %.1f°"), 
		aoa_values.Num(), aoa_values[0], aoa_values.Last());

	// Parse data rows
	// Group by Reynolds number (3 rows per Reynolds: CL, CD, CM)
	TMap<float, TMap<FString, TArray<float>>> reynolds_map;

	for (int32 line_idx = 1; line_idx < lines.Num(); ++line_idx)
	{
		FString line = lines[line_idx].TrimStartAndEnd();
		if (line.IsEmpty())
		{
			continue;
		}

		TArray<FString> tokens;
		line.ParseIntoArray(tokens, TEXT(","), true);

		if (tokens.Num() < 3)
		{
			if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Skipping invalid line %d: insufficient data"), line_idx + 1);
			continue;
		}

		// Parse Reynolds number
		float reynolds = FCString::Atof(*tokens[0]);

		// Parse coefficient type (CL, CD, or CM)
		FString coeff_type = tokens[1].TrimStartAndEnd().ToUpper();

		// Parse coefficient values
		TArray<float> coeff_values;
		for (int32 i = 2; i < tokens.Num(); ++i)
		{
			FString value_str = tokens[i].TrimStartAndEnd();
			if (value_str.IsEmpty())
			{
				coeff_values.Add(0.0f); // Default for missing data
			}
			else
			{
				coeff_values.Add(FCString::Atof(*value_str));
			}
		}

		// Store in map
		if (!reynolds_map.Contains(reynolds))
		{
			reynolds_map.Add(reynolds, TMap<FString, TArray<float>>());
		}
		reynolds_map[reynolds].Add(coeff_type, coeff_values);
	}

	// Convert map to FReynoldsXfoilData array
	FParsedAeroTableCSV parsed_data;
	
	// Extract airfoil name from filename
	FString filename = FPaths::GetBaseFilename(file_path);
	parsed_data.airfoil_name = filename;

	TArray<float> reynolds_numbers;
	reynolds_map.GetKeys(reynolds_numbers);
	reynolds_numbers.Sort();

	for (float reynolds : reynolds_numbers)
	{
		const TMap<FString, TArray<float>>& coeff_map = reynolds_map[reynolds];

		// Verify we have all three coefficient types
		if (!coeff_map.Contains(TEXT("CL")) || !coeff_map.Contains(TEXT("CD")) || !coeff_map.Contains(TEXT("CM")))
		{
			if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Incomplete data for Reynolds %.0f (missing CL/CD/CM), skipping"), reynolds);
			continue;
		}

		const TArray<float>& cl_values = coeff_map[TEXT("CL")];
		const TArray<float>& cd_values = coeff_map[TEXT("CD")];
		const TArray<float>& cm_values = coeff_map[TEXT("CM")];

		// Verify all arrays have same length
		if (cl_values.Num() != aoa_values.Num() || 
			cd_values.Num() != aoa_values.Num() || 
			cm_values.Num() != aoa_values.Num())
		{
			if (warn) warn->Logf(ELogVerbosity::Warning, TEXT("Mismatched data lengths for Reynolds %.0f, skipping"), reynolds);
			continue;
		}

		// Create FReynoldsXfoilData
		FReynoldsXfoilData reynolds_data;
		reynolds_data.reynolds_number = reynolds;

		for (int32 i = 0; i < aoa_values.Num(); ++i)
		{
			FAngleOfAttackXfoilData aoa_data;
			aoa_data.angle_of_attack = aoa_values[i];
			aoa_data.lift_coefficient = cl_values[i];
			aoa_data.drag_coefficient = cd_values[i];
			aoa_data.moment_coefficient = cm_values[i];

			reynolds_data.angle_of_attack_data.Add(aoa_data);
		}

		parsed_data.reynolds_data.Add(reynolds_data);
	}

	if (parsed_data.reynolds_data.Num() == 0)
	{
		if (warn) warn->Logf(ELogVerbosity::Error, TEXT("No valid Reynolds data found in CSV"));
		return TOptional<FParsedAeroTableCSV>();
	}

	if (warn) warn->Logf(ELogVerbosity::Display, TEXT("Successfully parsed %d Reynolds number datasets"), 
		parsed_data.reynolds_data.Num());

	return parsed_data;
}

