#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"
#include "Misc/Optional.h"

class UDroneAirfoilAssetBase;
class UDroneBatteryAsset;
class UDroneMotorAsset;
class UDroneFrameAsset;
class UDronePropellerAsset;

namespace conversion
{
	TOptional<FDronePropellerBemt> convert_propeller_bemt_asset(const UDronePropellerAsset* asset);

	TOptional<FDronePropellerSimplified> convert_propeller_simplified_asset(const UDronePropellerAsset* asset);

	FDroneAirfoil convert_airfoil_asset(const UDroneAirfoilAssetBase* asset);

	FDroneFrame convert_frame_asset(const UDroneFrameAsset* asset);

	FDroneMotor convert_motor_asset(const UDroneMotorAsset* asset);

	FDroneBattery convert_battery_asset(const UDroneBatteryAsset* asset);
}
