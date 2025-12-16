#pragma once

#include "CoreMinimal.h"
#include "Structural.h"              // your structs

namespace inertia
{
	// ------------------------------------------------------------------
	// Tunable "good-enough" constants (SI units: meters, kilograms)
	// ------------------------------------------------------------------

	// Central box extents (around CoM) for a 5" freestyle quad
	inline constexpr double extent_x_m = 0.17;             // front-back
	inline constexpr double extent_y_m = 0.17;             // right-left
	inline constexpr double extent_z_m = 0.05;             // height

	// Geometry defaults
	inline constexpr double prop_disc_gap_m = 0.015;       // spacing between prop discs
	inline constexpr double fallback_wheelbase_m = 0.230;  // 230 mm → r = 0.115 m
	inline constexpr double default_prop_radius_m = 0.0635; // 2.5" = 0.0635 m (5" prop)

	// Mass defaults (typical 5" freestyle, ~600 g total)
	inline constexpr double default_motor_mass_kg    = 0.030; // per motor (≈30 g)
	inline constexpr double default_frame_mass_kg    = 0.300; // ≈300 g
	inline constexpr double default_battery_mass_kg  = 0.180; // ≈180 g (e.g., 6S ~1100–1300 mAh)
	inline constexpr double extra_tip_mass_total_kg  = 0.040; // props + arm ends

	// Arms modeling
	inline constexpr double k_arms_frac_of_frame = 0.35;
	inline constexpr bool   k_include_arms_rods  = true;

	// ------------------------------------------------------------------
	// Helpers
	// ------------------------------------------------------------------

	inline double safe_positive(double value, double fallback)
	{
		return (value > 0.0) ? value : fallback;
	}

	double DRONESIMULATORCORE_API estimate_radius_from_prop_optional(const TOptional<TDronePropeller>& prop_opt);

	inline FVector kgm2_to_kgcm2(const FVector& inertia_si)
	{
		return inertia_si * 10000.0f; // 1 m² = 10,000 cm²
	}

	// Extract mass contributions with per-field fallbacks
	struct FMassBreakdown
	{
		double mass_motors;     // total of the 4 motors
		double mass_frame;
		double mass_battery;
		double mass_extra_tips;
	};

	inline FMassBreakdown get_masses(const TOptional<FDroneFrame>& frame_opt, const TOptional<FDroneMotor>& motor_opt,
		const TOptional<FDroneBattery>& battery_opt)
	{
		const double motor_mass_each = motor_opt.IsSet() ? safe_positive(motor_opt->mass, default_motor_mass_kg) : default_motor_mass_kg;

		const double frame_mass = frame_opt.IsSet() ? safe_positive(frame_opt->mass, default_frame_mass_kg) : default_frame_mass_kg;

		const double battery_mass =
			battery_opt.IsSet() ? safe_positive(battery_opt->mass, default_battery_mass_kg) : default_battery_mass_kg;

		return {
			/*mass_motors*/ 4.0 * motor_mass_each,
			/*mass_frame*/  frame_mass,
			/*mass_battery*/battery_mass,
			/*mass_extra_tips*/ extra_tip_mass_total_kg
		};
	}

	// ------------------------------------------------------------------
	// Core computation (all params optional): diagonal inertia (kg·m²)
	// ------------------------------------------------------------------
	inline FVector compute_inertia_si(const TOptional<FDroneFrame>& frame_opt, const TOptional<FDroneMotor>& motor_opt,
		const TOptional<TDronePropeller>& prop_opt, const TOptional<FDroneBattery>& battery_opt)
	{
		// masses with smart fallbacks
		const FMassBreakdown mb = get_masses(frame_opt, motor_opt, battery_opt);
		const double mass_total = mb.mass_motors + mb.mass_frame + mb.mass_battery + mb.mass_extra_tips;

		if (mass_total <= 0.0)
		{
			return FVector::ZeroVector;
		}

		// 1) uniform central box (baseline)
		const double lx = extent_x_m;
		const double ly = extent_y_m;
		const double lz = extent_z_m;

		double i_xx = (1.0 / 12.0) * mass_total * (ly * ly + lz * lz);
		double i_yy = (1.0 / 12.0) * mass_total * (lx * lx + lz * lz);
		double i_zz = (1.0 / 12.0) * mass_total * (lx * lx + ly * ly);

		// 2) tips (motors + props + end-of-arms) at radius r
		const double r   = estimate_radius_from_prop_optional(prop_opt);
		const double r2  = r * r;
		const double mass_tips_total = mb.mass_motors + mb.mass_extra_tips;

		i_xx += 0.5 * mass_tips_total * r2;
		i_yy += 0.5 * mass_tips_total * r2;
		i_zz += 1.0 * mass_tips_total * r2;

		// 3) optional arms as slender rods (length ≈ 2r), from a fraction of frame mass
		if (k_include_arms_rods)
		{
			const double mass_arms = k_arms_frac_of_frame * mb.mass_frame;
			i_xx += (1.0 / 6.0) * mass_arms * r2;
			i_yy += (1.0 / 6.0) * mass_arms * r2;
			i_zz += (1.0 / 3.0) * mass_arms * r2;
		}

		return FVector(static_cast<float>(i_xx),
		               static_cast<float>(i_yy),
		               static_cast<float>(i_zz));
	}

	// Convenience: UE units (kg·cm²)
	inline FVector compute_inertia_uu(const TOptional<FDroneFrame>& frame_opt, const TOptional<FDroneMotor>& motor_opt,
		const TOptional<TDronePropeller>& prop_opt, const TOptional<FDroneBattery>& battery_opt)
	{
		const FVector inertia_si = compute_inertia_si(frame_opt, motor_opt, prop_opt, battery_opt);
		return kgm2_to_kgcm2(inertia_si);
	}
}
