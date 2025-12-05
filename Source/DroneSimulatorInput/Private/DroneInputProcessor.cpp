#include "DroneInputProcessor.h"
#include "DroneInputSubsystem.h"
#include "Framework/Application/SlateApplication.h"

FDroneInputProcessor::FDroneInputProcessor(UDroneInputSubsystem* InManager)
    : input_subsystem(InManager)
{
}

FDroneInputProcessor::~FDroneInputProcessor()
{
}

void FDroneInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FDroneInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
    if (input_subsystem)
    {
        input_subsystem->handle_key_down(InKeyEvent.GetInputDeviceId().GetId(), InKeyEvent.GetKey());
    }
    return false; // Don't consume, let others use it too
}

bool FDroneInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
    if (input_subsystem)
    {
        input_subsystem->handle_key_up(InKeyEvent.GetInputDeviceId().GetId(), InKeyEvent.GetKey());
    }
    return false;
}

bool FDroneInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
    if (input_subsystem)
    {
        input_subsystem->handle_raw_analog_input(InAnalogInputEvent.GetInputDeviceId().GetId(), InAnalogInputEvent.GetKey(), InAnalogInputEvent.GetAnalogValue());
    }
    return false;
}

bool FDroneInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    return false;
}
