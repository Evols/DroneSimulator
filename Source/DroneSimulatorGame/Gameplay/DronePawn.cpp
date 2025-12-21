#include "DroneSimulatorGame/Gameplay/DronePawn.h"
#include "DroneSimulatorGame/Gameplay/DroneMovementComponent.h"
#include "DroneSimulatorInput/Public/DroneInputKeys.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"


ADronePawn::ADronePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	movement_component = CreateDefaultSubobject<UDroneMovementComponent>("MovementComponent");
}

UPawnMovementComponent* ADronePawn::GetMovementComponent() const
{
	return this->movement_component;
}

void ADronePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (input_bound)
	{
		return;
	}

	auto* enhanced_input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!enhanced_input)
	{
		return;
	}

	ensure_input_actions();
	ensure_input_mapping_context();

	if (input_mapping_context)
	{
		if (auto* player_controller = Cast<APlayerController>(GetController()))
		{
			if (auto* local_player = player_controller->GetLocalPlayer())
			{
				if (auto* input_subsystem = local_player->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					input_subsystem->AddMappingContext(input_mapping_context, input_mapping_priority);
				}
			}
		}
	}

	enhanced_input->BindAction(throttle_action, ETriggerEvent::Triggered, this, &ADronePawn::handle_throttle_input);
	enhanced_input->BindAction(throttle_action, ETriggerEvent::Completed, this, &ADronePawn::handle_throttle_input);
	enhanced_input->BindAction(throttle_action, ETriggerEvent::Canceled, this, &ADronePawn::handle_throttle_input);

	enhanced_input->BindAction(yaw_action, ETriggerEvent::Triggered, this, &ADronePawn::handle_yaw_input);
	enhanced_input->BindAction(yaw_action, ETriggerEvent::Completed, this, &ADronePawn::handle_yaw_input);
	enhanced_input->BindAction(yaw_action, ETriggerEvent::Canceled, this, &ADronePawn::handle_yaw_input);

	enhanced_input->BindAction(pitch_action, ETriggerEvent::Triggered, this, &ADronePawn::handle_pitch_input);
	enhanced_input->BindAction(pitch_action, ETriggerEvent::Completed, this, &ADronePawn::handle_pitch_input);
	enhanced_input->BindAction(pitch_action, ETriggerEvent::Canceled, this, &ADronePawn::handle_pitch_input);

	enhanced_input->BindAction(roll_action, ETriggerEvent::Triggered, this, &ADronePawn::handle_roll_input);
	enhanced_input->BindAction(roll_action, ETriggerEvent::Completed, this, &ADronePawn::handle_roll_input);
	enhanced_input->BindAction(roll_action, ETriggerEvent::Canceled, this, &ADronePawn::handle_roll_input);

	for (int32 aux_index = 0; aux_index < aux_actions.Num(); ++aux_index)
	{
		if (!aux_actions[aux_index])
		{
			continue;
		}

		enhanced_input->BindAction(aux_actions[aux_index], ETriggerEvent::Triggered, this, &ADronePawn::handle_aux_input, aux_index);
		enhanced_input->BindAction(aux_actions[aux_index], ETriggerEvent::Completed, this, &ADronePawn::handle_aux_input, aux_index);
		enhanced_input->BindAction(aux_actions[aux_index], ETriggerEvent::Canceled, this, &ADronePawn::handle_aux_input, aux_index);
	}

	input_bound = true;
}

void ADronePawn::ensure_input_actions()
{
	if (!throttle_action)
	{
		throttle_action = NewObject<UInputAction>(this, TEXT("DroneThrottleAction"));
		throttle_action->ValueType = EInputActionValueType::Axis1D;
	}

	if (!yaw_action)
	{
		yaw_action = NewObject<UInputAction>(this, TEXT("DroneYawAction"));
		yaw_action->ValueType = EInputActionValueType::Axis1D;
	}

	if (!pitch_action)
	{
		pitch_action = NewObject<UInputAction>(this, TEXT("DronePitchAction"));
		pitch_action->ValueType = EInputActionValueType::Axis1D;
	}

	if (!roll_action)
	{
		roll_action = NewObject<UInputAction>(this, TEXT("DroneRollAction"));
		roll_action->ValueType = EInputActionValueType::Axis1D;
	}

	if (aux_actions.Num() != drone_input::aux_axis_count)
	{
		aux_actions.SetNum(drone_input::aux_axis_count);
	}

	for (int32 aux_index = 0; aux_index < aux_actions.Num(); ++aux_index)
	{
		if (!aux_actions[aux_index])
		{
			const FString action_name = FString::Printf(TEXT("DroneAuxAction%d"), aux_index + 1);
			aux_actions[aux_index] = NewObject<UInputAction>(this, *action_name);
			aux_actions[aux_index]->ValueType = EInputActionValueType::Axis1D;
		}
	}

	if (aux_values.Num() != drone_input::aux_axis_count)
	{
		aux_values.Init(0.0f, drone_input::aux_axis_count);
	}
}

