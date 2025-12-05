#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DroneAirfoilFactory.generated.h"

UCLASS()
class UDroneAirfoilFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDroneAirfoilFactory();

	// UFactory interface
	virtual UObject* FactoryCreateFile(UClass* in_class, UObject* in_parent, FName in_name, EObjectFlags flags, const FString& filename, const TCHAR* parms, FFeedbackContext* warn, bool& out_cancel_operation) override;
	virtual bool FactoryCanImport(const FString& filename) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
	// End of UFactory interface
};
