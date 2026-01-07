#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"

#include "DroneAirfoilAsset.generated.h"

USTRUCT(BlueprintType)
struct DRONESIMULATORGAME_API FAngleOfAttackXfoilData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Angle of attack (in degrees)"))
  float angle_of_attack = 0.f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Lift coefficient"))
  float lift_coefficient = 0.f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Drag coefficient"))
  float drag_coefficient = 0.f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Moment coefficient"))
  float moment_coefficient = 0.f;
};

USTRUCT(BlueprintType)
struct DRONESIMULATORGAME_API FReynoldsXfoilData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Reynolds number"))
  float reynolds_number = 0.0f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Xfoil data per angle of attack"))
  TArray<FAngleOfAttackXfoilData> angle_of_attack_data;
};

USTRUCT(BlueprintType)
struct DRONESIMULATORGAME_API FRawXfoilData {
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Xfoil data per Reynolds number"))
  TArray<FReynoldsXfoilData> reynolds_data;
};

UCLASS(BlueprintType, Abstract)
class DRONESIMULATORGAME_API UDroneAirfoilAssetBase : public UPrimaryDataAsset {
  GENERATED_BODY()
};

UCLASS(BlueprintType)
class DRONESIMULATORGAME_API UDroneAirfoilAssetTable
    : public UDroneAirfoilAssetBase {
  GENERATED_BODY()

  // Imported data
public:
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Imported airfoil data (includes Viterna post-stall correction to ±90°)"))
  FRawXfoilData imported_xfoil_data;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Imported name"))
  FString imported_name;

  /** Convert to simulation-ready table */
  FDroneAirfoilTable ToSimulationTable() const;
};

UCLASS(BlueprintType)
class DRONESIMULATORGAME_API UDroneAirfoilAssetSimplified
    : public UDroneAirfoilAssetBase {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Cl k (1/rad) - Linear lift slope"))
  double cl_k_rad = 5.5;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName = "Cd 0 - Base profile drag"))
  double cd_0 = 0.022;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone simulator",
            meta = (DisplayName =
                        "Cd k - Induced drag multiplier (Cd += cd_k * Cl^2)"))
  double cd_k = 0.03;

  /** Convert to simulation-ready struct */
  FDroneAirfoilSimplified ToSimulationSimplified() const;
};
