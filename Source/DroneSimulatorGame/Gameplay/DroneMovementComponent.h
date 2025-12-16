#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"

#include "DroneSimulatorGame/Gameplay/Recording/PropulsionInfo.h"
#include "DroneSimulatorCore/Public/Controller/Throttle.h"
#include "DroneSimulatorCore/Public/Controller/ControllerInput.h"
#include "DroneSimulatorCore/Public/Controller/Setpoint.h"
#include "DroneSimulatorCore/Public/Controller/FlightMode.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"

#include "DroneMovementComponent.generated.h"

class UPropulsionModel;
struct FSubstepBody;
class URotorModelBase;
class UDroneController;
class UDroneBatteryAsset;
class UDronePropellerAsset;
class UDroneMotorAsset;
class UDroneFrameAsset;
struct FDroneFrame;

UCLASS(ClassGroup=(Movement), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class DRONESIMULATORGAME_API UDroneMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:

	UDroneMovementComponent();

	FVector last_rot_speed;

protected:

	virtual void BeginPlay() override;

	virtual void TickComponent(float delta_time, ELevelTick tick_type, FActorComponentTickFunction* this_tick_function) override;

	void set_updated_component_mass();

	void set_updated_component_inertia();

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Tick rate (Hz)"))
	double tick_rate_hz = 400.0;

private:

	UPROPERTY()
	double remaining_time_accumulator = 0.0;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Frame"))
	UDroneFrameAsset* frame_asset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Propeller"))
	UDronePropellerAsset* propeller_asset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Motor"))
	UDroneMotorAsset* motor_asset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Battery"))
	UDroneBatteryAsset* battery_asset;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Flight modes"))
	TMap<FName, UFlightModeBase*> flight_modes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Active flight mode"))
	FName active_flight_mode;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category="Drone", meta=(DisplayName="Thrust model"))
	UPropulsionModel* propulsion_model;

protected:

	UPROPERTY()
	TOptional<FDroneFrame> frame;

	UPROPERTY()
	TOptional<FDroneMotor> motor;

	UPROPERTY()
	TOptional<FDroneBattery> battery;

	UPROPERTY()
	TOptional<FDronePropellerBemt> propeller_bemt;

	UPROPERTY()
	TOptional<FDronePropellerSimplified> propeller_simplified;

	UFUNCTION()
	void init_drone_parts();

public:

	UPROPERTY(BlueprintReadWrite)
	FDronePlayerInput player_input;

protected:

	UPROPERTY(BlueprintReadWrite)
	FDronePlayerInput controller_input;

	UPROPERTY(BlueprintReadWrite)
	FDroneSetpoint setpoint;

protected:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FPropellerSetThrottle propeller_set_throttle;

protected:

	// This is exposed for blueprints read. It is not used internally
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TOptional<FPropulsionInfo> propulsion_info;

public:

	UFUNCTION(BlueprintCallable, Category="Drone|Flight mode")
	bool set_active_flight_mode(FName flight_mode_name);

	UFUNCTION(BlueprintPure, Category="Drone|Flight mode")
	UFlightModeBase* get_active_flight_mode();

	UFUNCTION(BlueprintPure, Category="Drone|Flight mode")
	FName get_active_flight_mode_name() const;

private:

	UFUNCTION()
	void enqueue_custom_physics();

	FCalculateCustomPhysics calculate_custom_physics_delegate;

	void calculate_custom_physics(float delta_time, FBodyInstance* body_instance);

	void calculate_thrust_custom_physics(float delta_time, FSubstepBody* substep_body);

	void calculate_drag_custom_physics(float delta_time, FSubstepBody* substep_body);

	void record_flight_data(FSubstepBody* substep_body);

private:

	UFUNCTION()
	UPrimitiveComponent* get_primitive_component() const;

	UFUNCTION()
	void ensure_default_flight_mode();

	FDronePlayerInput read_player_input(const UFlightModeBase* active_mode);

	FFlightModeState build_flight_mode_state(float delta_time) const;

private:

	UPROPERTY()
	FVector angular_velocity;
};
