#include "TypeActions/AssetTypeActions_DroneBattery.h"
#include "DroneSimulator/Assets/DroneBatteryAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_DroneBattery::GetName() const
{
	return LOCTEXT("FDroneBatteryAssetName", "Battery");
}

FColor FAssetTypeActions_DroneBattery::GetTypeColor() const
{
	// oklch(0.827 0.078 280.029)
	return FColor::FromHex("bcc1f9");
}

UClass* FAssetTypeActions_DroneBattery::GetSupportedClass() const
{
	return UDroneBatteryAsset::StaticClass();
}

uint32 FAssetTypeActions_DroneBattery::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
