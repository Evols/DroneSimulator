#include "SDroneControllerCalibrationWidget.h"

#include "DroneSimulatorGame/Gameplay/DronePlayerController.h"
#include "DroneSimulatorInput/Public/DroneInputLocalPlayerSubsystem.h"
#include "DroneSimulatorInput/Public/DroneInputSubsystem.h"
#include "DroneSimulatorInput/Public/DroneInputCalibrator.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/STableRow.h"

namespace
{
	FString get_axis_label(EDroneInputAxis axis)
	{
		switch (axis)
		{
			case EDroneInputAxis::Throttle: return TEXT("Throttle");
			case EDroneInputAxis::Yaw: return TEXT("Yaw");
			case EDroneInputAxis::Pitch: return TEXT("Pitch");
			case EDroneInputAxis::Roll: return TEXT("Roll");
			default: break;
		}

		const int32 aux_start = static_cast<int32>(EDroneInputAxis::Aux1);
		const int32 aux_index = static_cast<int32>(axis) - aux_start;
		if (aux_index >= 0 && aux_index < drone_input::aux_axis_count)
		{
			return FString::Printf(TEXT("Aux %d"), aux_index + 1);
		}

		return TEXT("Unknown");
	}

	TArray<EDroneInputAxis> get_config_axes()
	{
		TArray<EDroneInputAxis> axes = {
			EDroneInputAxis::Throttle,
			EDroneInputAxis::Yaw,
			EDroneInputAxis::Pitch,
			EDroneInputAxis::Roll
		};

		for (int32 aux_index = 0; aux_index < drone_input::aux_axis_count; ++aux_index)
		{
			axes.Add(static_cast<EDroneInputAxis>(
				static_cast<int32>(EDroneInputAxis::Aux1) + aux_index));
		}

		return axes;
	}

	TArray<EDroneInputAxis> get_default_channel_order()
	{
		return {
			EDroneInputAxis::Roll,
			EDroneInputAxis::Pitch,
			EDroneInputAxis::Throttle,
			EDroneInputAxis::Yaw
		};
	}

	bool try_parse_channel_order(const FString& order, TArray<EDroneInputAxis>& out_axes)
	{
		out_axes.Reset();

		for (int32 index = 0; index < order.Len(); ++index)
		{
			const TCHAR channel = FChar::ToUpper(order[index]);
			EDroneInputAxis axis = EDroneInputAxis::None;

			switch (channel)
			{
				case 'A': axis = EDroneInputAxis::Roll; break;
				case 'E': axis = EDroneInputAxis::Pitch; break;
				case 'T': axis = EDroneInputAxis::Throttle; break;
				case 'R': axis = EDroneInputAxis::Yaw; break;
				default: break;
			}

			if (axis == EDroneInputAxis::None || out_axes.Contains(axis))
			{
				continue;
			}

			out_axes.Add(axis);
			if (out_axes.Num() == 4)
			{
				return true;
			}
		}

		return false;
	}

	TArray<EDroneInputAxis> resolve_channel_order_axes(const TSharedPtr<FString>& selected_order,
		const FString& custom_order)
	{
		if (!selected_order.IsValid())
		{
			return get_default_channel_order();
		}

		const bool use_custom = selected_order->Equals(TEXT("Custom"));
		const FString& order = use_custom ? custom_order : *selected_order;
		TArray<EDroneInputAxis> axes;
		if (try_parse_channel_order(order, axes))
		{
			return axes;
		}

		return get_default_channel_order();
	}

	bool is_hex_string(const FString& text)
	{
		if (text.IsEmpty())
		{
			return false;
		}

		for (int32 index = 0; index < text.Len(); ++index)
		{
			if (!FChar::IsHexDigit(text[index]))
			{
				return false;
			}
		}

		return true;
	}

