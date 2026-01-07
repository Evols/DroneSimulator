#pragma once

#include "CoreMinimal.h"

#include "SubstepBody.generated.h"

USTRUCT()
struct DRONESIMULATORCORE_API FSubstepBody
{
    GENERATED_BODY()

// These properties are set at initialization, then should not be updated during the simulation step.
public:

    // Transform of the body instance, without the scale, all in unreal units.
    UPROPERTY()
    FTransform transform_world;

    // Mass in kg
    UPROPERTY()
    double mass = 0.0;

    // In kg·m^2, in local space.
    // These are the diagonal coefficients of the matrix, because in local space the matrix is diagonal
    UPROPERTY()
    FVector inertia_tensor = FVector::ZeroVector;

// These properties are updated at the end of each substep, but should not be updated during the substep.
public:

    // In m/s
    UPROPERTY()
    FVector linear_velocity_world = FVector::ZeroVector;

    // Angular velocity, as a rotation vector:
    // - the direction of this vector is its axis of rotation
    // - its magnitude is the rotation speed in rad/s
    UPROPERTY()
    FVector angular_velocity_radians_world = FVector::ZeroVector;

// These properties are accumulated during the substep.
public:

    // In N (kg·m/s^2)
    UPROPERTY()
    FVector accumulated_force_world = FVector::ZeroVector;

    // In N·m·rad (rad·kg·m^2/s^2), as a rotation vector
    UPROPERTY()
    FVector accumulated_torque_world = FVector::ZeroVector;

public:
    /**
     * Adds a force in Newtons (kg·m/s^2) at the center of gravity
     * @param force Force to apply at the center of gravity, in Newtons, in world space
     */
    void add_force(const FVector& force);

    /**
     * Adds a torque in N·m (kg·m^2/s^2) at the center of the object
     * @param torque Torque in N·m to apply at the center of the object, as a rotation vector in world space
     * (rotation axis is the direction of the vector, torque value in N·m is the magnitude of the vector)
     */
    void add_torque(const FVector& torque);

    // force_world in N (kg·m/s^2), location_local in unreal units
    void add_force_at_point(const FVector& force_world, const FVector& point_location_local);

    /**
     * Consumes the forces to apply them to the linear and angular velocities, then resets the accumulators.
     */
    void consume_forces_and_torques(double substep_delta_time);

	FVector get_gyroscopic_torque_local() const;
	
public:

    // location_local is in unreal units, output is in m/s
    FVector get_velocity_at_location(const FVector& location_local) const;

public:

    FSubstepBody() = default;

    FSubstepBody(const FVector& location_world, const FQuat& rotation_world, double in_mass, const FVector& in_inertia_tensor,
        const FVector& in_linear_velocity_world, const FVector& in_angular_velocity_radians_world);

    static FSubstepBody from_body_instance(const FBodyInstance* body_instance);
};
