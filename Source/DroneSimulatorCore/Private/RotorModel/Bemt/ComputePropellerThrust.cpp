#include "DroneSimulatorCore/Public/RotorModel/Bemt/ComputePropellerThrust.h"
#include "DroneSimulatorCore/Public/RotorModel/Bemt/AirfoilCoefficients.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"


/**
 * Gets the pitch (in rad) at a given radius (in meters)
 */
double get_pitch_angle_at_radius(double radius, const FDronePropellerBemt* propeller)
{
	const double pitch = FMath::Atan2(propeller->pitch, radius * UE_DOUBLE_TWO_PI);
	return pitch;
}

constexpr int32 blade_elements_count = 5;

constexpr int32 integrations = 6;
constexpr double integration_relaxation = 0.8;

constexpr int32 a_prime_integrations = 4;
constexpr double a_prime_relaxation = 0.7;

struct FIntegrationResult
{
	double thrust = 0.0; // In Newtons
	double torque = 0.0; // In N.m
	double angle_of_attack = 0.0; // In radians
	TArray<double> reynolds = {};

	FDebugLog debug_log;

	FIntegrationResult() = default;

	FIntegrationResult(double in_thrust, double in_torque, double in_angle_of_attack, const TArray<double>& in_reynolds, const FDebugLog& in_debug_log)
		: thrust(in_thrust), torque(in_torque), angle_of_attack(in_angle_of_attack), reynolds(in_reynolds), debug_log(in_debug_log)
	{
	}
};

static double compute_prandtl_factor(int32 B, double r, double R, double Rhub, double phi)
{
	const double sinphi = FMath::Max(1e-6, FMath::Sin(FMath::Abs(phi)));

	// Tip and root factors
	const double f_tip  = (B * 0.5) * (R - r)   / (r * sinphi);
	const double f_root = (B * 0.5) * (r - Rhub)/ (r * sinphi);

	const double F_tip  = (2.0/PI) * FMath::Acos(FMath::Clamp(FMath::Exp(-f_tip),  0.0, 1.0));
	const double F_root = (2.0/PI) * FMath::Acos(FMath::Clamp(FMath::Exp(-f_root), 0.0, 1.0));

	// Combine & clamp for numerical safety
	return FMath::Clamp(F_tip * F_root, 1e-3, 1.0);
}

/**
 * @param v_axial Axial velocity in the airflow tube, in m/s. Positive when air velocity is downstream.
 * @param v_induced Estimate of the velocity induced by the propeller disk, in m/s. Positive when air velocity is downstream.
 * @param propeller Properties of the propeller
 * @param air_density Air density, in kg/m^3
 * @param propeller_angular_speed Angular velocity of the propeller, in rad/s
 * @return
 */
