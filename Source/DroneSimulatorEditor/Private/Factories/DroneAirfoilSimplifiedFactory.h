#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DroneAirfoilSimplifiedFactory.generated.h"

UCLASS()
class UDroneAirfoilSimplifiedFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDroneAirfoilSimplifiedFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* in_class, UObject* in_parent, FName in_name, EObjectFlags flags, UObject* context, FFeedbackContext* warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
	// End of UFactory interface
};