	bool try_parse_hid_axis_name(const FString& axis_name, int32& out_usage_page, int32& out_usage)
	{
		if (!axis_name.StartsWith(TEXT("HID_")))
		{
			return false;
		}

		const FString trimmed = axis_name.Mid(4);
		const int32 separator_index = trimmed.Find(TEXT("_"));
		if (separator_index == INDEX_NONE)
		{
			return false;
		}

		const FString page_text = trimmed.Left(separator_index);
		const FString usage_text = trimmed.Mid(separator_index + 1);
		if (!is_hex_string(page_text) || !is_hex_string(usage_text))
		{
			return false;
		}

		out_usage_page = FParse::HexNumber(*page_text);
		out_usage = FParse::HexNumber(*usage_text);
		return true;
	}

	TArray<FName> get_sorted_axis_names(const FDroneInputDevice& device)
	{
		TArray<FName> axis_names = device.axis_names;
		if (axis_names.Num() == 0)
		{
			device.raw_axes.GetKeys(axis_names);
		}

		axis_names.Sort([](const FName& left, const FName& right)
		{
			int32 left_page = 0;
			int32 left_usage = 0;
			int32 right_page = 0;
			int32 right_usage = 0;

			const FString left_name = left.ToString();
			const FString right_name = right.ToString();

			const bool left_hid = try_parse_hid_axis_name(left_name, left_page, left_usage);
			const bool right_hid = try_parse_hid_axis_name(right_name, right_page, right_usage);

			if (left_hid && right_hid)
			{
				if (left_page != right_page)
				{
					return left_page < right_page;
				}
				if (left_usage != right_usage)
				{
					return left_usage < right_usage;
				}
			}
			else if (left_hid != right_hid)
			{
				return left_hid;
			}

			return left_name < right_name;
		});

		return axis_names;
	}
}

// ============================================================================
// Component: Device Selection List
// ============================================================================

class SDroneCalibrationDeviceList : public SCompoundWidget
{
public:
    DECLARE_DELEGATE_OneParam(FOnDeviceSelected, TSharedPtr<FDroneInputDevice>);

    SLATE_BEGIN_ARGS(SDroneCalibrationDeviceList)
        : _ListItemsSource(nullptr)
        , _SelectedDeviceId(-1)
    {}
        SLATE_ARGUMENT(const TArray<TSharedPtr<FDroneInputDevice>>*, ListItemsSource)
        SLATE_ATTRIBUTE(int32, SelectedDeviceId)
        SLATE_ARGUMENT(UDroneInputSubsystem*, InputSubsystem)
        SLATE_EVENT(FOnDeviceSelected, OnDeviceSelected)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        list_items_source = InArgs._ListItemsSource;
        selected_device_id = InArgs._SelectedDeviceId;
        input_subsystem = InArgs._InputSubsystem;
        on_device_selected = InArgs._OnDeviceSelected;

        ChildSlot
        [
            SAssignNew(list_view, SListView<TSharedPtr<FDroneInputDevice>>)
            .ListItemsSource(list_items_source)
            .OnGenerateRow(this, &SDroneCalibrationDeviceList::on_generate_row)
            .SelectionMode(ESelectionMode::None)
        ];
    }

    void RefreshList()
    {
        if (list_view.IsValid())
        {
            list_view->RequestListRefresh();
        }
    }

private:
    const TArray<TSharedPtr<FDroneInputDevice>>* list_items_source = nullptr;
    TAttribute<int32> selected_device_id;
    UDroneInputSubsystem* input_subsystem = nullptr;
    FOnDeviceSelected on_device_selected;
    TSharedPtr<SListView<TSharedPtr<FDroneInputDevice>>> list_view;

    TSharedRef<ITableRow> on_generate_row(TSharedPtr<FDroneInputDevice> item, const TSharedRef<STableViewBase>& owner_table)
    {
        return SNew(STableRow<TSharedPtr<FDroneInputDevice>>, owner_table)
        [
            SNew(SBox)
            .Padding(5)
            [
                SNew(SButton)
                .OnClicked_Lambda([this, item]()
                {
                    if (item.IsValid())
                    {
                        on_device_selected.ExecuteIfBound(item);
                    }
                    return FReply::Handled();
                })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(item->device_name))
                        .ColorAndOpacity_Lambda([this, item]()
                        {
                            const int32 current_selected = selected_device_id.Get();
                            return (item.IsValid() && item->device_id == current_selected)
                                ? FSlateColor(FLinearColor::Green)
                                : FSlateColor(FLinearColor::White);
                        })
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(10, 0, 0, 0)
                    [
                        SNew(STextBlock)
                        .Text_Lambda([this, item]()
                        {
                            const bool is_calibrated = input_subsystem && item.IsValid() && input_subsystem->is_device_calibrated(item->device_id);
                            return is_calibrated ? FText::FromString("[Calibrated]") : FText::FromString("[Not calibrated]");
                        })
                        .ColorAndOpacity_Lambda([this, item]()
                        {
                            const bool is_calibrated = input_subsystem && item.IsValid() && input_subsystem->is_device_calibrated(item->device_id);
                            return is_calibrated ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Gray);
                        })
                    ]
                ]
            ]
        ];
    }
};

