#include "TypeActions/AssetTypeActions_DroneAirfoil.h"
#include "DroneSimulatorGame/Assets/DroneAirfoilAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_DroneAirfoil::GetName() const
{
	return LOCTEXT("FDroneXfoilDataAssetName", "Airfoil");
}

FColor FAssetTypeActions_DroneAirfoil::GetTypeColor() const
{
	// oklch(0.827 0.078 161.22)
	return FColor::FromHex("99d7b6");
}

UClass* FAssetTypeActions_DroneAirfoil::GetSupportedClass() const
{
	return UDroneAirfoilAssetTable::StaticClass();
}

uint32 FAssetTypeActions_DroneAirfoil::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
