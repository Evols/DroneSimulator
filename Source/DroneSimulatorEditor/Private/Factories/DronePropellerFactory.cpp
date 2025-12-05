#include "DroneSimulatorEditor/Private/Factories/DronePropellerFactory.h"

#include "DroneSimulator/Assets/DronePropellerAsset.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"


#define LOCTEXT_NAMESPACE "DronePropellerFactory"

UDronePropellerBemtFactory::UDronePropellerBemtFactory()
{
	SupportedClass = UDronePropellerAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDronePropellerBemtFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDronePropellerAsset>(InParent, InClass, InName, Flags);
}

uint32 UDronePropellerBemtFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDronePropellerBemtFactory::GetDisplayName() const
{
	return LOCTEXT("DronePropellerBemtFactoryDisplayName", "Propeller (BEMT)");
}

UDronePropellerSimplifiedFactory::UDronePropellerSimplifiedFactory()
{
	SupportedClass = UDronePropellerSimplifiedAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDronePropellerSimplifiedFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDronePropellerSimplifiedAsset>(InParent, InClass, InName, Flags);
}

uint32 UDronePropellerSimplifiedFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDronePropellerSimplifiedFactory::GetDisplayName() const
{
	return LOCTEXT("DronePropellerSimplifiedFactoryDisplayName", "Propeller (Simplified)");
}

#undef LOCTEXT_NAMESPACE
