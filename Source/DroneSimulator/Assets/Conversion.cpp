#include "DroneSimulator/Assets/Conversion.h"
#include "DroneSimulator/Assets/DroneAirfoilAsset.h"
#include "DroneSimulator/Assets/DroneBatteryAsset.h"
#include "DroneSimulator/Assets/DroneFrameAsset.h"
#include "DroneSimulator/Assets/DroneMotorAsset.h"
#include "DroneSimulator/Assets/DronePropellerAsset.h"
#include "DroneSimulator/Simulation/Math.h"
#include "DroneSimulator/Simulation/Structural.h"
#include "Misc/Optional.h"


TOptional<FDronePropellerBemt> conversion::convert_propeller_bemt_asset(const UDronePropellerAsset* asset)
{
	const auto* asset_bemt = Cast<UDronePropellerBemtAsset>(asset);
	if (!asset_bemt || asset_bemt->airfoil == nullptr)
	{
		return TOptional<FDronePropellerBemt>();
	}

	FDronePropellerBemt result;

	result.num_blades = asset_bemt->num_blades;

	constexpr double inch_to_meters = 0.0254;

	// Convert units to SI (meters, radians)
	result.radius = asset_bemt->diameter_inch * inch_to_meters * 0.5; // inches to meters, diameter to radius
	result.hub_radius = asset_bemt->hub_radius_cm * 0.01; // cm to m
	result.chord = asset_bemt->chord_cm * 0.01; // cm to m

	result.pitch = asset_bemt->pitch_inch * inch_to_meters;

	result.airfoil = conversion::convert_airfoil_asset(asset_bemt->airfoil);

	return result;
}

TOptional<FDronePropellerSimplified> conversion::convert_propeller_simplified_asset(const UDronePropellerAsset* asset)
{
	return { FDronePropellerSimplified() };
}

FDroneAirfoilTable convert_airfoil_asset_table(const UDroneAirfoilAssetTable* asset)
{
	FDroneAirfoilTable result;

	// Use imported data (already includes Viterna post-stall correction from Python pipeline)
	const FRawXfoilData& data_source = asset->imported_xfoil_data;

	// Reserve space for efficiency
	result.reynolds_numbers.Reserve(data_source.reynolds_data.Num());
	result.reynolds_entries.Reserve(data_source.reynolds_data.Num());

	for (const FReynoldsXfoilData& reynolds_entry : data_source.reynolds_data)
	{
		result.reynolds_numbers.Add(reynolds_entry.reynolds_number);

		FDroneAirfoilTable::FReynoldsEntry entry;
		const int32 num_aoa = reynolds_entry.angle_of_attack_data.Num();

		entry.angles_of_attack.Reserve(num_aoa);
		entry.lift_coefficients.Reserve(num_aoa);
		entry.drag_coefficients.Reserve(num_aoa);

		for (const FAngleOfAttackXfoilData& aoa_data : reynolds_entry.angle_of_attack_data)
		{
			// Convert degrees to radians
			const float aoa_radians = FMath::DegreesToRadians(aoa_data.angle_of_attack);
			entry.angles_of_attack.Add(aoa_radians);
			entry.lift_coefficients.Add(aoa_data.lift_coefficient);
			entry.drag_coefficients.Add(aoa_data.drag_coefficient);
		}

		result.reynolds_entries.Add(entry);
	}

	return result;
}

FDroneAirfoil conversion::convert_airfoil_asset(const UDroneAirfoilAssetBase* asset)
{
	const auto* asset_table = Cast<UDroneAirfoilAssetTable>(asset);
	if (asset_table != nullptr)
	{
		const auto airfoil_table = convert_airfoil_asset_table(asset_table);
		return FDroneAirfoil(airfoil_table);
	}

	const auto* asset_simplified = Cast<UDroneAirfoilAssetSimplified>(asset);
	if (asset_simplified != nullptr)
	{
		FDroneAirfoilSimplified airfoil_simplified(
			asset_simplified->cl_k_rad,
			asset_simplified->cd_0,
			asset_simplified->cd_k
		);
		return FDroneAirfoil(airfoil_simplified);
	}

	return FDroneAirfoil();
}

FDroneFrame conversion::convert_frame_asset(const UDroneFrameAsset* asset)
{
	ensure(asset);

	FDroneFrame result;

	result.props_extent_back = asset->props_extent_back_cm;
	result.props_extent_front =  asset->props_extent_front_cm;
	result.area = asset->area_m2;
	result.drag_coefficient = asset->drag_coefficient;
	result.mass = asset->mass_kg;

	return result;
}

FDroneMotor conversion::convert_motor_asset(const UDroneMotorAsset* asset)
{
	ensure(asset);

	FDroneMotor result;

	result.kv = math::rpm_to_rad_per_sec(asset->kv);
	result.mass = asset->mass_kg;

	return result;
}

FDroneBattery conversion::convert_battery_asset(const UDroneBatteryAsset* asset)
{
	ensure(asset);

	FDroneBattery result;

	result.voltage = asset->voltage;
	result.mass = asset->mass_kg;

	return result;
}
