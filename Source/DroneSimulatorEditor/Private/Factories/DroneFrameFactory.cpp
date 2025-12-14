#include "DroneSimulatorEditor/Private/Factories/DroneFrameFactory.h"

#include "DroneSimulatorGame/Assets/DroneFrameAsset.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"


#define LOCTEXT_NAMESPACE "DroneFrameFactory"

UDroneFrameFactory::UDroneFrameFactory()
{
	SupportedClass = UDroneFrameAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDroneFrameFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDroneFrameAsset>(InParent, InClass, InName, Flags);
}

uint32 UDroneFrameFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDroneFrameFactory::GetDisplayName() const
{
	return LOCTEXT("DroneFrameFactoryDisplayName", "Frame");
}

#undef LOCTEXT_NAMESPACE
