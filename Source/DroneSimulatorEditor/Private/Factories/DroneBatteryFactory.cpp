#include "DroneSimulatorEditor/Private/Factories/DroneBatteryFactory.h"

#include "DroneSimulatorGame/Assets/DroneBatteryAsset.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"


#define LOCTEXT_NAMESPACE "DroneBatteryFactory"

UDroneBatteryFactory::UDroneBatteryFactory()
{
	SupportedClass = UDroneBatteryAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDroneBatteryFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDroneBatteryAsset>(InParent, InClass, InName, Flags);
}

uint32 UDroneBatteryFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDroneBatteryFactory::GetDisplayName() const
{
	return LOCTEXT("DroneBatteryFactoryDisplayName", "Battery");
}

#undef LOCTEXT_NAMESPACE
