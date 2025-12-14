#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DronePlayerController.generated.h"

/**
 *
 */
UCLASS()
class DRONESIMULATORGAME_API ADronePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void toggle_calibration_menu();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void show_calibration_menu();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void hide_calibration_menu();

private:
    TSharedPtr<class SDroneControllerCalibrationWidget> calibration_widget;
};