FIntegrationResult integrate_with_v_induced(double v_axial, double v_induced, const FDronePropellerBemt* propeller,
	double air_density, double propeller_angular_speed)
{
	FDebugLog debug_log;

	double total_thrust = 0.0, total_torque = 0.0;

	const double element_width = (propeller->radius - propeller->hub_radius) / static_cast<double>(blade_elements_count);
	double angle_of_attack_accumulator = 0.0;

	// Axial component at the disk (global for this pass; local a' inside)
	const double Vx_disk = v_axial + v_induced; // downstream-positive

	const double representative_index = FMath::RoundToInt32(0.7 * blade_elements_count);

	TArray<double> reynolds_sections;

	for (int i = 0; i < blade_elements_count; ++i)
	{
		// 1/2 offset quadrature
		const double element_radius = propeller->hub_radius + (i + 0.5) * element_width;

		// Solve local tangential induction a'(r) with a few relaxed iterations
		double a_prime = 0.0;
		for (int k = 0; k < a_prime_integrations; ++k)
		{
			const double Vtheta = propeller_angular_speed * element_radius * (1.0 + a_prime);
			const double wind_speed = FMath::Sqrt(FMath::Square(Vx_disk) + FMath::Square(Vtheta));
			const double inflow_angle = FMath::Atan2(Vx_disk, Vtheta); // inflow angle, often named phi

			const double theta_b = get_pitch_angle_at_radius(element_radius, propeller);
			const double aoa = theta_b - inflow_angle;

			// Aerodynamics

			constexpr double kinematic_viscosity = 1.5e-5; // m^2/s
			const double reynolds = (wind_speed * propeller->chord) / kinematic_viscosity;

			const auto coefficients_result = simulation_bemt::interpolate_airfoil_coefficients(reynolds, aoa, propeller->airfoil);

			// If interpolation failed, use sensible defaults
			const auto coefficients = coefficients_result.Get(FAirfoilCoefficients::get_sensible_defaults());

			const auto lift_coefficient = coefficients.lift;
			const auto drag_coefficient = coefficients.drag;

			const double dynamic_pressure = 0.5 * air_density * wind_speed * wind_speed;

			// Per-blade element forces
			const double dL = dynamic_pressure * propeller->chord * lift_coefficient * element_width;
			const double dD = dynamic_pressure * propeller->chord * drag_coefficient * element_width;

			// Resolve to thrust/torque (sum over blades)
			const double s = FMath::Sin(inflow_angle);
			const double c = FMath::Cos(inflow_angle);

			const double dT_BE = propeller->num_blades * (dL * c - dD * s);
			const double dQ_BE = propeller->num_blades * (dL * s + dD * c) * element_radius;

			// Prandtl factor
			const double prandtl_factor = compute_prandtl_factor(propeller->num_blades, element_radius, propeller->radius, propeller->hub_radius, inflow_angle);

			// Momentum torque model: dQ_MT = 4πρ F r^3 Vx Ω a' dr  =>  a' = dQ_BE / (4πρ F r^3 Vx Ω dr)
			const double denom = 4.0 * PI * air_density * prandtl_factor * FMath::Max(1e-6, Vx_disk)
				* propeller_angular_speed * element_radius * element_radius * element_radius * FMath::Max(1e-9, element_width);

			const double a_prime_new = FMath::Clamp(dQ_BE / denom, -0.5, 0.5);

			// Light relaxation for stability
			a_prime = a_prime_relaxation * a_prime + (1.0 - a_prime_relaxation) * a_prime_new;

			// Optional: early exit if converged
			if (FMath::Abs(a_prime_new - a_prime) < 1e-4)
			{
				break;
			}
		}

		// Final pass to accumulate loads with converged a'
		const double Vtheta = propeller_angular_speed * element_radius * (1.0 + a_prime);
		const double wind_speed = FMath::Sqrt(FMath::Square(Vx_disk) + FMath::Square(Vtheta));
		const double inflow_angle = FMath::Atan2(Vx_disk, Vtheta);  // inflow angle, often named phi

		const double element_pitch_angle = get_pitch_angle_at_radius(element_radius, propeller);
		const double angle_of_attack = element_pitch_angle - inflow_angle;
		angle_of_attack_accumulator += angle_of_attack;

		// Calculate Reynolds number: Re = (density * velocity * chord) / dynamic_viscosity
		// For air at sea level, kinematic viscosity is approximately 1.5e-5 m^2/s
		constexpr double kinematic_viscosity = 1.5e-5; // m^2/s
		const double reynolds = (wind_speed * propeller->chord) / kinematic_viscosity;

		const auto coefficients_result = simulation_bemt::interpolate_airfoil_coefficients(reynolds, angle_of_attack, propeller->airfoil);

		// If interpolation failed, use sensible defaults
		const auto coefficients = coefficients_result.Get(FAirfoilCoefficients::get_sensible_defaults());

		const double lift_coefficient = coefficients.lift;
		const double drag_coefficient = coefficients.drag;

		// debug_log.log(FString::Printf(TEXT("aoa=%.3f cl=%f cd=%f"), FMath::RadiansToDegrees(angle_of_attack), lift_coefficient, drag_coefficient));

		const double dynamic_pressure  = 0.5 * air_density * wind_speed * wind_speed;
		const double element_lift = dynamic_pressure * propeller->chord * lift_coefficient * element_width;
		const double element_drag = dynamic_pressure * propeller->chord * drag_coefficient * element_width;

		const double inflow_angle_sin = FMath::Sin(inflow_angle);
		const double inflow_angle_cos = FMath::Cos(inflow_angle);

		const double element_thrust = propeller->num_blades * (element_lift * inflow_angle_cos - element_drag * inflow_angle_sin);
		const double element_torque = propeller->num_blades * (element_lift * inflow_angle_sin + element_drag * inflow_angle_cos) * element_radius;

		total_thrust += element_thrust;
		total_torque += element_torque;

		if (i == representative_index)
		{
			debug_log.log(FString::Printf(TEXT("i=%d -> aoa_d=%f"), i, FMath::RadiansToDegrees(angle_of_attack)));
		}

		reynolds_sections.Add(reynolds);
	}

	const double average_angle_of_attack = angle_of_attack_accumulator / blade_elements_count;

	return FIntegrationResult(total_thrust, total_torque, average_angle_of_attack, reynolds_sections, debug_log);
}

double simulation_bemt::compute_axial_velocity(const FVector& thrust_axis, const FVector& wind_velocity,
	const FVector& propeller_velocity)
{
	const auto freestream_velocity = wind_velocity - propeller_velocity;
	return freestream_velocity.Dot(-thrust_axis);
}

