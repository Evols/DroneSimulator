#include "DroneSimulatorEditor/Private/Factories/DroneMotorFactory.h"

#include "DroneSimulatorGame/Assets/DroneMotorAsset.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"


#define LOCTEXT_NAMESPACE "DroneMotorFactory"

UDroneMotorFactory::UDroneMotorFactory()
{
	SupportedClass = UDroneMotorAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDroneMotorFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDroneMotorAsset>(InParent, InClass, InName, Flags);
}

uint32 UDroneMotorFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDroneMotorFactory::GetDisplayName() const
{
	return LOCTEXT("DroneMotorFactoryDisplayName", "Motor");
}

#undef LOCTEXT_NAMESPACE
