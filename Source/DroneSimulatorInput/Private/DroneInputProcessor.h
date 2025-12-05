#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"

class UDroneInputSubsystem;

class FDroneInputProcessor : public IInputProcessor
{
public:
    FDroneInputProcessor(UDroneInputSubsystem* InManager);
    virtual ~FDroneInputProcessor();

    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

    // Key down/up
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

    // Analog
    virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;

    // Mouse
    virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

private:
    UDroneInputSubsystem* input_subsystem;
};
