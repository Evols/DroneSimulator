#pragma once

#include "Containers/Union.h"
#include "CoreMinimal.h"

#include "Structural.generated.h"

/**
 * Fast lookup table for Xfoil aerodynamic coefficients.
 * Optimized for cache-friendly access during Bemt simulation.
 * Data is stored in contiguous arrays for fast interpolation.
 */
struct DRONESIMULATORCORE_API FDroneAirfoilTable
{
	// Reynolds numbers (sorted ascending)
	TArray<float> reynolds_numbers;

	// For each Reynolds number, we store angle of attack data
	// Each entry corresponds to one Reynolds number
	struct FReynoldsEntry {
	TArray<float> angles_of_attack; // In radians, sorted ascending
	TArray<float> lift_coefficients;
	TArray<float> drag_coefficients;
	};

	TArray<FReynoldsEntry> reynolds_entries;

	/** Default constructor */
	FDroneAirfoilTable() = default;

	/** Constructor with all fields */
	FDroneAirfoilTable(TArray<float> InReynoldsNumbers, TArray<FReynoldsEntry> InReynoldsEntries)
		: reynolds_numbers(MoveTemp(InReynoldsNumbers)), reynolds_entries(MoveTemp(InReynoldsEntries))
	{}

	/** Check if table is valid and ready for use */
	bool is_valid() const
	{
		return reynolds_numbers.Num() > 0 && reynolds_entries.Num() == reynolds_numbers.Num() &&
			reynolds_entries.Num() > 0 && reynolds_entries[0].angles_of_attack.Num() > 0;
	}
};

struct DRONESIMULATORCORE_API FDroneAirfoilSimplified
{
	// Linear lift coefficient: Cl = cl_k_rad * angle_of_attack
	double cl_k_rad = 5.5;

	// Base drag coefficient (profile drag)
	double cd_0 = 0.022;

	// Induced drag coefficient multiplier: Cd_induced = cd_k * Cl^2
	// For propellers, typical values: 0.01 to 0.05
	// Higher values = more drag from lift generation (important for yaw torque!)
	double cd_k = 0.03;

	/** Default constructor */
	FDroneAirfoilSimplified() = default;

	/** Constructor with all fields */
	FDroneAirfoilSimplified(double InClKRad, double InCd0, double InCdK)
		: cl_k_rad(InClKRad), cd_0(InCd0), cd_k(InCdK)
	{}
};

using FDroneAirfoil = TUnion<FDroneAirfoilTable, FDroneAirfoilSimplified>;

/**
 * Inside simulation, we want to work with SI units (meters, kilograms, seconds,
 * Newtons) and radians.
 *
 * For configuration, we display easy to understand units (inches, cm, kg) and
 * degrees. Unreal uses centimeters and centi-Newtons (kg·cm/s²).
 */
USTRUCT(BlueprintType)
struct DRONESIMULATORCORE_API FDronePropellerBemt
{
	GENERATED_BODY()

public:
	/** Number of blades */
	UPROPERTY(BlueprintReadWrite)
	int32 num_blades = 0;

	/** Tip radius, in meters (blade length from hub center). */
	UPROPERTY(BlueprintReadWrite)
	double radius = 0.0;

	/** Hub (root) radius, in meters – no lift inside this radius. */
	UPROPERTY(BlueprintReadWrite)
	double hub_radius = 0.0;

	/** Chord, in meters, assumed constant along span (keep it simple). */
	UPROPERTY(BlueprintReadWrite)
	double chord = 0.0;

	/** Linear twist, in meters. */
	UPROPERTY(BlueprintReadWrite)
	double pitch = 0.0;

	/** airfoil table for more accurate aerodynamics. */
	FDroneAirfoil airfoil;
};

USTRUCT(BlueprintType)
struct DRONESIMULATORCORE_API FDronePropellerSimplified
{
	GENERATED_BODY()

public:

	// In meters
	UPROPERTY()
	double blade_diameter;

	// Thrust coefficient, dimensionless
	UPROPERTY()
	double thrust_coefficient;

	// Torque coefficient, dimensionless
	UPROPERTY()
	double torque_coefficient;
};

using TDronePropeller = TVariant<FDronePropellerBemt, FDronePropellerSimplified>;

/**
 * Inside simulation, we want to work with SI units (meters, kilograms, seconds,
 * Newtons) and radians.
 *
 * For configuration, we display easy to understand units (inches, cm, kg) and
 * degrees. Unreal uses centimeters and centi-Newtons (kg·cm/s²).
 */
USTRUCT(BlueprintType)
struct DRONESIMULATORCORE_API FDroneFrame
{
	GENERATED_BODY()

public:
	// Distance in each axis from the center of the drone, in unreal-units
	// (centimeters).
	UPROPERTY(BlueprintReadWrite)
	FVector props_extent_front = FVector(10.0, 10.0, 0.0);

	/**
	 * Distance in each axis from the center of the drone, in unreal-units
	 * (centimeters).
	 */
	UPROPERTY(BlueprintReadWrite)
	FVector props_extent_back = FVector(-10.0, 10.0, 0.0);

	/**
	 * Drag coefficient of the frame.
	 * Dimension-less property, often referred as "Cd".
	 * X axis is forward-backward, Y axis is right-left, Z axis is top-down.
	 *
	 * See https://en.wikipedia.org/wiki/Drag_coefficient
	 */
	UPROPERTY(BlueprintReadWrite)
	FVector drag_coefficient = FVector::ZeroVector;

	/**
	 * Area of the frame in m², in each axis.
	 */
	UPROPERTY(BlueprintReadWrite)
	FVector area = FVector::ZeroVector;

	/**
	 * Mass of the frame in kg.
	 */
	UPROPERTY(BlueprintReadWrite)
	double mass = 0.0;
};

/**
 * Inside simulation, we want to work with SI units (meters, kilograms, seconds,
 * Newtons) and radians.
 *
 * For configuration, we display easy to understand units (inches, cm, kg) and
 * degrees. Unreal uses centimeters and centi-Newtons (kg·cm/s²).
 */
USTRUCT(BlueprintType)
struct DRONESIMULATORCORE_API FDroneMotor
{
	GENERATED_BODY()

public:
	/**
	 * Motor velocity constant, in radians per second per volt
	 */
	UPROPERTY(BlueprintReadWrite)
	double kv = 0.0;

	/**
	 * Mass of each motor, in kg.
	 */
	UPROPERTY(BlueprintReadWrite)
	double mass = 0.0;
};

/**
 * Inside simulation, we want to work with SI units (meters, kilograms, seconds,
 * Newtons) and radians.
 *
 * For configuration, we display easy to understand units (inches, cm, kg) and
 * degrees. Unreal uses centimeters and centi-Newtons (kg·cm/s²).
 */
USTRUCT(BlueprintType)
struct DRONESIMULATORCORE_API FDroneBattery
{
	GENERATED_BODY()

public:
	/**
	 * Voltage of the battery, in volts
	 */
	UPROPERTY(BlueprintReadWrite)
	double voltage = 0.0;

	/**
	 * Mass of the battery, in kg.
	 */
	UPROPERTY(BlueprintReadWrite)
	double mass = 0.0;
};
