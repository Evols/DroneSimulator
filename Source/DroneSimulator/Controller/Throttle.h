#pragma once

#include "Throttle.generated.h"

/**
 * Throttle for each propeller. In [0..1] range
 */
USTRUCT(BlueprintType)
struct FPropellerSetThrottle
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	double front_left = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double front_right = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double rear_left = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double rear_right = 0.0;

public:
	
	FPropellerSetThrottle() = default;

	FPropellerSetThrottle(double in_front_left, double in_front_right, double in_rear_left, double in_rear_right);

	static FPropellerSetThrottle from_real(double real);

	FString to_string() const;
	
public:
	
	FPropellerSetThrottle operator+(const FPropellerSetThrottle& other) const;

	FPropellerSetThrottle operator-(const FPropellerSetThrottle& other) const;

	FPropellerSetThrottle operator-() const;

	FPropellerSetThrottle operator*(const FPropellerSetThrottle& other) const;

	FPropellerSetThrottle operator*(double other) const;

	FPropellerSetThrottle operator/(const FPropellerSetThrottle& other) const;

	FPropellerSetThrottle operator/(double other) const;
};