// ============================================================================
// Component: Axis Configuration List
// ============================================================================

class SDroneAxisConfigList : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDroneAxisConfigList)
        : _SelectedDeviceId(-1)
    {}
        SLATE_ATTRIBUTE(int32, SelectedDeviceId)
        SLATE_ARGUMENT(UDroneInputSubsystem*, InputSubsystem)
        SLATE_ATTRIBUTE(bool, IsCalibrating)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        selected_device = InArgs._SelectedDeviceId;
        input_subsystem = InArgs._InputSubsystem;
        is_calibrating = InArgs._IsCalibrating;

        TSharedRef<SVerticalBox> axis_list = SNew(SVerticalBox)
            .Visibility_Lambda([this]()
            {
                if (is_calibrating.Get()) return EVisibility::Collapsed;
                if (selected_device.Get() == -1) return EVisibility::Collapsed;
                if (input_subsystem && !input_subsystem->is_device_calibrated(selected_device.Get())) return EVisibility::Collapsed;
                return EVisibility::Visible;
            });

        for (EDroneInputAxis axis : get_config_axes())
        {
            axis_list->AddSlot()
                .AutoHeight()
                [ make_axis_row(axis, get_axis_label(axis)) ];
        }

        ChildSlot
        [
            axis_list
        ];
    }

private:
    TAttribute<int32> selected_device;
    UDroneInputSubsystem* input_subsystem = nullptr;
    TAttribute<bool> is_calibrating;

    TSharedRef<SWidget> make_axis_row(EDroneInputAxis axis, const FString& label)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(5)
            [
                SNew(STextBlock)
                .Text(FText::FromString(label))
                .MinDesiredWidth(60.f)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(5)
            [
                SNew(SCheckBox)
                .IsChecked_Lambda([this, axis]()
                {
                    if (input_subsystem && selected_device.Get() != -1)
                    {
                        return input_subsystem->is_axis_inverted(selected_device.Get(), axis) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                    }
                    return ECheckBoxState::Unchecked;
                })
                .OnCheckStateChanged_Lambda([this, axis](ECheckBoxState state)
                {
                    if (input_subsystem && selected_device.Get() != -1)
                    {
                        input_subsystem->set_axis_inverted(selected_device.Get(), axis, state == ECheckBoxState::Checked);
                    }
                })
                [
                    SNew(STextBlock).Text(FText::FromString("Inverted"))
                ]
            ];
    }
};

// ============================================================================
// Component: Calibration Interaction (Instructions + Buttons)
// ============================================================================

