#include "DroneSimulatorEditor/Private/Factories/DronePropellerFactory.h"

#include "DroneSimulatorGame/Assets/DronePropellerAsset.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "DroneSimulatorGame/Assets/Conversion.h"


#define LOCTEXT_NAMESPACE "DronePropellerFactory"

UDronePropellerFactory::UDronePropellerFactory()
{
	SupportedClass = UDronePropellerAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDronePropellerFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDronePropellerAsset>(InParent, InClass, InName, Flags);
}

uint32 UDronePropellerFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDronePropellerFactory::GetDisplayName() const
{
	return LOCTEXT("DronePropellerBemtFactoryDisplayName", "Propeller (BEMT)");
}

#undef LOCTEXT_NAMESPACE
