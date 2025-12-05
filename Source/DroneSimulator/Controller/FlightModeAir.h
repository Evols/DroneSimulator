#pragma once

#include "DroneSimulator/Controller/FlightMode.h"

#include "FlightModeAir.generated.h"

UCLASS(BlueprintType)
class UFlightModeAir : public UFlightModeBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Roll rate (degrees per second)"))
    double roll_rate_deg_per_second = 500.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Pitch rate (degrees per second)"))
    double pitch_rate_deg_per_second = 500.0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Rates", meta=(DisplayName="Yaw rate (degrees per second)"))
    double yaw_rate_deg_per_second = 500.0;

    virtual ECameraTiltMode get_camera_tilt_mode() const override;

    virtual EThrottleCalibrationSpace get_throttle_calibration_space() const override;

    virtual FDroneSetpoint compute_setpoint(const FDronePlayerInput& player_input, const FFlightModeState& flight_state) override;

};