class SDroneCalibrationInteraction : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDroneCalibrationInteraction)
        : _IsCalibrating(false)
        , _IsMeasuringLimits(false)
    {}
        SLATE_ATTRIBUTE(FText, InstructionText)
        SLATE_ATTRIBUTE(bool, IsStartEnabled)
        SLATE_ATTRIBUTE(bool, IsNextEnabled)
        SLATE_ATTRIBUTE(bool, IsCalibrating)
        SLATE_ATTRIBUTE(bool, IsMeasuringLimits)

        SLATE_EVENT(FOnClicked, OnStartClicked)
        SLATE_EVENT(FOnClicked, OnNextClicked)
        SLATE_EVENT(FOnClicked, OnCancelClicked)
        SLATE_EVENT(FOnClicked, OnRemoveCalibrationClicked)
        SLATE_ATTRIBUTE(bool, IsRemoveEnabled)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        TAttribute<FText> instruction_text = InArgs._InstructionText;
        TAttribute<bool> is_start_enabled = InArgs._IsStartEnabled;
        TAttribute<bool> is_next_enabled = InArgs._IsNextEnabled;
        TAttribute<bool> is_calibrating = InArgs._IsCalibrating;
        TAttribute<bool> is_measuring_limits = InArgs._IsMeasuringLimits;

        ChildSlot
        [
            SNew(SVerticalBox)

            // Instructions
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 20)
            [
                SNew(STextBlock)
                .Text(instruction_text)
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
                .AutoWrapText(true)
                .Justification(ETextJustify::Center)
            ]

            // Buttons
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                // Start Button
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(5)
                [
                    SNew(SButton)
                    .OnClicked(InArgs._OnStartClicked)
                    .IsEnabled(is_start_enabled)
                    .Visibility_Lambda([is_calibrating]() { return !is_calibrating.Get() ? EVisibility::Visible : EVisibility::Collapsed; })
                    [
                        SNew(STextBlock).Text(FText::FromString("Start Calibration"))
                    ]
                ]

                // Next / Done Button
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(5)
                [
                    SNew(SButton)
                    .OnClicked(InArgs._OnNextClicked)
                    .IsEnabled(is_next_enabled)
                    .Visibility_Lambda([is_calibrating]() { return is_calibrating.Get() ? EVisibility::Visible : EVisibility::Collapsed; })
                    [
                        SNew(STextBlock).Text_Lambda([is_measuring_limits]()
                        {
                            return is_measuring_limits.Get() ? FText::FromString("Done") : FText::FromString("Next");
                        })
                    ]
                ]

                // Cancel Button
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(5)
                [
                    SNew(SButton)
                    .OnClicked(InArgs._OnCancelClicked)
                    [
                        SNew(STextBlock).Text(FText::FromString("Cancel"))
                    ]
                ]

                // Remove Calibration Button
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(5)
                [
                    SNew(SButton)
                    .OnClicked(InArgs._OnRemoveCalibrationClicked)
                    .IsEnabled(InArgs._IsRemoveEnabled)
                    .Visibility_Lambda([is_calibrating]() { return !is_calibrating.Get() ? EVisibility::Visible : EVisibility::Collapsed; })
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Remove Calibration"))
                        .ColorAndOpacity(FSlateColor(FLinearColor::Red))
                    ]
                ]
            ]
        ];
    }
};

// ============================================================================
// SDroneControllerCalibrationWidget
// ============================================================================

