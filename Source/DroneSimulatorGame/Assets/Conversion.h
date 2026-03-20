#pragma once

#include "CoreMinimal.h"
#include "DroneSimulatorCore/Public/Simulation/Structural.h"
#include "Misc/Optional.h"

class UDronePropellerAsset;
class UDroneAirfoilAssetBase;
class UDroneBatteryAsset;
class UDroneMotorAsset;
class UDroneFrameAsset;

namespace conversion
{
	TOptional<TDronePropeller> convert_propeller_asset(const UDronePropellerAsset* asset);

	FDroneAirfoil convert_airfoil_asset(const UDroneAirfoilAssetBase* asset);

	FDroneFrame convert_frame_asset(const UDroneFrameAsset* asset);

	FDroneMotor convert_motor_asset(const UDroneMotorAsset* asset);

	FDroneBattery convert_battery_asset(const UDroneBatteryAsset* asset);
}
