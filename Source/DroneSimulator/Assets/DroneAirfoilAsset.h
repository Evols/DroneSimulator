#pragma once

#include "CoreMinimal.h"

#include "DroneAirfoilAsset.generated.h"

USTRUCT(BlueprintType)
struct DRONESIMULATOR_API FAngleOfAttackXfoilData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Angle of attack (in degrees)"))
    float angle_of_attack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Lift coefficient"))
    float lift_coefficient;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Drag coefficient"))
    float drag_coefficient;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Moment coefficient"))
    float moment_coefficient = 0.0f;
};

USTRUCT(BlueprintType)
struct DRONESIMULATOR_API FReynoldsXfoilData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Reynolds number"))
    float reynolds_number;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Xfoil data per angle of attack"))
    TArray<FAngleOfAttackXfoilData> angle_of_attack_data;
};

USTRUCT(BlueprintType)
struct DRONESIMULATOR_API FRawXfoilData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Xfoil data per Reynolds number"))
    TArray<FReynoldsXfoilData> reynolds_data;
};

UCLASS(BlueprintType, Abstract)
class DRONESIMULATOR_API UDroneAirfoilAssetBase : public UPrimaryDataAsset
{
    GENERATED_BODY()
};

UCLASS(BlueprintType)
class DRONESIMULATOR_API UDroneAirfoilAssetTable : public UDroneAirfoilAssetBase
{
    GENERATED_BODY()

    // Imported data
public:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Imported airfoil data (includes Viterna post-stall correction to ±90°)"))
    FRawXfoilData imported_xfoil_data;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Imported name"))
    FString imported_name;
};

UCLASS(BlueprintType)
class DRONESIMULATOR_API UDroneAirfoilAssetSimplified : public UDroneAirfoilAssetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Cl k (1/rad) - Linear lift slope"))
    double cl_k_rad = 5.5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Cd 0 - Base profile drag"))
    double cd_0 = 0.022;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator", meta=(DisplayName="Cd k - Induced drag multiplier (Cd += cd_k * Cl^2)"))
    double cd_k = 0.03;
};
