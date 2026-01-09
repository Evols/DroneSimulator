#include "DroneSimulatorCore/Public/Simulation/Inertia.h"

#include "InertiaInternal.h"
#include "Utils/Variant.h"

// ------------------------------------------------------------------
// Tunable "good-enough" constants (SI units: meters, kilograms)
// ------------------------------------------------------------------

// Central box half-extents (distance from CoM to side) for a 5" freestyle quad, in meters
// This represents a ~10cm x 10cm x 5cm main body box
FVector default_extent = FVector(0.05, 0.05, 0.0);

// Geometry defaults
constexpr double default_prop_radius = 0.0635; // // 5 inch diameter = 12.7 cm diameter = 6.35 cm

// Mass defaults (typical 5" freestyle, ~600 g total), in kg
constexpr double default_motor_mass = 0.030; // per motor (≈30 g)
constexpr double default_frame_mass = 0.3; // ≈300 g
constexpr double default_battery_mass = 0.18; // ≈180 g (e.g., 6S ~1100–1300 mAh)

double get_radius_of_prop_optional(const TOptional<TDronePropeller>& prop_opt)
{
	if (!prop_opt.IsSet())
	{
		return default_prop_radius;
	}

	const TDronePropeller& propeller = prop_opt.GetValue();
	return match_variant(
		propeller,

		[](const FDronePropellerBemt& prop_bemt)
		{
			return prop_bemt.radius;
		},

		[](const FDronePropellerSimplified& prop_simplified)
		{
			return prop_simplified.blade_diameter * 0.5;
		}
	);
}

double estimate_radius_from_frame_optional(const TOptional<FDroneFrame>& frame_opt)
{
	if (frame_opt.IsSet())
	{
		const double dist_front = frame_opt->props_extent_front.Size2D();
		const double dist_back = frame_opt->props_extent_back.Size2D();
		const double r_uu = (dist_front + dist_back) * 0.5;
		if (r_uu > 1.0) // Expecting non-zero dimension
		{
			return r_uu * 0.01;
		}
	}
	return 0.0;
}

FVector get_frame_half_extents(const TOptional<FDroneFrame>& frame_opt)
{
	if (!frame_opt.IsSet())
	{
		return default_extent;
	}

	// Distance from center to motors in meters
	return (frame_opt->props_extent_front.GetAbs() + frame_opt->props_extent_back.GetAbs()) * 0.5 * 0.01;
}

FVector inertia::compute_frame_inertia(const TOptional<FDroneFrame>& frame_opt)
{
	const double mass_frame = frame_opt.IsSet() ? frame_opt->mass : default_frame_mass;
	const auto front_extent = frame_opt.IsSet() ? frame_opt->props_extent_front : default_extent;
	const auto back_extent = frame_opt.IsSet() ? frame_opt->props_extent_back : (default_extent * FVector(-1.0, 0.0, 0.0));
	return compute_frame_inertia_primitive(mass_frame, front_extent, back_extent);
}

FVector inertia::compute_motors_inertia(const TOptional<FDroneMotor>& motor_opt, const TOptional<FDroneFrame>& frame_opt)
{
	const double motor_mass = motor_opt.IsSet() ? motor_opt->mass : default_motor_mass;
	const auto front_extent = frame_opt.IsSet() ? frame_opt->props_extent_front : default_extent;
	const auto back_extent = frame_opt.IsSet() ? frame_opt->props_extent_back : (default_extent * FVector(-1.0, 0.0, 0.0));
	return compute_motors_inertia_primitive(motor_mass, front_extent, back_extent);
}

FVector inertia::compute_battery_inertia(const TOptional<FDroneBattery>& battery_opt)
{
	const double battery_mass = battery_opt.IsSet() ? battery_opt->mass : default_battery_mass;
	return compute_battery_inertia_primitive(battery_mass);
}

FVector inertia::compute_inertia_si(const TOptional<FDroneFrame>& frame_opt, const TOptional<FDroneMotor>& motor_opt,
	const TOptional<FDroneBattery>& battery_opt)
{
	// Neglect propeller

	return (
		compute_frame_inertia(frame_opt) +
		compute_motors_inertia(motor_opt, frame_opt) +
		compute_battery_inertia(battery_opt)
	);
}

FVector inertia::compute_inertia_uu(const TOptional<FDroneFrame>& frame_opt, const TOptional<FDroneMotor>& motor_opt,
	const TOptional<FDroneBattery>& battery_opt)
{
	const FVector inertia_si = compute_inertia_si(frame_opt, motor_opt, battery_opt);
	UE_LOG(LogTemp, Log, TEXT("inertia_si: X=%.5f Y=%.5f Z=%.5f"), inertia_si.X, inertia_si.Y, inertia_si.Z);
	return kgm2_to_kgcm2(inertia_si);
}
