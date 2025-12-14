#include "TypeActions/AssetTypeActions_FlightRecord.h"
#include "DroneSimulatorGame/Assets/FlightRecordAsset.h"
#include "DroneSimulatorEditor/Public/DroneSimulatorEditor.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_FlightRecord::GetName() const
{
	return LOCTEXT("FFlightRecordAssetName", "Flight Record");
}

FColor FAssetTypeActions_FlightRecord::GetTypeColor() const
{
	// oklch(0.827 0.078 40.029) - Orange/amber color for flight records
	return FColor::FromHex("f9d0bc");
}

UClass* FAssetTypeActions_FlightRecord::GetSupportedClass() const
{
	return UFlightRecordAsset::StaticClass();
}

uint32 FAssetTypeActions_FlightRecord::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FAssetTypeActions_FlightRecord::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Object : InObjects)
	{
		if (UFlightRecordAsset* FlightRecord = Cast<UFlightRecordAsset>(Object))
		{
			FDroneSimulatorEditorModule& EditorModule = FModuleManager::LoadModuleChecked<FDroneSimulatorEditorModule>("DroneSimulatorEditor");
			EditorModule.open_flight_record_in_timeline(FlightRecord);
		}
	}
}

#undef LOCTEXT_NAMESPACE
