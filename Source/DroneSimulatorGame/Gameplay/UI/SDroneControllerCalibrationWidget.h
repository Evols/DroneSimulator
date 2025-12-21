#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "DroneSimulatorInput/Public/DroneInputTypes.h"

class ADronePlayerController;
class UDroneInputSubsystem;
class UDroneInputLocalPlayerSubsystem;
class FDroneInputCalibrator;
class SDroneCalibrationDeviceList;
class SDroneCalibrationInteraction;
class STextComboBox;

// Main Widget
class DRONESIMULATORGAME_API SDroneControllerCalibrationWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDroneControllerCalibrationWidget)
        : _drone_player_controller(nullptr)
    {}
        SLATE_ARGUMENT(ADronePlayerController*, drone_player_controller)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
    TObjectPtr<ADronePlayerController> drone_player_controller;

    TWeakObjectPtr<UDroneInputSubsystem> input_subsystem;
    TWeakObjectPtr<UDroneInputLocalPlayerSubsystem> local_player_subsystem;

    // Components
    TSharedPtr<SDroneCalibrationDeviceList> device_list_widget;

    // UI State
    int32 selected_device_id = -1;

    // Calibration Flow
    TArray<EDroneInputAxis> axes_to_map;
    int32 current_axis_index = 0;
    bool is_calibrating = false;
    bool is_measuring_limits = false;
    bool is_calibration_success = false;
    TArray<TSharedPtr<FString>> channel_order_options;
    TSharedPtr<FString> selected_channel_order;
    FString custom_channel_order = "AETR";
    TWeakPtr<FDroneInputCalibrator> calibrator;
    TSet<FName> mapped_device_axes;

    // Device List Data
    TArray<TSharedPtr<FDroneInputDevice>> device_list_items;

    // Helpers
    void refresh_device_list_data(const TArray<FDroneInputDevice>& devices);
    void on_device_selected(TSharedPtr<FDroneInputDevice> device);

    FReply on_start_calibration_clicked();
    FReply on_next_step_clicked();
    FReply on_cancel_clicked();
    FReply on_remove_calibration_clicked();
    void auto_map_aux_channels();

    // Functional Getters
    FText get_instruction_text() const;
    bool is_start_enabled() const;
    bool is_next_enabled() const;
    bool is_remove_enabled() const;
    bool is_calibrating_attr() const { return is_calibrating; }
    bool is_measuring_limits_attr() const { return is_measuring_limits; }
    EVisibility get_device_list_visibility() const;

    // Callbacks
    void on_axis_mapped(EDroneInputAxis game_axis, FName device_axis_name);

    // Device tracking
    TArray<int32> device_ids_snapshot;
};
