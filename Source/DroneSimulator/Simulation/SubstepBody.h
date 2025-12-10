#pragma once

#include "CoreMinimal.h"

#include "SubstepBody.generated.h"

USTRUCT()
struct FSubstepBody
{
    GENERATED_BODY()

// These properties are set at initialization, then should not be updated during the simulation step.
public:

    // Transform of the body instance, without the scale, all in unreal units.
    UPROPERTY()
    FTransform transform_world;

    // Mass in kg
    UPROPERTY()
    double mass;

    // In kg·m^2
    UPROPERTY()
    FVector inertia_tensor;

// These properties are updated at the end of each substep, but should not be updated during the substep.
public:

    // In m/s
    UPROPERTY()
    FVector linear_velocity_world;

    // Angular velocity, as a rotation vector:
    // - the direction of this vector is its axis of rotation
    // - its magnitude is the rotation speed in rad/s
    UPROPERTY()
    FVector angular_velocity_radians_world;

// These properties are accumulated during the substep.
public:

    // In N (kg·m/s^2)
    UPROPERTY()
    FVector accumulated_force_world;

    // In N·m·rad (rad·kg·m^2/s^2), as a rotation vector
    UPROPERTY()
    FVector accumulated_torque_world;

public:

    // Force in N (kg·m/s^2)
    void add_force(const FVector& force);

    // Torque in N·m (kg·m^2/s^2), as a rotation vector
    void add_torque(const FVector& torque);

    // force_world in N (kg·m/s^2), location_local in unreal units
    void add_force_at_point(const FVector& force_world, const FVector& point_location_local);

    /**
     * Consumes the forces to apply them to the linear and angular velocities, then resets the accumulators.
     */
    void consume_forces_and_torques(double substep_delta_time);

public:

    // location_local is in unreal units, output is in m/s
    FVector get_velocity_at_location(const FVector& location_local) const;

public:

    FSubstepBody() = default;

    FSubstepBody(const FVector& location_world, const FQuat& rotation_world, double in_mass, const FVector& in_inertia_tensor,
        const FVector& in_linear_velocity_world, const FVector& in_angular_velocity_radians_world);

    static FSubstepBody from_body_instance(const FBodyInstance* body_instance);
};
