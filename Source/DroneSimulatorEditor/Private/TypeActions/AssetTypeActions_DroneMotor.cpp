#include "TypeActions/AssetTypeActions_DroneMotor.h"
#include "DroneSimulator/Assets/DroneMotorAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_DroneMotor::GetName() const
{
	return LOCTEXT("FDroneMotorAssetName", "Motor");
}

FColor FAssetTypeActions_DroneMotor::GetTypeColor() const
{
	// oklch(0.827 0.078 186.362)
	return FColor::FromHex("8ad7ce");
}

UClass* FAssetTypeActions_DroneMotor::GetSupportedClass() const
{
	return UDroneMotorAsset::StaticClass();
}

uint32 FAssetTypeActions_DroneMotor::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
