#include "DroneSimulatorGame/Gameplay/DroneMovementComponent.h"
#include "DroneSimulatorGame/Assets/Conversion.h"
#include "DroneSimulatorGame/Gameplay/DronePawn.h"
#include "DroneSimulatorCore/Public/PropulsionModel/PropulsionModel.h"
#include "DroneSimulatorCore/Public/Simulation/Inertia.h"
#include "DroneSimulatorCore/Public/Simulation/LinearDrag.h"
#include "DroneSimulatorCore/Public/Simulation/RotationalDrag.h"
#include "DroneSimulatorCore/Public/Controller/FlightModeAir.h"
#include "DroneSimulatorCore/Public/Simulation/SimulationWorld.h"
#include "DroneSimulatorCore/Public/Simulation/SubstepBody.h"
#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include "DroneSimulatorInput/Public/DroneInputTypes.h"

UDroneMovementComponent::UDroneMovementComponent()
{
	calculate_custom_physics_delegate = FCalculateCustomPhysics::CreateUObject(
		this, &UDroneMovementComponent::calculate_custom_physics);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UDroneMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	this->init_drone_parts();
	this->set_updated_component_mass();
	this->set_updated_component_inertia();
	this->ensure_default_flight_mode();
}

void UDroneMovementComponent::TickComponent(float delta_time, ELevelTick tick_type, FActorComponentTickFunction* this_tick_function)
{
	Super::TickComponent(delta_time, tick_type, this_tick_function);

	this->player_input = FDronePlayerInput::zero();

	this->ensure_default_flight_mode();
	auto* active_mode = this->get_active_flight_mode();

	this->player_input = this->read_player_input(active_mode);
	this->controller_input = this->player_input;

	const auto flight_state = this->build_flight_mode_state(delta_time);
	this->angular_velocity = flight_state.angular_velocity_world;

	this->setpoint = active_mode != nullptr
		? active_mode->compute_setpoint(this->player_input, flight_state)
		: FDroneSetpoint(0.0, FVector::ZeroVector);

	const auto vertical_speed = flight_state.linear_velocity_world.Z * 3.6;
	const auto horizontal_speed = flight_state.linear_velocity_world.Size2D() * 3.6;

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Speed (km/h): Vertical=%.1f - Horizontal=%.1f"), vertical_speed, horizontal_speed));

	this->enqueue_custom_physics();
}

void UDroneMovementComponent::set_updated_component_mass()
{
	auto* primitive_component = get_primitive_component();
	if (primitive_component == nullptr)
	{
		return;
	}

	const double frame_mass = this->frame.IsSet() ? this->frame.GetValue().mass : 0.0;
	const double battery_mass = this->battery.IsSet() ? this->battery.GetValue().mass : 0.0;
	const double motor_mass = this->motor.IsSet() ? this->motor.GetValue().mass : 0.0;

	const double total_mass = frame_mass + battery_mass + 4.0 * motor_mass;

	primitive_component->SetMassOverrideInKg(NAME_None, total_mass);
}

void UDroneMovementComponent::set_updated_component_inertia()
{
	auto* primitive_component = get_primitive_component();
	if (primitive_component == nullptr)
	{
		return;
	}

	const auto inertia = inertia::compute_inertia_uu(this->frame, this->motor, this->propeller, this->battery);

	auto physics_handle = primitive_component->GetBodyInstance()->GetPhysicsActorHandle();
	FPhysicsCommand::ExecuteWrite(physics_handle, [inertia](auto handle)
	{
		const Chaos::FVec3 inertia_chaos(inertia.X, inertia.Y, inertia.Z);
		FPhysicsInterface::SetMassSpaceInertiaTensor_AssumesLocked(handle, inertia_chaos);
	});
}

void UDroneMovementComponent::init_drone_parts()
{
	this->frame = this->frame_asset == nullptr ? TOptional<FDroneFrame>() : conversion::convert_frame_asset(this->frame_asset);
	this->motor = this->motor_asset == nullptr ? TOptional<FDroneMotor>() : conversion::convert_motor_asset(this->motor_asset);
	this->battery = this->battery_asset == nullptr ? TOptional<FDroneBattery>() : conversion::convert_battery_asset(this->battery_asset);
	this->propeller = this->propeller_asset == nullptr ? TOptional<TDronePropeller>() : conversion::convert_propeller_asset(this->propeller_asset);
}

