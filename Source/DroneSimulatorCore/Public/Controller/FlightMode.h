#pragma once

#include "DroneSimulatorCore/Public/Controller/ControllerInput.h"
#include "DroneSimulatorCore/Public/Controller/Setpoint.h"

#ifdef WITH_DRONE_INPUT
#include "DroneInputTypes.h"
#endif

#include "FlightMode.generated.h"

USTRUCT(BlueprintType)
struct FFlightModeState
{
    GENERATED_BODY()

public:

	// World linear velocity in m/s
    UPROPERTY(BlueprintReadOnly)
    FVector linear_velocity_world = FVector::ZeroVector;

	// World angular velocity in rad/s
    UPROPERTY(BlueprintReadOnly)
    FVector angular_velocity_world = FVector::ZeroVector;

	// World rotation
    UPROPERTY(BlueprintReadOnly)
    FRotator rotation = FRotator::ZeroRotator;

	// Delta time used for the current tick
    UPROPERTY(BlueprintReadOnly)
    double delta_time = 0.0;
};

UENUM(BlueprintType)
enum ECameraTiltMode : uint8
{
    Zero,
    Tilted,
    Stabilized
};

/**
 * Flight mode class, that computes the desired rotation of the drone.
 * The controller (typically a PID) will then actuate the motors via a PID to try to reach this target setpoint.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class UFlightModeBase : public UObject
{
    GENERATED_BODY()

public:

#ifdef WITH_DRONE_INPUT
    virtual EThrottleCalibrationSpace get_throttle_calibration_space() const;
#endif

    UFUNCTION(BlueprintPure, Category="FlightMode")
    virtual ECameraTiltMode get_camera_tilt_mode() const;

    /**
     * Target setpoint, that the controller will try to reach using the motors and its PID.
     * This is called at each subtick
     *
     * @param player_input Input of the player controller, typically via the remote controller
     */
    virtual FDroneSetpoint compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state);
};
