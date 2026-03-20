#include "TypeActions/AssetTypeActions_DronePropeller.h"
#include "DroneSimulatorGame/Assets/DronePropellerAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_DronePropeller::GetName() const
{
	return LOCTEXT("FDronePropellerAssetName", "Propeller");
}

FColor FAssetTypeActions_DronePropeller::GetTypeColor() const
{
	// oklch(0.6529 0.1623 6.804)
	return FColor::FromHex("df5e7f");
}

UClass* FAssetTypeActions_DronePropeller::GetSupportedClass() const
{
	return UDronePropellerAsset::StaticClass();
}

uint32 FAssetTypeActions_DronePropeller::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
