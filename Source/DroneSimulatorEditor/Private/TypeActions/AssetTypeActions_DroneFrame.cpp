#include "TypeActions/AssetTypeActions_DroneFrame.h"
#include "DroneSimulator/Assets/DroneFrameAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_DroneFrame::GetName() const
{
	return LOCTEXT("FDroneFrameAssetName", "Frame");
}

FColor FAssetTypeActions_DroneFrame::GetTypeColor() const
{
	// oklch(0.827 0.078 105.773)
	return FColor::FromHex("cdca8f");
}

UClass* FAssetTypeActions_DroneFrame::GetSupportedClass() const
{
	return UDroneFrameAsset::StaticClass();
}

uint32 FAssetTypeActions_DroneFrame::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