void UDroneMovementComponent::enqueue_custom_physics()
{
	auto* primitive_component = get_primitive_component();
	if (primitive_component == nullptr)
	{
		return;
	}

	primitive_component->WakeRigidBody();
	primitive_component->BodyInstance.AddCustomPhysics(this->calculate_custom_physics_delegate);
}

void UDroneMovementComponent::calculate_custom_physics(float delta_time, FBodyInstance* body_instance)
{
	auto substep_body = FSubstepBody::from_body_instance(body_instance);

	const auto substep_duration = 1.0 / this->tick_rate_hz;
	this->remaining_time_accumulator += delta_time;

	bool has_ticked = false;
	for (; this->remaining_time_accumulator >= substep_duration; this->remaining_time_accumulator -= substep_duration)
	{
		has_ticked = true;
		const auto substep_delta_time = substep_duration;

		this->calculate_thrust_custom_physics(substep_delta_time, &substep_body);

		// Apply gravity
		substep_body.add_force(FVector(0.0, 0.0, -9.81 * substep_body.mass));
		this->calculate_drag_custom_physics(substep_delta_time, &substep_body);

		this->record_flight_data(&substep_body);

		substep_body.consume_forces_and_torques(substep_delta_time);
	}

	if (!has_ticked)
	{
		return;
	}

	const auto linear_velocity_uu = substep_body.linear_velocity_world * 100.0;
	const auto angular_velocity_uu = substep_body.angular_velocity_radians_world;

	this->angular_velocity = substep_body.angular_velocity_radians_world;
	body_instance->SetLinearVelocity(linear_velocity_uu, false);
	body_instance->SetAngularVelocityInRadians(angular_velocity_uu, false);
}

void UDroneMovementComponent::calculate_thrust_custom_physics(float delta_time, FSubstepBody* substep_body)
{
	if (this->propulsion_model == nullptr || !this->frame.IsSet() || !this->motor.IsSet() || !this->battery.IsSet() || !this->propeller.IsSet())
	{
		return;
	}

	const auto frame_value = this->frame.GetValue();
	const auto motor_value = this->motor.GetValue();
	const auto battery_value = this->battery.GetValue();
	const auto propeller_value = this->propeller.GetValue();

	const auto drone_setup = FPropulsionDroneSetup(&frame_value, &motor_value, &battery_value, &propeller_value);

	auto* simulation_world = NewObject<USimulationWorld>();

	this->propulsion_model->tick_propulsion(delta_time, substep_body, this->setpoint, drone_setup, simulation_world);
}

void UDroneMovementComponent::calculate_drag_custom_physics(float delta_time, FSubstepBody* substep_body)
{
	if (!this->frame.IsSet() || !this->propeller.IsSet())
	{
		return;
	}

	const auto frame_value = this->frame.GetValue();
	const auto propeller_value = this->propeller.GetValue();
	auto* simulation_world = NewObject<USimulationWorld>();

	simulation::calculate_linear_drag(substep_body, frame_value, propeller_value, simulation_world);

	simulation::calculate_rotational_drag(substep_body, frame_value, simulation_world);
}

void UDroneMovementComponent::record_flight_data(FSubstepBody* substep_body)
{
	auto* pawn = this->GetPawnOwner();
	auto* drone_pawn = Cast<ADronePawn>(pawn);
	auto* world = this->GetWorld();

	if (drone_pawn == nullptr || world == nullptr || substep_body == nullptr)
	{
		return;
	}

	const auto time_seconds = world->GetTimeSeconds();

	const auto transform = substep_body->transform_world;
	const auto velocity = substep_body->linear_velocity_world;

	// Create event data with all properties
	const auto event_data = FFlightRecordEventData(
		transform.GetLocation(),
		transform.GetRotation().Rotator(),
		velocity,
		this->angular_velocity,
		this->controller_input,
		this->propulsion_info
	);

	drone_pawn->enqueue_flight_record({ FFlightRecordPawnEvent(time_seconds, event_data) });
}

