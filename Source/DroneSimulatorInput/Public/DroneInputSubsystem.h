#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DroneInputTypes.h"
#include "DroneInputSubsystem.generated.h"

class FDroneInputProcessor;
class FDroneInputCalibrator;
class FDroneInputProfileManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputDeviceDetected, const FDroneInputDevice&, Device);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputDeviceDetectedNative, const FDroneInputDevice&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAxisMapped, EDroneInputAxis, GameAxis, FName, DeviceAxisName);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAxisMappedNative, EDroneInputAxis, FName);

UCLASS()
class DRONESIMULATORINPUT_API UDroneInputSubsystem : public UEngineSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

    // Allow HID polling code to register devices and reuse internal mapping/calibration state.
    friend class FHIDInputManager;
    friend class FDroneInputCalibrator;

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override { return TStatId(); }
    virtual bool IsTickable() const override { return true; }

    // Called by FDroneInputProcessor
    void handle_raw_analog_input(int32 device_id, FKey key, float analog_value);
    void update_raw_int_axis_value(int32 device_id, FName axis_name, int32 raw_value);
    void handle_key_down(int32 device_id, FKey key);
    void handle_key_up(int32 device_id, FKey key);
    void handle_device_disconnected(int32 device_id);

    // Device Management
    UFUNCTION(BlueprintCallable, Category = "Drone Input")
    TArray<FDroneInputDevice> get_all_devices() const;

    UFUNCTION(BlueprintCallable, Category = "Drone Input")
    FDroneInputDevice get_device(int32 device_id) const;

    UFUNCTION(BlueprintCallable, Category = "Drone Input")
    void set_axis_mapping(int32 device_id, FName axis_name, EDroneInputAxis game_axis);

    UFUNCTION(BlueprintPure, Category = "Drone Input")
    float get_calibrated_axis_value(EDroneInputAxis axis, EThrottleCalibrationSpace throttle_calibration_space = EThrottleCalibrationSpace::PositiveOnly) const;

    // Access raw values for UI
    UFUNCTION(BlueprintPure, Category = "Drone Input")
    float get_raw_axis_value(int32 device_id, FName axis_name) const;

    UFUNCTION(BlueprintPure, Category = "Drone Input")
    bool is_device_calibrated(int32 device_id) const;

    UFUNCTION(BlueprintCallable, Category = "Drone Input")
    void remove_device_calibration(int32 device_id);

    UFUNCTION(BlueprintPure, Category = "Drone Input")
    bool is_axis_inverted(int32 device_id, EDroneInputAxis axis) const;

    UFUNCTION(BlueprintCallable, Category = "Drone Input")
    void set_axis_inverted(int32 device_id, EDroneInputAxis axis, bool inverted);

    bool try_get_calibrated_axis_value_for_user(EDroneInputAxis axis,
        EThrottleCalibrationSpace throttle_calibration_space,
        EDroneInputPrecisionMode precision_mode,
        FPlatformUserId platform_user_id,
        float& out_value,
        int32& out_device_id) const;

    FPlatformUserId get_platform_user_for_device(int32 device_id) const;
    void set_device_platform_user(int32 device_id, FPlatformUserId platform_user_id);
    bool is_hid_device(int32 device_id) const;
    FInputDeviceId get_or_create_input_device_id(int32 device_id, bool is_hid_device);

    UPROPERTY(BlueprintAssignable, Category = "Drone Input")
    FOnAxisMapped on_axis_mapped;

    FOnAxisMappedNative on_axis_mapped_native;
    FOnInputDeviceDetectedNative on_input_device_detected_native;

    TSharedPtr<FDroneInputCalibrator> get_calibrator() const { return calibrator; }

protected:
    void register_device(int32 device_id, const FString& device_name = FString(), const FString& device_uid = FString(), bool is_hid_device = false);
    TSharedPtr<FDroneInputProcessor> input_processor;

    UPROPERTY()
    TMap<int32, FDroneInputDevice> devices;

    // Mappings: DeviceID -> (AxisName -> GameAxis)
    TMap<int32, TMap<FName, FInputAxisMapping>> device_mappings;

    // Calibration: DeviceID -> (AxisName -> Data)
    TMap<int32, TMap<FName, FAxisCalibrationData>> calibration_data;

    TSharedPtr<FDroneInputCalibrator> calibrator;
    TSharedPtr<FDroneInputProfileManager> profile_manager;

    void reset_calibration_state_for_device(int32 device_id);

    TSet<int32> hid_device_ids;
    TMap<int32, FInputDeviceId> device_input_ids;
    TMap<int32, FPlatformUserId> device_platform_users;
};