void SDroneControllerCalibrationWidget::Construct(const FArguments& InArgs)
{
    this->drone_player_controller = InArgs._drone_player_controller;

    if (GEngine)
    {
        this->input_subsystem = GEngine->GetEngineSubsystem<UDroneInputSubsystem>();
    }

    if (this->drone_player_controller != nullptr)
    {
        if (ULocalPlayer* local_player = this->drone_player_controller->GetLocalPlayer())
        {
            this->local_player_subsystem = local_player->GetSubsystem<UDroneInputLocalPlayerSubsystem>();
        }
    }

    if (this->input_subsystem.IsValid())
    {
        this->calibrator = this->input_subsystem->get_calibrator();
        this->input_subsystem->on_axis_mapped_native.AddSP(this, &SDroneControllerCalibrationWidget::on_axis_mapped);
    }

    if (channel_order_options.Num() == 0)
    {
        channel_order_options = {
            MakeShared<FString>(TEXT("AETR")),
            MakeShared<FString>(TEXT("TAER")),
            MakeShared<FString>(TEXT("ETAR")),
            MakeShared<FString>(TEXT("AERT")),
            MakeShared<FString>(TEXT("ATRE")),
            MakeShared<FString>(TEXT("TARE")),
            MakeShared<FString>(TEXT("RETA")),
            MakeShared<FString>(TEXT("RTAE")),
            MakeShared<FString>(TEXT("REAT")),
            MakeShared<FString>(TEXT("Custom"))
        };
        selected_channel_order = channel_order_options[0];
    }

    ChildSlot
    [
        SNew(SBox)
        .WidthOverride(300.f)
        [
            SNew(SBorder)
            .Padding(20)
            [
                SNew(SVerticalBox)

                // Title
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 20)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Controller Calibration"))
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
                    .Justification(ETextJustify::Center)
                ]

                // Device Selection List Component
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock).Text(FText::FromString("Select Device:"))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 10)
                .MaxHeight(200.f)
                [
                    SAssignNew(device_list_widget, SDroneCalibrationDeviceList)
                    .ListItemsSource(&device_list_items)
                    .SelectedDeviceId_Lambda([this]() { return selected_device_id; })
                    .InputSubsystem(this->input_subsystem.Get())
                    .OnDeviceSelected(this, &SDroneControllerCalibrationWidget::on_device_selected)
                    .Visibility(this, &SDroneControllerCalibrationWidget::get_device_list_visibility)
                ]

                // Axis Configuration List
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 10)
                [
                    SNew(SDroneAxisConfigList)
                    .SelectedDeviceId_Lambda([this]() { return selected_device_id; })
                    .InputSubsystem(this->input_subsystem.Get())
                    .IsCalibrating(this, &SDroneControllerCalibrationWidget::is_calibrating_attr)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 10)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock).Text(FText::FromString("Channel order"))
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(10, 0, 0, 0)
                    [
                        SNew(STextComboBox)
                        .OptionsSource(&channel_order_options)
                        .InitiallySelectedItem(selected_channel_order)
                        .OnSelectionChanged_Lambda([this](TSharedPtr<FString> new_selection, ESelectInfo::Type)
                        {
                            selected_channel_order = new_selection;
                        })
                        .IsEnabled_Lambda([this]()
                        {
                            return !is_calibrating;
                        })
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(10, 0, 0, 0)
                    [
                        SNew(SEditableTextBox)
                        .MinDesiredWidth(70.f)
                        .Text_Lambda([this]()
                        {
                            return FText::FromString(custom_channel_order);
                        })
                        .OnTextChanged_Lambda([this](const FText& new_text)
                        {
                            custom_channel_order = new_text.ToString().ToUpper();
                        })
                        .IsEnabled_Lambda([this]()
                        {
                            return !is_calibrating && selected_channel_order.IsValid()
                                && selected_channel_order->Equals(TEXT("Custom"));
                        })
                    ]
                ]

                // Interaction Component (Instructions + Buttons)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 20)
                [
                    SNew(SDroneCalibrationInteraction)
                    .InstructionText(this, &SDroneControllerCalibrationWidget::get_instruction_text)
                    .IsStartEnabled(this, &SDroneControllerCalibrationWidget::is_start_enabled)
                    .IsNextEnabled(this, &SDroneControllerCalibrationWidget::is_next_enabled)
                    .IsCalibrating(this, &SDroneControllerCalibrationWidget::is_calibrating_attr)
                    .IsMeasuringLimits(this, &SDroneControllerCalibrationWidget::is_measuring_limits_attr)
                    .OnStartClicked(this, &SDroneControllerCalibrationWidget::on_start_calibration_clicked)
                    .OnNextClicked(this, &SDroneControllerCalibrationWidget::on_next_step_clicked)
                    .OnCancelClicked(this, &SDroneControllerCalibrationWidget::on_cancel_clicked)
                    .OnRemoveCalibrationClicked(this, &SDroneControllerCalibrationWidget::on_remove_calibration_clicked)
                    .IsRemoveEnabled(this, &SDroneControllerCalibrationWidget::is_remove_enabled)
                ]
            ]
        ]
    ];

    // Initial population
    if (this->input_subsystem.IsValid())
    {
        const TArray<FDroneInputDevice> devices = this->input_subsystem->get_all_devices();
        refresh_device_list_data(devices);
    }
}

void SDroneControllerCalibrationWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    if (input_subsystem.IsValid())
    {
        TArray<FDroneInputDevice> devices = input_subsystem->get_all_devices();

        // Track changes by device id
        TArray<int32> new_ids;
        new_ids.Reserve(devices.Num());
        for (const auto& dev : devices)
        {
            new_ids.Add(dev.device_id);
        }
        new_ids.Sort();

        TArray<int32> sorted_snapshot = device_ids_snapshot;
        sorted_snapshot.Sort();

        if (new_ids != sorted_snapshot)
        {
            // If the selected device vanished, clear selection
            const bool selected_present = new_ids.Contains(selected_device_id);
            if (!selected_present)
            {
                selected_device_id = -1;
                is_calibrating = false;
                is_measuring_limits = false;
                current_axis_index = 0;
                is_calibration_success = false;
            }

            refresh_device_list_data(devices);
        }
    }
}

void SDroneControllerCalibrationWidget::refresh_device_list_data(const TArray<FDroneInputDevice>& devices)
{
    device_ids_snapshot.Empty(devices.Num());
    device_list_items.Empty(devices.Num());

    for (const FDroneInputDevice& device : devices)
    {
        device_ids_snapshot.Add(device.device_id);
        device_list_items.Add(MakeShared<FDroneInputDevice>(device));
    }

    if (device_list_widget.IsValid())
    {
        device_list_widget->RefreshList();
    }
}

void SDroneControllerCalibrationWidget::on_device_selected(TSharedPtr<FDroneInputDevice> device)
{
    if (device.IsValid())
    {
        selected_device_id = device->device_id;
        is_calibration_success = false;
        if (local_player_subsystem.IsValid())
        {
            local_player_subsystem->assign_device_to_local_player(selected_device_id);
        }
    }
}

FReply SDroneControllerCalibrationWidget::on_start_calibration_clicked()
{
    if (selected_device_id != -1)
    {
        if (TSharedPtr<FDroneInputCalibrator> calibrator_ptr = calibrator.Pin())
        {
            if (local_player_subsystem.IsValid())
            {
                local_player_subsystem->assign_device_to_local_player(selected_device_id);
            }
            is_calibrating = true;
            current_axis_index = 0;
            is_measuring_limits = false;
            is_calibration_success = false;
            mapped_device_axes.Empty();
            axes_to_map = resolve_channel_order_axes(selected_channel_order, custom_channel_order);
            calibrator_ptr->start_calibration(selected_device_id);

            // Start Mapping Phase
            if (axes_to_map.Num() > 0)
            {
                calibrator_ptr->start_mapping_axis(axes_to_map[0]);
            }
        }
    }
    return FReply::Handled();
}

void SDroneControllerCalibrationWidget::on_axis_mapped(EDroneInputAxis game_axis, FName device_axis_name)
{
    mapped_device_axes.Add(device_axis_name);
    this->current_axis_index++;
    if (this->current_axis_index < axes_to_map.Num())
    {
        if (TSharedPtr<FDroneInputCalibrator> calibrator_ptr = calibrator.Pin())
        {
            calibrator_ptr->start_mapping_axis(this->axes_to_map[this->current_axis_index]);
        }
    }
    else
    {
        this->is_measuring_limits = false;
        auto_map_aux_channels();
    }
}

void SDroneControllerCalibrationWidget::auto_map_aux_channels()
{
    if (!input_subsystem.IsValid() || selected_device_id == -1)
    {
        return;
    }

    const FDroneInputDevice device = input_subsystem->get_device(selected_device_id);
    if (device.device_id == -1)
    {
        return;
    }

    const TArray<FName> axis_names = get_sorted_axis_names(device);
    const int32 base_axis_count = 4;
    for (int32 axis_index = base_axis_count; axis_index < axis_names.Num(); ++axis_index)
    {
        const int32 aux_index = axis_index - base_axis_count;
        if (aux_index >= drone_input::aux_axis_count)
        {
            break;
        }

        const FName axis_name = axis_names[axis_index];
        if (mapped_device_axes.Contains(axis_name))
        {
            continue;
        }

        input_subsystem->set_axis_mapping(selected_device_id, axis_name,
            static_cast<EDroneInputAxis>(static_cast<int32>(EDroneInputAxis::Aux1) + aux_index));
    }
}

