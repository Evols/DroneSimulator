#include "TypeActions/AssetTypeActions_DroneAirfoilSimplified.h"
#include "DroneSimulator/Assets/DroneAirfoilAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_DroneAirfoilSimplified::GetName() const
{
	return LOCTEXT("FDroneAirfoilSimplifiedAssetName", "Airfoil (Simplified)");
}

FColor FAssetTypeActions_DroneAirfoilSimplified::GetTypeColor() const
{
	// Same color as table airfoil but slightly different shade
	// oklch(0.827 0.078 161.22)
	return FColor::FromHex("7dc7a0");
}

UClass* FAssetTypeActions_DroneAirfoilSimplified::GetSupportedClass() const
{
	return UDroneAirfoilAssetSimplified::StaticClass();
}

uint32 FAssetTypeActions_DroneAirfoilSimplified::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE


