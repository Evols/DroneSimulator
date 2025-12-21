#pragma once

#include "DroneSimulatorGame/Gameplay/Recording/FlightRecord.h"
#include "DroneSimulatorInput/Public/DroneInputTypes.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"

#include "DronePawn.generated.h"


class UDroneController;
class UInputAction;
class UDroneMovementComponent;
class UDroneMovementController;
class UInputMappingContext;
struct FInputActionValue;


USTRUCT(BlueprintType)
struct FFlightRecordPawnEvent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double event_time;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FFlightRecordEventData event_tata;

	FFlightRecordPawnEvent() = default;

	FFlightRecordPawnEvent(double in_event_time, const FFlightRecordEventData& in_event_data)
		: event_time(in_event_time), event_tata(in_event_data)
	{
	}
};

class UDroneInputSettings;


UCLASS(meta=(BlueprintSpawnableComponent))
class DRONESIMULATORGAME_API ADronePawn : public APawn
{
	GENERATED_BODY()

public:

	ADronePawn();

public:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true", DisplayName="Movement component"))
	TObjectPtr<UDroneMovementComponent> movement_component;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void ensure_input_actions();
	void ensure_input_mapping_context();

	void handle_throttle_input(const FInputActionValue& value);
	void handle_yaw_input(const FInputActionValue& value);
	void handle_pitch_input(const FInputActionValue& value);
	void handle_roll_input(const FInputActionValue& value);
	void handle_aux_input(const FInputActionValue& value, int32 aux_index);

	EDroneInputSwitchPosition compute_switch_position(float raw_value) const;
	void apply_switch_position(EDroneInputSwitchPosition new_position);

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> input_mapping_context;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> throttle_action;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> yaw_action;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> pitch_action;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> roll_action;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInputAction>> aux_actions;

	UPROPERTY(EditAnywhere, Category="Input")
	int32 input_mapping_priority = 0;

	UPROPERTY(EditAnywhere, Category="Input|Switch")
	float switch_low_threshold = -0.5f;

	UPROPERTY(EditAnywhere, Category="Input|Switch")
	float switch_high_threshold = 0.5f;

	UPROPERTY(EditAnywhere, Category="Input|Switch")
	TMap<EDroneInputSwitchPosition, FName> switch_flight_modes;

	UPROPERTY(EditAnywhere, Category="Input|Switch")
	int32 switch_aux_channel = 1;

	EDroneInputSwitchPosition current_switch_position = EDroneInputSwitchPosition::Mid;

	bool input_bound = false;

protected:
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Input")
	TArray<float> aux_values;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool enable_recording = true;
	
	FCriticalSection temp_flight_record_mutex;
	TArray<FFlightRecordPawnEvent> temp_flight_record;

	UFUNCTION()
	void enqueue_flight_record(const TArray<FFlightRecordPawnEvent>& events);

	UFUNCTION()
	[[nodiscard]] TArray<FFlightRecordPawnEvent> consume_flight_record();

	UFUNCTION(BlueprintPure, Category="Input")
	float get_aux_value(int32 aux_channel) const;
};
