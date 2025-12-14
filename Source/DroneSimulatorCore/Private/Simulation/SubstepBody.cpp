#include "DroneSimulatorCore/Public/Simulation/SubstepBody.h"


void FSubstepBody::add_force(const FVector& force)
{
    this->accumulated_force_world += force;
}

void FSubstepBody::add_torque(const FVector& torque)
{
    this->accumulated_torque_world += torque;
}

void FSubstepBody::add_force_at_point(const FVector& force_world, const FVector& point_location_local)
{
    const Chaos::FVec3 point_location_world = this->transform_world.TransformPosition(point_location_local);
    const Chaos::FVec3 torque_world = Chaos::FVec3::CrossProduct((point_location_world - this->transform_world.GetLocation()) / 100.0, force_world);

    this->accumulated_force_world += force_world;
    this->accumulated_torque_world += torque_world;
}

void FSubstepBody::consume_forces_and_torques(double substep_delta_time)
{
	const auto rotation_world = this->transform_world.GetRotation();
	const auto accumulated_torque_local = rotation_world.UnrotateVector(this->accumulated_torque_world);
	const auto angular_velocity_acceleration_local = (accumulated_torque_local - get_gyroscopic_torque_local()) / this->inertia_tensor;
	const auto angular_velocity_acceleration_world = rotation_world.RotateVector(angular_velocity_acceleration_local);

	this->linear_velocity_world += (this->accumulated_force_world / this->mass) * substep_delta_time;
	this->angular_velocity_radians_world += angular_velocity_acceleration_world * substep_delta_time;

	this->accumulated_force_world = FVector::ZeroVector;
	this->accumulated_torque_world = FVector::ZeroVector;
}

FVector FSubstepBody::get_gyroscopic_torque_local() const
{
	const auto rotation_world = this->transform_world.GetRotation();

	// Convert to body space where the inertia tensor is diagonal
	const FVector omega_body = rotation_world.UnrotateVector(this->angular_velocity_radians_world);

	// Euler rigid-body equation (body frame): I * ω_dot = τ - ω × (Iω)
	// Gyroscopic term: G = ω × (Iω); then ω_dot = (τ - G) / I (component-wise because I is diagonal in body frame)
	return FVector::CrossProduct(omega_body, this->inertia_tensor * omega_body);
}

FVector FSubstepBody::get_velocity_at_location(const FVector& location_local) const
{
    const FVector location_world = this->transform_world.GetRotation().RotateVector(location_local);
    const FVector rotational_velocity = FVector::CrossProduct(this->angular_velocity_radians_world, location_world / 100.0);
    return this->linear_velocity_world + rotational_velocity;
}

FSubstepBody::FSubstepBody(const FVector& location_world, const FQuat& rotation_world, double in_mass, const FVector& in_inertia_tensor,
                           const FVector& in_linear_velocity_world, const FVector& in_angular_velocity_radians_world)
    : transform_world(FTransform(rotation_world, location_world))
    , mass(in_mass)
    , inertia_tensor(in_inertia_tensor)
    , linear_velocity_world(in_linear_velocity_world)
    , angular_velocity_radians_world(in_angular_velocity_radians_world)
    , accumulated_force_world(FVector::ZeroVector)
    , accumulated_torque_world(FVector::ZeroVector)
{
}

FSubstepBody FSubstepBody::from_body_instance(const FBodyInstance* body_instance)
{
    const auto transform = body_instance->GetUnrealWorldTransform();
    return FSubstepBody(
        transform.GetLocation(),
        transform.GetRotation(),
        body_instance->GetBodyMass(),
        body_instance->GetBodyInertiaTensor() / 10000.0,
        body_instance->GetUnrealWorldVelocity() / 100.0,
        body_instance->GetUnrealWorldAngularVelocityInRadians()
    );
}
