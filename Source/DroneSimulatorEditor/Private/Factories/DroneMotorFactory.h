#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"

#include "DroneMotorFactory.generated.h"

UCLASS()
class UDroneMotorFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDroneMotorFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	virtual uint32 GetMenuCategories() const override;

	virtual FText GetDisplayName() const override;
};
