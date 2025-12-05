#pragma once

#include "AssetTypeActions_Base.h"

class FAssetTypeActions_FlightRecord : public FAssetTypeActions_Base
{
public:

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	
	// Enable double-click to open in timeline
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
};