bool UDroneMovementComponent::set_active_flight_mode(FName flight_mode_name)
{
	this->ensure_default_flight_mode();

	if (this->flight_modes.Contains(flight_mode_name))
	{
		this->active_flight_mode = flight_mode_name;
		return true;
	}

	return false;
}

UFlightModeBase* UDroneMovementComponent::get_active_flight_mode()
{
	if (UFlightModeBase** selected_mode = this->flight_modes.Find(this->active_flight_mode))
	{
		return *selected_mode;
	}

	for (const auto& pair : this->flight_modes)
	{
		return pair.Value;
	}

	return nullptr;
}

FName UDroneMovementComponent::get_active_flight_mode_name() const
{
	if (this->flight_modes.Contains(this->active_flight_mode))
	{
		return this->active_flight_mode;
	}

	for (const auto& pair : this->flight_modes)
	{
		return pair.Key;
	}

	return NAME_None;
}

void UDroneMovementComponent::ensure_default_flight_mode()
{
	if (!this->flight_modes.Contains(this->active_flight_mode) && this->flight_modes.Num() > 0)
	{
		this->active_flight_mode = this->flight_modes.CreateConstIterator().Key();
	}
}

FDronePlayerInput UDroneMovementComponent::read_player_input(const UFlightModeBase* active_mode)
{
	auto input = FDronePlayerInput::zero();

	const EThrottleCalibrationSpace throttle_calibration_space = active_mode != nullptr
		? active_mode->get_throttle_calibration_space()
		: EThrottleCalibrationSpace::PositiveOnly;
	EDroneInputPrecisionMode precision_mode = EDroneInputPrecisionMode::LowPrecision;

	if (active_mode != nullptr && active_mode->IsA(UFlightModeAir::StaticClass()))
	{
		precision_mode = EDroneInputPrecisionMode::HighPrecision;
	}

	const auto* pawn = this->GetPawnOwner();
	if (!pawn)
	{
		return input;
	}

	// We don't want input on pawns that are not controlled by the player
	const auto* player_controller = Cast<APlayerController>(pawn->Controller);
	if (!player_controller)
	{
		return input;
	}

	if (const auto* game_instance = pawn->GetGameInstance())
	{
		if (auto* input_subsystem = game_instance->GetSubsystem<UDroneInputSubsystem>())
		{
			input_subsystem->set_precision_mode(precision_mode);
			input.throttle = input_subsystem->get_calibrated_axis_value(EDroneInputAxis::Throttle, throttle_calibration_space);
			input.yaw = input_subsystem->get_calibrated_axis_value(EDroneInputAxis::Yaw);
			input.pitch = -input_subsystem->get_calibrated_axis_value(EDroneInputAxis::Pitch);
			input.roll = input_subsystem->get_calibrated_axis_value(EDroneInputAxis::Roll);
		}
	}

	return input;
}

FFlightModeState UDroneMovementComponent::build_flight_mode_state(float delta_time) const
{
	FFlightModeState flight_state;
	flight_state.delta_time = delta_time;

	if (const auto* primitive_component = this->get_primitive_component())
	{
		flight_state.linear_velocity_world = primitive_component->GetComponentVelocity() * 0.01;
		flight_state.angular_velocity_world = primitive_component->GetPhysicsAngularVelocityInRadians();
		flight_state.rotation = primitive_component->GetComponentRotation();
		return flight_state;
	}

	if (const auto* primitive_component = Cast<UPrimitiveComponent>(this->UpdatedComponent))
	{
		flight_state.linear_velocity_world = primitive_component->GetComponentVelocity();
		flight_state.rotation = primitive_component->GetComponentRotation();
	}

	return flight_state;
}

UPrimitiveComponent* UDroneMovementComponent::get_primitive_component() const
{
	if (this->UpdatedComponent == nullptr)
	{
		return nullptr;
	}

	auto* primitive_component = Cast<UPrimitiveComponent>(this->UpdatedComponent);
	if (primitive_component == nullptr || !primitive_component->IsSimulatingPhysics())
	{
		return nullptr;
	}

	return primitive_component;
}