double compute_induced_velocity_from_thrust(double thrust, double v_axial, double air_density, double area)
{
	// Guard against badly configured values
	if (air_density <= 0.0 || area <= 0.0)
	{
		return 0.0;
	}

	/*
	 * Newtons' second law:
	 * F = d momentum / dt (derivative of momentum)
	 * F = thrust
	 * d momentum / dt = ??? = mass flow * (v downstream - v upstream)
	 * thrust = mass flow * (v downstream - v upstream)
	 *
	 * To determine mass flow:
	 * mass flow = air density * area * air velocity in disk
	 * air velocity in disk = v axial + v induced
	 * mass flow = air density * area * (v axial + v induced)
	 *
	 * To determine delta velocity:
	 * v upstream = v axial ; v downstream = v axial + 2 * v induced
	 * v downstream - v upstream = v axial + 2 * v induced - v axial = 2 * v induced
	 *
	 * Reintegrating into Newton's formula:
	 * thrust = air density * area * (v axial + v induced) * 2 * v induced
	 * thrust = 2 * v induced * air density * area * v axial + 2 * v induced ^ 2 * air density * area
	 *
	 * we know that air density ≠ 0 and area ≠ 0, so we can divide both sides by 2 * air density * area
	 * thrust / (2 * air density * area) = v induced * v axial + v induced ^ 2
	 * v induced ^ 2 + v induced * v axial - thrust / (2 * air density * area) = 0
	 *
	 * This is a polynomial of the 2nd degree for v induced, with 2 solutions
	 * a * v induced ^ 2 + b * v induced + c = 0
	 * a = 1, b = v axial, c = -thrust / (2 * air density * area)
	 * Its discriminant is
	 * delta = b ^ 2 - 4 * a * c = v axial + 2 * thrust / (air density * area)
	 * and its roots are
	 * r1 = (-b + sqrt(delta)) / (2 * a)
	 * r1 = 0.5 * (-v axial + sqrt(delta))
	 * r2 = (-b - sqrt(delta)) / (2 * a)
	 * r2 = 0.5 * (-v axial - sqrt(delta))
	 *
	 * From the theory, r1 is the good one
	 */

	const double discriminant = v_axial * v_axial + 2.0 * thrust / (air_density * area);

	if (discriminant < 0.0)
	{
		return -0.5 * v_axial; // complex roots, return something reasonable
	}

	const auto discriminant_root = FMath::Sqrt(discriminant);
	const auto root_1 = 0.5 * (-v_axial + discriminant_root);
	const auto root_2 = 0.5 * (-v_axial - discriminant_root);

	return v_axial >= 0.0 ? root_2 : root_1;
}

TTuple<FPropThrustResult, FDebugLog> simulation_bemt::compute_thrust_and_torque(double propeller_angular_speed, const FVector& thrust_axis,
	const FVector& wind_velocity, const FVector& propeller_velocity, double air_density,
	const FDronePropellerBemt* propeller)
{
	FDebugLog debug_log;

	if (propeller->radius <= propeller->hub_radius || propeller->num_blades <= 0 || FMath::IsNearlyZero(propeller_angular_speed, 1e-3))
	{
		return TTuple<FPropThrustResult, FDebugLog> { FPropThrustResult(), FDebugLog() };
	}

	// v_axial is the free-stream velocity of the air, far away from the propeller in the air tube
	// Downstream-positive, which means that v_axial is positive when air velocity goes toward the propeller from above the propeller
	const double v_axial = compute_axial_velocity(thrust_axis, wind_velocity, propeller_velocity);

	// Disk area
	const double area = PI * propeller->radius * propeller->radius;

	// Fixed-point one-iteration induced inflow (momentum theory): T ≈ 2*rho*A*vi*(Vaxial + vi)
	// We'll refine vi after the first blade-element pass. Start with external inflow + body axial flow.
	double v_induced = 0.0; // start guess (>=0, into disk)

	FIntegrationResult last_integration_result;

	for (int32 i = 0; i < integrations; i += 1)
	{
		last_integration_result = integrate_with_v_induced(v_axial, v_induced, propeller, air_density, propeller_angular_speed);
		debug_log.append_debug_log(last_integration_result.debug_log);

		const double thrust = last_integration_result.thrust;

		// Update v_induced from momentum (into disk, non-negative)

		const double new_v_induced = compute_induced_velocity_from_thrust(thrust, v_axial, air_density, area);

		v_induced = (1.0 - integration_relaxation) * v_induced + integration_relaxation * new_v_induced;
	}

	// Direction with Omega: if you reverse spin, thrust still points along +Axis (for positive pitch),
	// but our simple model above already accounts for sign via phi/Vtan. Keep it as computed.

	return TTuple<FPropThrustResult, FDebugLog> {
		FPropThrustResult {
			last_integration_result.thrust,
			last_integration_result.torque, // magnitude; sign applied at call site using sign(Omega)
			last_integration_result.angle_of_attack,
			last_integration_result.reynolds,
			v_induced,
			v_axial,
		},
		debug_log
	};
}
