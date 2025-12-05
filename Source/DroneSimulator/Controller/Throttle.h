#pragma once

#include "Throttle.generated.h"

/**
 * Throttle for each propeller. In [0..1] range
 */
USTRUCT(BlueprintType)
struct FPropellerSetThrottle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	double front_left_throttle = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double front_right_throttle = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double rear_left_throttle = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double rear_right_throttle = 0.0;
};
