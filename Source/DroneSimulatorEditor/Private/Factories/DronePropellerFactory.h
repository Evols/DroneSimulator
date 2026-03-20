#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"

#include "DronePropellerFactory.generated.h"

UCLASS()
class UDronePropellerFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDronePropellerFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	virtual uint32 GetMenuCategories() const override;

	virtual FText GetDisplayName() const override;
};
