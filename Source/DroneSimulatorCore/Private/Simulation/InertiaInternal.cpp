#include "DroneSimulatorCore/Private/Simulation/InertiaInternal.h"

double inertia::uu_to_meters(double uu)
{
	return uu * 0.01;
}

FVector inertia::kgm2_to_kgcm2(const FVector& inertia_si)
{
	return inertia_si * 10000.0; // 1 m² = 10,000 cm²
}

FVector inertia::compute_box_inertia(double mass, const FVector& extent)
{
	if (mass <= 0.0)
	{
		return FVector::ZeroVector;
	}

	const double x = extent.X;
	const double y = extent.Y;
	const double z = extent.Z;
	return FVector(
		(1.0 / 3.0) * mass * (y * y + z * z),
		(1.0 / 3.0) * mass * (x * x + z * z),
		(1.0 / 3.0) * mass * (x * x + y * y)
	);
}

FVector inertia::compute_point_mass_inertia(double mass, const FVector& offset)
{
	if (mass <= 0.0) return FVector::ZeroVector;
	
	const double x2 = offset.X * offset.X;
	const double y2 = offset.Y * offset.Y;
	const double z2 = offset.Z * offset.Z;
	
	return FVector(
		mass * (y2 + z2),
		mass * (x2 + z2),
		mass * (x2 + y2)
	);
}

FVector inertia::compute_rotated_box_inertia(double mass, const FVector& extent, const FRotator& rotation)
{
	const FVector I_diag = compute_box_inertia(mass, extent);

	// The inertia tensor I is diagonal in local space: diag(I_diag.X, I_diag.Y, I_diag.Z).
	// When rotated by R, the new tensor is R * I * R^T.
	// We only need the diagonal elements of the resulting matrix.
	
	const FQuat quat = rotation.Quaternion();
	const FVector axis_x = quat.GetAxisX();
	const FVector axis_y = quat.GetAxisY();
	const FVector axis_z = quat.GetAxisZ();

	return FVector(
		I_diag.X * axis_x.X * axis_x.X + I_diag.Y * axis_y.X * axis_y.X + I_diag.Z * axis_z.X * axis_z.X,
		I_diag.X * axis_x.Y * axis_x.Y + I_diag.Y * axis_y.Y * axis_y.Y + I_diag.Z * axis_z.Y * axis_z.Y,
		I_diag.X * axis_x.Z * axis_x.Z + I_diag.Y * axis_y.Z * axis_y.Z + I_diag.Z * axis_z.Z * axis_z.Z
	);
}

FVector inertia::compute_frame_inertia_primitive(double mass_frame, const FVector& front_extent, const FVector& back_extent)
{
	const auto average_extent = (front_extent.GetAbs() + back_extent.GetAbs()) * 0.5;
	
	// Reference frame that we know (a Kayouloin)
	const auto reference_frame_box_size = FVector(0.2, 0.045, 0.03);
	const auto frame_box_mass_fraction = 0.5;

	// Compute the inertia of the central box
	const auto front_back_box_size = average_extent.X * 2.0;
	const auto frame_box_size = reference_frame_box_size * (front_back_box_size / reference_frame_box_size.X);
	const auto frame_box_mass = frame_box_mass_fraction * mass_frame;

	auto inertia = compute_box_inertia(frame_box_mass, frame_box_size * 0.5);

	// Compute the inertia of the arms
	const auto reference_arm_size = FVector(0.16, 0.015, 0.008);

	const auto arm_length = (front_extent.Length() + back_extent.Length()) * 0.5;
	const auto arm_size = reference_arm_size * (arm_length / reference_arm_size.X);
	const auto arm_mass = (1.0 - frame_box_mass_fraction) * mass_frame * 0.25;

	const auto yaw = FMath::RadiansToDegrees(FMath::Atan2(average_extent.Y, average_extent.X));
	
	// Two diagonal arms, each carrying half the arms mass
	inertia += compute_rotated_box_inertia(arm_mass * 2.0, arm_size * 0.5, FRotator(0, 90.0 - yaw, 0));
	inertia += compute_rotated_box_inertia(arm_mass * 2.0, arm_size * 0.5, FRotator(0, 90.0 + yaw, 0));

	return inertia;
}

FVector inertia::compute_motors_inertia_primitive(double mass_motors, const FVector& front_extent, const FVector& back_extent)
{
	return (
		compute_point_mass_inertia(mass_motors, front_extent) +
		compute_point_mass_inertia(mass_motors, front_extent * FVector(1.0, -1.0, 1.0)) +
		compute_point_mass_inertia(mass_motors, back_extent) +
		compute_point_mass_inertia(mass_motors, back_extent * FVector(1.0, -1.0, 1.0))
	);
}

FVector inertia::compute_battery_inertia_primitive(double mass_battery)
{
	// OVONIC 6S 100C 1000mAh is 75 * 36 * 35 mm and 171g
	const auto reference_battery_mass = 0.171;
	const auto reference_battery_size = FVector(0.075, 0.036, 0.035);

	const double battery_size_scale = FMath::Pow(mass_battery / reference_battery_mass, 1.0 / 3.0);
	const FVector extent = (battery_size_scale * reference_battery_size) * 0.5;

	return compute_box_inertia(mass_battery, extent);
}
