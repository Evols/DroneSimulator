#pragma once

#include "KnownHid.generated.h"

UENUM(BlueprintType)
enum class EKnownHidVendor : uint8
{
    DJI UMETA(DisplayName = "DJI"),
    PidCodes UMETA(DisplayName = "PidCodes"),
    Rode UMETA(DisplayName = "Rode"),
    Microsoft UMETA(DisplayName = "Microsoft"),
    Logitech UMETA(DisplayName = "Logitech"),
    Razer UMETA(DisplayName = "Razer"),
    Sony UMETA(DisplayName = "Sony"),
    Unknown UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EKnownHidProduct : uint8
{
    DJI_FpvRc2 UMETA(DisplayName = "FPV RC2"),
    MS_XboxController UMETA(DisplayName = "Xbox Controller"),
    Sony_Ps5Controller_Wired UMETA(DisplayName = "PS5 Controller (Wired)"),
    Sony_Ps5Controller_BT UMETA(DisplayName = "PS5 Controller (Bluetooth)"),
    OpenTx UMETA(DisplayName = "OpenTX radio transmitter"),
    Unknown UMETA(DisplayName = "Unknown")
};

UCLASS()
class UKnownHidLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="KnownHID")
    static EKnownHidVendor parse_vendor_id(int32 vendor_id);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="KnownHID")
    static EKnownHidProduct parse_product_id(const EKnownHidVendor& vendor_known, int32 product_id);
};
