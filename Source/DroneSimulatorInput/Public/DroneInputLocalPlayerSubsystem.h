#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "DroneInputTypes.h"
#include "DroneInputLocalPlayerSubsystem.generated.h"

class UDroneInputSubsystem;

UCLASS()
class DRONESIMULATORINPUT_API UDroneInputLocalPlayerSubsystem
	: public ULocalPlayerSubsystem
	, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return TStatId(); }
	virtual bool IsTickable() const override { return true; }

	void set_precision_mode(EDroneInputPrecisionMode in_precision_mode);
	void set_throttle_calibration_space(EThrottleCalibrationSpace in_throttle_calibration_space);
	EDroneInputPrecisionMode get_precision_mode() const;

	void assign_device_to_local_player(int32 device_id);
	FPlatformUserId get_platform_user_id() const { return platform_user_id; }

private:
	TWeakObjectPtr<UDroneInputSubsystem> device_subsystem;
	EDroneInputPrecisionMode precision_mode = EDroneInputPrecisionMode::LowPrecision;
	EThrottleCalibrationSpace throttle_calibration_space = EThrottleCalibrationSpace::PositiveOnly;
	TMap<EDroneInputAxis, int32> last_axis_device_ids;
	FPlatformUserId platform_user_id = PLATFORMUSERID_NONE;

	void dispatch_axis(EDroneInputAxis axis, const FKey& key, EThrottleCalibrationSpace axis_throttle_space);
};
