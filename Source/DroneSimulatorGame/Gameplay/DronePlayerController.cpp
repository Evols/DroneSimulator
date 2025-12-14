#include "DronePlayerController.h"
#include "UI/SDroneControllerCalibrationWidget.h"
#include "Widgets/SWeakWidget.h"
#include "Engine/Engine.h"

void ADronePlayerController::BeginPlay()
{
    Super::BeginPlay();
}

void ADronePlayerController::toggle_calibration_menu()
{
    if (calibration_widget.IsValid())
    {
        hide_calibration_menu();
    }
    else
    {
        show_calibration_menu();
    }
}

void ADronePlayerController::show_calibration_menu()
{
    if (!calibration_widget.IsValid())
    {
        SAssignNew(calibration_widget, SDroneControllerCalibrationWidget)
            .drone_player_controller(this);
    }

    if (calibration_widget.IsValid())
    {
        GEngine->GameViewport->AddViewportWidgetContent(
            SNew(SWeakWidget).PossiblyNullContent(calibration_widget.ToSharedRef())
        );

        bShowMouseCursor = true;
        SetInputMode(FInputModeUIOnly());
    }
}

void ADronePlayerController::hide_calibration_menu()
{
    if (calibration_widget.IsValid())
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(calibration_widget.ToSharedRef());
        calibration_widget.Reset();

        bShowMouseCursor = false;
        SetInputMode(FInputModeGameOnly());
    }
}
