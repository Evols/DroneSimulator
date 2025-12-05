#pragma once

#include "CoreMinimal.h"

#include "DroneSimulatorInput/Public/KnownHid.h"

#include "DroneInputSettings.generated.h"

UENUM(BlueprintType)
enum class ERawInputDeviceType : uint8
{
	Mouse     UMETA(DisplayName = "Mouse"),
	Keyboard  UMETA(DisplayName = "Keyboard"),
	HID       UMETA(DisplayName = "HID"),
	Unknown   UMETA(DisplayName = "Unknown")
};

/**
 * Device that can be handled by the Raw Input plugin.
 * Used to enumerate the devices to be shown to the user, where he can choose the input device.
 */
USTRUCT(BlueprintType)
struct DRONESIMULATORINPUT_API FRawInputDeviceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	FString device_name;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	ERawInputDeviceType device_type = ERawInputDeviceType::Unknown;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	int32 vendor_id = 0;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	int32 product_id = 0;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	EKnownHidVendor vendor_known = EKnownHidVendor::Unknown;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	EKnownHidProduct product_known = EKnownHidProduct::Unknown;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	int32 usage_page = 0;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	int32 usage = 0;

	UPROPERTY(BlueprintReadOnly, Category="Drone Raw Input")
	FString display_name;

	FRawInputDeviceInfo() = default;
};

UCLASS()
class DRONESIMULATORINPUT_API UDroneInputSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drone Raw Input", meta = (DisplayName = "Get Drone Input Settings"))
	static UDroneInputSettings* get_instance();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drone Raw Input")
	TArray<FRawInputDeviceInfo> get_hid_devices();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drone Raw Input")
	TArray<FRawInputDeviceInfo> find_devices_by_vendor_id(int32 vendor_id);

	UFUNCTION(BlueprintCallable, Category="Drone Raw Input")
	bool register_device(int32 usage_page, int32 usage);

	UFUNCTION(BlueprintCallable, Category="Drone Raw Input")
	void register_all_hid_devices();

	UFUNCTION(BlueprintCallable, Category="Drone Raw Input")
	void log_hid_devices();

	UFUNCTION(BlueprintCallable, Category="Drone Raw Input")
	void initialize();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drone Raw Input")
	static FString get_device_vid(const FRawInputDeviceInfo& device);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Drone Raw Input")
	static FString get_device_pid(const FRawInputDeviceInfo& device);

private:
	TArray<FRawInputDeviceInfo> cached_devices;

	TArray<FRawInputDeviceInfo> query_devices();
	FString create_device_id(const FRawInputDeviceInfo& device) const;
	void log_device(const FRawInputDeviceInfo& device) const;
};
