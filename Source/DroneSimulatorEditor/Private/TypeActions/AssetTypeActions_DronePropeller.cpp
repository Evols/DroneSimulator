#include "TypeActions/AssetTypeActions_DronePropeller.h"
#include "DroneSimulator/Assets/DronePropellerAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_DronePropellerBemt::GetName() const
{
	return LOCTEXT("FDronePropellerBemtAssetName", "Propeller (BEMT)");
}

FColor FAssetTypeActions_DronePropellerBemt::GetTypeColor() const
{
	// oklch(0.827 0.078 6.804)
	return FColor::FromHex("f3b2be");
}

UClass* FAssetTypeActions_DronePropellerBemt::GetSupportedClass() const
{
	return UDronePropellerBemtAsset::StaticClass();
}

uint32 FAssetTypeActions_DronePropellerBemt::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

FText FAssetTypeActions_DronePropellerSimplified::GetName() const
{
	return LOCTEXT("FDronePropellerSimplifiedAssetName", "Propeller (Simplified)");
}

FColor FAssetTypeActions_DronePropellerSimplified::GetTypeColor() const
{
	// oklch(0.827 0.078 6.804)
	return FColor::FromHex("f3b2be");
}

UClass* FAssetTypeActions_DronePropellerSimplified::GetSupportedClass() const
{
	return UDronePropellerSimplifiedAsset::StaticClass();
}

uint32 FAssetTypeActions_DronePropellerSimplified::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