FReply SDroneControllerCalibrationWidget::on_next_step_clicked()
{
    if (!this->is_calibrating)
    {
        return FReply::Handled();
    }

    const bool mapping_complete = this->current_axis_index >= this->axes_to_map.Num();

    if (mapping_complete && !this->is_measuring_limits)
    {
        if (TSharedPtr<FDroneInputCalibrator> calibrator_ptr = calibrator.Pin())
        {
            calibrator_ptr->start_measuring_limits();
        }
        this->is_measuring_limits = true;
    }
    else if (mapping_complete && this->is_measuring_limits)
    {
        if (TSharedPtr<FDroneInputCalibrator> calibrator_ptr = calibrator.Pin())
        {
            calibrator_ptr->stop_calibration();
        }
        this->is_calibrating = false;
        this->is_measuring_limits = false;
        this->is_calibration_success = true;
    }
    return FReply::Handled();
}

FReply SDroneControllerCalibrationWidget::on_cancel_clicked()
{
    if (TSharedPtr<FDroneInputCalibrator> calibrator_ptr = calibrator.Pin())
    {
        calibrator_ptr->stop_calibration();
    }

    this->is_calibrating = false;
    this->is_measuring_limits = false;
    this->is_calibration_success = false;

    if (this->drone_player_controller != nullptr)
    {
        this->drone_player_controller->hide_calibration_menu();
    }

    // TODO

    return FReply::Handled();
}

FText SDroneControllerCalibrationWidget::get_instruction_text() const
{
    if (is_calibration_success)
    {
        return FText::FromString("Calibration Saved.");
    }

    if (selected_device_id == -1)
    {
        return FText::FromString("Select a device to start.");
    }

    if (!is_calibrating)
    {
        return FText::FromString(FString::Printf(TEXT("Selected Device ID: %d. Click Start to calibrate."), selected_device_id));
    }

    if (is_measuring_limits)
    {
        return FText::FromString("Measuring limits. Wiggle every stick through its full range (throttle min<->max), then click Done.");
    }
    else
    {
        if (current_axis_index < axes_to_map.Num())
        {
            const FString axis_name = get_axis_label(axes_to_map[current_axis_index]);
            return FText::FromString(FString::Printf(TEXT("Move stick for: %s"), *axis_name));
        }
        else
        {
            return FText::FromString("Mapping complete. Aux channels are auto-mapped to channels 5+. Center yaw/pitch/roll, set throttle to minimum, then click Next to record min/max ranges.");
        }
    }
}

bool SDroneControllerCalibrationWidget::is_start_enabled() const
{
    return selected_device_id != -1 && !is_calibrating;
}

bool SDroneControllerCalibrationWidget::is_next_enabled() const
{
    if (!is_calibrating) return false;

    const bool mapping_complete = current_axis_index >= axes_to_map.Num();
    return mapping_complete;
}

EVisibility SDroneControllerCalibrationWidget::get_device_list_visibility() const
{
    return is_calibrating ? EVisibility::HitTestInvisible : EVisibility::Visible;
}

FReply SDroneControllerCalibrationWidget::on_remove_calibration_clicked()
{
    if (selected_device_id != -1 && input_subsystem.IsValid())
    {
        input_subsystem->remove_device_calibration(selected_device_id);
        if (device_list_widget.IsValid())
        {
            device_list_widget->RefreshList();
        }
    }
    return FReply::Handled();
}

bool SDroneControllerCalibrationWidget::is_remove_enabled() const
{
    if (selected_device_id == -1 || is_calibrating || !input_subsystem.IsValid())
    {
        return false;
    }
    return input_subsystem->is_device_calibrated(selected_device_id);
}
