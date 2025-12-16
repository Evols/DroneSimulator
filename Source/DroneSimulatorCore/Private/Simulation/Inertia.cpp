#include "DroneSimulatorCore/Public/Simulation/Inertia.h"

#include "Utils/Variant.h"

double inertia::estimate_radius_from_prop_optional(const TOptional<TDronePropeller>& prop_opt)
{
	// fallback: classic 5" wheelbase
	const auto fallback_radius = 0.5 * fallback_wheelbase_m;

	if (!prop_opt.IsSet())
	{
		return fallback_radius;
	}

	const TDronePropeller& propeller = prop_opt.GetValue();
	return match_variant(
		propeller,

		[fallback_radius](const FDronePropellerBemt& prop_bemt)
		{
			// prefer provided radius if positive
			if (prop_bemt.radius > 0.0)
			{
				const double prop_diameter = 2.0 * prop_bemt.radius;
	
				// true-X: adjacent motor spacing ≈ prop_diameter + gap, so r ≈ spacing / √2
				const double adjacent_spacing = prop_diameter + prop_disc_gap_m;
				return adjacent_spacing * DOUBLE_UE_INV_SQRT_2;
			}
			return fallback_radius;
		},

		[fallback_radius](const FDronePropellerSimplified& prop_simplified)
		{
			if (prop_simplified.blade_diameter > 0.0)
			{
				// true-X: adjacent motor spacing ≈ prop_diameter + gap, so r ≈ spacing / √2
				const double adjacent_spacing = prop_simplified.blade_diameter + prop_disc_gap_m;
				return adjacent_spacing * DOUBLE_UE_INV_SQRT_2;
			}
			return fallback_radius;
		}
	);
}