void ADronePawn::ensure_input_mapping_context()
{
	if (input_mapping_context)
	{
		return;
	}

	input_mapping_context = NewObject<UInputMappingContext>(this, TEXT("DroneInputMappingContext"));
	if (!input_mapping_context)
	{
		return;
	}

	input_mapping_context->MapKey(throttle_action, FDroneInputKeys::throttle);
	input_mapping_context->MapKey(yaw_action, FDroneInputKeys::yaw);
	input_mapping_context->MapKey(pitch_action, FDroneInputKeys::pitch);
	input_mapping_context->MapKey(roll_action, FDroneInputKeys::roll);

	for (int32 aux_index = 0; aux_index < aux_actions.Num(); ++aux_index)
	{
		if (!aux_actions[aux_index])
		{
			continue;
		}

		const FKey aux_key = FDroneInputKeys::get_aux_key(aux_index);
		if (aux_key.IsValid())
		{
			input_mapping_context->MapKey(aux_actions[aux_index], aux_key);
		}
	}
}

void ADronePawn::handle_throttle_input(const FInputActionValue& value)
{
	if (!movement_component)
	{
		return;
	}

	movement_component->player_input.throttle = static_cast<double>(value.Get<float>());
}

void ADronePawn::handle_yaw_input(const FInputActionValue& value)
{
	if (!movement_component)
	{
		return;
	}

	movement_component->player_input.yaw = static_cast<double>(value.Get<float>());
}

void ADronePawn::handle_pitch_input(const FInputActionValue& value)
{
	if (!movement_component)
	{
		return;
	}

	movement_component->player_input.pitch = -static_cast<double>(value.Get<float>());
}

void ADronePawn::handle_roll_input(const FInputActionValue& value)
{
	if (!movement_component)
	{
		return;
	}

	movement_component->player_input.roll = static_cast<double>(value.Get<float>());
}

void ADronePawn::handle_aux_input(const FInputActionValue& value, int32 aux_index)
{
	const float axis_value = value.Get<float>();

	if (aux_values.IsValidIndex(aux_index))
	{
		aux_values[aux_index] = axis_value;
	}

	if (switch_aux_channel == aux_index + 1)
	{
		apply_switch_position(compute_switch_position(axis_value));
	}
}

EDroneInputSwitchPosition ADronePawn::compute_switch_position(float raw_value) const
{
	if (raw_value <= switch_low_threshold)
	{
		return EDroneInputSwitchPosition::Low;
	}

	if (raw_value >= switch_high_threshold)
	{
		return EDroneInputSwitchPosition::High;
	}

	return EDroneInputSwitchPosition::Mid;
}

void ADronePawn::apply_switch_position(EDroneInputSwitchPosition new_position)
{
	if (new_position == current_switch_position)
	{
		return;
	}

	current_switch_position = new_position;

	if (!movement_component)
	{
		return;
	}

	if (const FName* flight_mode_name = switch_flight_modes.Find(new_position))
	{
		if (!flight_mode_name->IsNone())
		{
			movement_component->set_active_flight_mode(*flight_mode_name);
		}
	}
}

void ADronePawn::enqueue_flight_record(const TArray<FFlightRecordPawnEvent>& events)
{
	if (!enable_recording)
	{
		return;
	}

	FScopeLock Lock(&this->temp_flight_record_mutex);
	temp_flight_record.Append(events);
}

TArray<FFlightRecordPawnEvent> ADronePawn::consume_flight_record()
{
	FScopeLock Lock(&this->temp_flight_record_mutex);
	auto temp_flight_record_copy = temp_flight_record;
	temp_flight_record = {};
	return temp_flight_record_copy;
}

float ADronePawn::get_aux_value(int32 aux_channel) const
{
	const int32 index = aux_channel - 1;
	if (aux_values.IsValidIndex(index))
	{
		return aux_values[index];
	}

	return 0.0f;
}
