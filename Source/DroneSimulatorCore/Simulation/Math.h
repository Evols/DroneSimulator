#pragma once

#include "CoreMinimal.h"

namespace math
{
	constexpr double rpm_to_rad_per_sec(double rpm)
	{
		return rpm * (TWO_PI / 60.0);
	}
}
