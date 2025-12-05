#pragma once

#include "CoreMinimal.h"

namespace math
{
	/**
	 * @see rotator_to_euler_rad
	 */
	FVector delta_rotation_to_angular_velocity(const FRotator& delta_rotation, double delta_time);

	/**
	 * Converts an FRotator to an Euler/Tait–Bryan rotation vector, in radians.
	 *
	 * To be consistent with Unreal's rotators and euler angles:
	 * - X (roll) is clockwise when viewed from the front (up goes right)
	 * - Y (pitch) is clockwise when viewed from the right (forward goes up)
	 * - Z (yaw) is clockwise when viewed from above (forward goes right)
	 *
	 * @param rotator Rotator
	 * @return Euler/Tait–Bryan, rotation in radians
	 */
	FVector rotator_to_euler_rad(const FRotator& rotator);

	constexpr double rpm_to_rad_per_sec(double rpm)
	{
		return rpm * (TWO_PI / 60.0);
	}

	/**
	 * Rotates a torque vector
	 *
	 * @param rotation
	 * @param torque Torque vector, in N.m, oriented like unreal's units. Note that torque in N.m is equivalent to N.m.rad
	 * @return
	 */
	FVector rotate_torque_vector(const FQuat& rotation, const FVector& torque);
}
