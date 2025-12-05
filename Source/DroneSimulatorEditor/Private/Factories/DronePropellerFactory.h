#pragma once

#include "Runtime/Core/Public/CoreMinimal.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"

#include "DronePropellerFactory.generated.h"

UCLASS()
class UDronePropellerBemtFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDronePropellerBemtFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	virtual uint32 GetMenuCategories() const override;

	virtual FText GetDisplayName() const override;
};

UCLASS()
class UDronePropellerSimplifiedFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDronePropellerSimplifiedFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	virtual uint32 GetMenuCategories() const override;

	virtual FText GetDisplayName() const override;
};
