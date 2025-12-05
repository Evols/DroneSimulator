#include "DroneSimulator/RotorModel/Bemt/AirfoilCoefficients.h"
#include "DroneSimulator/Simulation/Structural.h"

/**
 * Performs bilinear interpolation for Xfoil table lookup.
 * @param reynolds Reynolds number
 * @param angle_of_attack Angle of attack in radians
 * @param airfoil The Xfoil lookup table
 * @return Coefficients if interpolation succeeded, none if out of bounds or invalid table
 */
TOptional<FAirfoilCoefficients> interpolate_airfoil_table_coefficients(double reynolds, double angle_of_attack, const FDroneAirfoilTable& airfoil)
{
	if (!airfoil.is_valid())
	{
		return {};
	}

	const int32 num_reynolds = airfoil.reynolds_numbers.Num();

	// Find Reynolds bounds using binary search
	int32 re_idx_low = 0;
	int32 re_idx_high = num_reynolds - 1;

	// Clamp Reynolds to available range
	if (reynolds <= airfoil.reynolds_numbers[0])
	{
		re_idx_low = re_idx_high = 0;
	}
	else if (reynolds >= airfoil.reynolds_numbers[num_reynolds - 1])
	{
		re_idx_low = re_idx_high = num_reynolds - 1;
	}
	else
	{
		// Binary search for Reynolds index
		for (int32 i = 0; i < num_reynolds - 1; ++i)
		{
			if (reynolds >= airfoil.reynolds_numbers[i] && reynolds <= airfoil.reynolds_numbers[i + 1])
			{
				re_idx_low = i;
				re_idx_high = i + 1;
				break;
			}
		}
	}

	// Lambda to interpolate within one Reynolds entry
	auto interpolate_at_reynolds = [&](int32 re_idx, double& cl, double& cd) -> bool
	{
		const auto& entry = airfoil.reynolds_entries[re_idx];
		const int32 num_aoa = entry.angles_of_attack.Num();

		if (num_aoa == 0)
		{
			return false;
		}

		// Clamp or find AoA bounds
		if (angle_of_attack <= entry.angles_of_attack[0])
		{
			cl = entry.lift_coefficients[0];
			cd = entry.drag_coefficients[0];
	
			// Validate clamped values
			if (!FMath::IsFinite(cl)) cl = 0.0;
			if (!FMath::IsFinite(cd)) cd = 0.1;
	
			return true;
		}
		if (angle_of_attack >= entry.angles_of_attack[num_aoa - 1])
		{
			cl = entry.lift_coefficients[num_aoa - 1];
			cd = entry.drag_coefficients[num_aoa - 1];
	
			// Validate clamped values
			if (!FMath::IsFinite(cl)) cl = 0.0;
			if (!FMath::IsFinite(cd)) cd = 0.1;
	
			return true;
		}

		// Linear search for AoA (small array, faster than binary)
		for (int32 i = 0; i < num_aoa - 1; ++i)
		{
			if (angle_of_attack >= entry.angles_of_attack[i] && angle_of_attack <= entry.angles_of_attack[i + 1])
			{
				const double aoa_low = entry.angles_of_attack[i];
				const double aoa_high = entry.angles_of_attack[i + 1];
				const double aoa_diff = aoa_high - aoa_low;
		
				// Safety check for division by zero
				if (FMath::Abs(aoa_diff) < 1e-9)
				{
					cl = entry.lift_coefficients[i];
					cd = entry.drag_coefficients[i];
				}
				else
				{
					const double alpha = (angle_of_attack - aoa_low) / aoa_diff;
					cl = FMath::Lerp(entry.lift_coefficients[i], entry.lift_coefficients[i + 1], alpha);
					cd = FMath::Lerp(entry.drag_coefficients[i], entry.drag_coefficients[i + 1], alpha);
				}
		
				// Validate results before returning
				if (!FMath::IsFinite(cl)) cl = 0.0;
				if (!FMath::IsFinite(cd)) cd = 0.1;
		
				return true;
			}
		}

		return false;
	};

	// Interpolate at both Reynolds numbers
	double cl_low = 0.0, cd_low = 0.0;
	double cl_high = 0.0, cd_high = 0.0;

	if (!interpolate_at_reynolds(re_idx_low, cl_low, cd_low))
	{
		return {};
	}

	if (re_idx_low == re_idx_high)
	{
		// No Reynolds interpolation needed
		double out_cl = cl_low;
		double out_cd = cd_low;

		// Final validation
		if (!FMath::IsFinite(out_cl)) out_cl = 0.0;
		if (!FMath::IsFinite(out_cd)) out_cd = 0.1;

		return { { out_cl, out_cd } };
	}

	if (!interpolate_at_reynolds(re_idx_high, cl_high, cd_high))
	{
		return {};
	}

	// Bilinear interpolation for Reynolds
	const double re_low = airfoil.reynolds_numbers[re_idx_low];
	const double re_high = airfoil.reynolds_numbers[re_idx_high];
	const double re_diff = re_high - re_low;

	double out_cl = 0.0, out_cd = 0.0;

	// Safety check for division by zero
	if (FMath::Abs(re_diff) < 1e-6)
	{
		out_cl = cl_low;
		out_cd = cd_low;
	}
	else
	{
		const double re_alpha = (reynolds - re_low) / re_diff;
		out_cl = FMath::Lerp(cl_low, cl_high, re_alpha);
		out_cd = FMath::Lerp(cd_low, cd_high, re_alpha);
	}

	// Final validation to prevent NaN/Inf propagation
	if (!FMath::IsFinite(out_cl)) out_cl = 0.0;
	if (!FMath::IsFinite(out_cd)) out_cd = 0.1;

	return { { out_cl, out_cd } };
}

/**
 * Computes airfoil coefficients using simplified linear model.
 * TRIVIAL CALCULATION for easy debugging:
 * - Cl = cl_k_rad * angle_of_attack (linear, no stall)
 * - Cd = cd_0 + cd_k * Cl^2 (profile drag + induced drag)
 *
 * @param angle_of_attack Angle of attack in radians
 * @param airfoil The simplified airfoil model
 * @return Lift and drag coefficients
 */
FAirfoilCoefficients compute_simplified_airfoil_coefficients(double angle_of_attack, const FDroneAirfoilSimplified& airfoil)
{
	// Linear lift coefficient
	const double cl = airfoil.cl_k_rad * angle_of_attack;

	// Drag = profile drag + induced drag (proportional to Cl^2)
	const double cd = airfoil.cd_0 + airfoil.cd_k * cl * cl;

	return FAirfoilCoefficients(cl, cd);
}

TOptional<FAirfoilCoefficients> simulation_bemt::interpolate_airfoil_coefficients(double reynolds, double angle_of_attack,
	const FDroneAirfoil& airfoil)
{
	if (airfoil.HasSubtype<FDroneAirfoilTable>())
	{
		return interpolate_airfoil_table_coefficients(reynolds, angle_of_attack, airfoil.GetSubtype<FDroneAirfoilTable>());
	}

	if (airfoil.HasSubtype<FDroneAirfoilSimplified>())
	{
		return compute_simplified_airfoil_coefficients(angle_of_attack, airfoil.GetSubtype<FDroneAirfoilSimplified>());
	}

	return {};
}
