#include "DroneSimulator/Simulation/Structural.h"
#include "DroneSimulator/Assets/DroneAirfoilAsset.h"


FDroneAirfoilTable FDroneAirfoilTable::from_airfoil_asset_table(const UDroneAirfoilAssetTable* asset)
{
    FDroneAirfoilTable table;

    if (!asset)
    {
        return table;
    }

    const FRawXfoilData& xfoil_data = asset->imported_xfoil_data;

    // Reserve space for efficiency
    const int32 num_reynolds = xfoil_data.reynolds_data.Num();
    table.reynolds_numbers.Reserve(num_reynolds);
    table.reynolds_entries.Reserve(num_reynolds);

    // Convert from asset format to table format
    for (const FReynoldsXfoilData& reynolds_data : xfoil_data.reynolds_data)
    {
        // Add Reynolds number
        table.reynolds_numbers.Add(reynolds_data.reynolds_number);

        // Create entry for this Reynolds number
        FDroneAirfoilTable::FReynoldsEntry entry;
        const int32 num_aoa = reynolds_data.angle_of_attack_data.Num();
        entry.angles_of_attack.Reserve(num_aoa);
        entry.lift_coefficients.Reserve(num_aoa);
        entry.drag_coefficients.Reserve(num_aoa);

        // Convert angle of attack data
        for (const FAngleOfAttackXfoilData& aoa_data : reynolds_data.angle_of_attack_data)
        {
            // Convert angle from degrees to radians
            entry.angles_of_attack.Add(FMath::DegreesToRadians(aoa_data.angle_of_attack));
            entry.lift_coefficients.Add(aoa_data.lift_coefficient);
            entry.drag_coefficients.Add(aoa_data.drag_coefficient);
        }

        table.reynolds_entries.Add(MoveTemp(entry));
    }

    return table;
}

FDroneAirfoilSimplified FDroneAirfoilSimplified::from_airfoil_asset_simplified(
    const UDroneAirfoilAssetSimplified* asset)
{
    if (!asset)
    {
        return FDroneAirfoilSimplified();
    }

    return FDroneAirfoilSimplified(
        asset->cl_k_rad,
        asset->cd_0,
        asset->cd_k);
}
