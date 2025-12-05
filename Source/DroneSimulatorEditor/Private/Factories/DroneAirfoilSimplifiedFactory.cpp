#include "DroneSimulatorEditor/Private/Factories/DroneAirfoilSimplifiedFactory.h"
#include "DroneSimulator/Assets/DroneAirfoilAsset.h"
#include "Developer/AssetTools/Public/IAssetTools.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "DroneAirfoilSimplifiedFactory"

UDroneAirfoilSimplifiedFactory::UDroneAirfoilSimplifiedFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = false;
	SupportedClass = UDroneAirfoilAssetSimplified::StaticClass();
}

UObject* UDroneAirfoilSimplifiedFactory::FactoryCreateNew(UClass* in_class, UObject* in_parent, FName in_name, EObjectFlags flags, UObject* context, FFeedbackContext* warn)
{
	// Create the simplified airfoil asset with default values
	UDroneAirfoilAssetSimplified* new_asset = NewObject<UDroneAirfoilAssetSimplified>(in_parent, in_class, in_name, flags);

	// Default values are already set in the UPROPERTY declarations:
	// cl_k_rad = 5.5
	// cd_0 = 0.022
	// cd_k = 0.03

	if (warn)
	{
		warn->Logf(ELogVerbosity::Display, TEXT("Created new simplified airfoil with default parameters (cl_k_rad=5.5, cd_0=0.022, cd_k=0.03)"));
	}

	return new_asset;
}

uint32 UDroneAirfoilSimplifiedFactory::GetMenuCategories() const
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Drone", LOCTEXT("AssetCategoryName", "Drone"));
}

FText UDroneAirfoilSimplifiedFactory::GetDisplayName() const
{
	return LOCTEXT("DroneAirfoilSimplifiedFactoryDisplayName", "Airfoil (Simplified)");
}

#undef LOCTEXT_NAMESPACE

