#include "DroneAirfoilAssetDetails.h"
#include "DroneSimulatorGame/Assets/DroneAirfoilAsset.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SAirfoilGraphWidget.h"

// Custom widget for table cells that supports hover highlighting per column and row
class STableCell : public SBorder
{
public:
	SLATE_BEGIN_ARGS(STableCell)
		: _content()
		, _column_index(0)
		, _row_index(0)
		, _is_even_row(false)
		, _column_hover_state()
		, _row_hover_state()
		{}
		SLATE_DEFAULT_SLOT(FArguments, content)
		SLATE_ARGUMENT(int32, column_index)
		SLATE_ARGUMENT(int32, row_index)
		SLATE_ARGUMENT(bool, is_even_row)
		SLATE_ARGUMENT(TSharedPtr<int32>, column_hover_state)
		SLATE_ARGUMENT(TSharedPtr<int32>, row_hover_state)
	SLATE_END_ARGS()

	void Construct(const FArguments& in_args)
	{
		column_index = in_args._column_index;
		row_index = in_args._row_index;
		is_even_row = in_args._is_even_row;
		column_hover_state = in_args._column_hover_state;
		row_hover_state = in_args._row_hover_state;

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &STableCell::GetCellBrush)
			.Padding(4)
			[
				in_args._content.Widget
			]
		);
	}

	virtual void OnMouseEnter(const FGeometry& my_geometry, const FPointerEvent& mouse_event) override
	{
		SBorder::OnMouseEnter(my_geometry, mouse_event);
		if (column_hover_state.IsValid())
		{
			*column_hover_state = column_index;
		}
		if (row_hover_state.IsValid())
		{
			*row_hover_state = row_index;
		}
	}

	virtual void OnMouseLeave(const FPointerEvent& mouse_event) override
	{
		SBorder::OnMouseLeave(mouse_event);
		if (column_hover_state.IsValid() && *column_hover_state == column_index)
		{
			*column_hover_state = -1;
		}
		if (row_hover_state.IsValid() && *row_hover_state == row_index)
		{
			*row_hover_state = -1;
		}
	}

private:
	const FSlateBrush* GetCellBrush() const
	{
		bool is_column_hovered = column_hover_state.IsValid() && *column_hover_state == column_index;
		bool is_row_hovered = row_hover_state.IsValid() && *row_hover_state == row_index;

		// Create brushes that combine alternating row colors with hover effect
		if (is_even_row)
		{
			if (is_column_hovered && is_row_hovered)
			{
				// Even row + both column and row hovered (intersection)
				static FSlateBrush EvenRowBothHoverBrush = FSlateBrush();
				EvenRowBothHoverBrush.TintColor = FLinearColor(0.10f, 0.13f, 0.15f, 1.0f);
				EvenRowBothHoverBrush.DrawAs = ESlateBrushDrawType::Box;
				return &EvenRowBothHoverBrush;
			}
			else if (is_column_hovered || is_row_hovered)
			{
				// Even row + column or row hovered
				static FSlateBrush EvenRowHoverBrush = FSlateBrush();
				EvenRowHoverBrush.TintColor = FLinearColor(0.07f, 0.09f, 0.10f, 1.0f);
				EvenRowHoverBrush.DrawAs = ESlateBrushDrawType::Box;
				return &EvenRowHoverBrush;
			}
			else
			{
				// Even row normal
				static FSlateBrush EvenRowBrush = FSlateBrush();
				EvenRowBrush.TintColor = FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);
				EvenRowBrush.DrawAs = ESlateBrushDrawType::Box;
				return &EvenRowBrush;
			}
		}
		else
		{
			if (is_column_hovered && is_row_hovered)
			{
				// Odd row + both column and row hovered (intersection)
				static FSlateBrush OddRowBothHoverBrush = FSlateBrush();
				OddRowBothHoverBrush.TintColor = FLinearColor(0.08f, 0.10f, 0.12f, 1.0f);
				OddRowBothHoverBrush.DrawAs = ESlateBrushDrawType::Box;
				return &OddRowBothHoverBrush;
			}
			else if (is_column_hovered || is_row_hovered)
			{
				// Odd row + column or row hovered
				static FSlateBrush OddRowHoverBrush = FSlateBrush();
				OddRowHoverBrush.TintColor = FLinearColor(0.04f, 0.06f, 0.07f, 1.0f);
				OddRowHoverBrush.DrawAs = ESlateBrushDrawType::Box;
				return &OddRowHoverBrush;
			}
			else
			{
				// Odd row normal
				static FSlateBrush OddRowBrush = FSlateBrush();
				OddRowBrush.TintColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
				OddRowBrush.DrawAs = ESlateBrushDrawType::Box;
				return &OddRowBrush;
			}
		}
	}

	int32 column_index;
	int32 row_index;
	bool is_even_row;
	TSharedPtr<int32> column_hover_state;
	TSharedPtr<int32> row_hover_state;
};

// Custom widget for header cells that supports hover highlighting per column
class STableHeaderCell : public SBorder
{
public:
	SLATE_BEGIN_ARGS(STableHeaderCell)
		: _content()
		, _column_index(0)
		, _column_hover_state()
		{}
		SLATE_DEFAULT_SLOT(FArguments, content)
		SLATE_ARGUMENT(int32, column_index)
		SLATE_ARGUMENT(TSharedPtr<int32>, column_hover_state)
	SLATE_END_ARGS()

	void Construct(const FArguments& in_args)
	{
		column_index = in_args._column_index;
		column_hover_state = in_args._column_hover_state;

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &STableHeaderCell::GetCellBrush)
			.Padding(0) // No padding on the wrapper, let the inner border handle it
			[
				in_args._content.Widget
			]
		);
	}

	virtual void OnMouseEnter(const FGeometry& my_geometry, const FPointerEvent& mouse_event) override
	{
		SBorder::OnMouseEnter(my_geometry, mouse_event);
		if (column_hover_state.IsValid())
		{
			*column_hover_state = column_index;
		}
	}

	virtual void OnMouseLeave(const FPointerEvent& mouse_event) override
	{
		SBorder::OnMouseLeave(mouse_event);
		if (column_hover_state.IsValid() && *column_hover_state == column_index)
		{
			*column_hover_state = -1;
		}
	}

private:
	const FSlateBrush* GetCellBrush() const
	{
		bool is_hovered = column_hover_state.IsValid() && *column_hover_state == column_index;
		
		if (is_hovered)
		{
			// Header hovered
			static FSlateBrush HeaderHoverBrush = FSlateBrush();
			HeaderHoverBrush.TintColor = FLinearColor(0.10f, 0.12f, 0.14f, 1.0f);
			HeaderHoverBrush.DrawAs = ESlateBrushDrawType::Box;
			return &HeaderHoverBrush;
		}
		else
		{
			// Header normal (transparent)
			return FAppStyle::GetBrush("NoBorder");
		}
	}

		int32 column_index;
	TSharedPtr<int32> column_hover_state;
};

TSharedRef<IDetailCustomization> FDroneAirfoilAssetDetails::make_instance()
{
	return MakeShareable(new FDroneAirfoilAssetDetails());
}

void FDroneAirfoilAssetDetails::CustomizeDetails(IDetailLayoutBuilder& detail_builder)
{
	// Get the object being customized
	TArray<TWeakObjectPtr<UObject>> Objects;
	detail_builder.GetObjectsBeingCustomized(Objects);
	
	if (Objects.Num() != 1)
	{
		return;
	}

	// Check if it's a table-based or simplified airfoil asset
	airfoil_asset_table = Cast<UDroneAirfoilAssetTable>(Objects[0].Get());
	airfoil_asset_simplified = Cast<UDroneAirfoilAssetSimplified>(Objects[0].Get());

	// Initialize view mode state (default to graph view)
 	show_raw_as_graph = MakeShared<bool>(true);

	// Get the default category
	IDetailCategoryBuilder& ImportedDataCategory = detail_builder.EditCategory("Drone simulator");

	// Customize for table-based airfoil
	if (airfoil_asset_table.IsValid())
	{
		// Hide the default array property
		TSharedRef<IPropertyHandle> ImportedDataProp = detail_builder.GetProperty(GET_MEMBER_NAME_CHECKED(UDroneAirfoilAssetTable, imported_xfoil_data));
		ImportedDataProp->MarkHiddenByCustomization();

		// Add imported name property (keep it visible)
		TSharedRef<IPropertyHandle> ImportedNameProp = detail_builder.GetProperty(GET_MEMBER_NAME_CHECKED(UDroneAirfoilAssetTable, imported_name));
		ImportedDataCategory.AddProperty(ImportedNameProp);

		// Add Imported Data section with toggle (includes Viterna post-stall correction from Python)
		ImportedDataCategory.AddCustomRow(NSLOCTEXT("DroneAirfoilAsset", "ImportedDataSearch", "Imported Data"))
			.WholeRowContent()
			[
				SNew(SExpandableArea)
				.InitiallyCollapsed(false)
				.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
				.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
				.Padding(8.0f)
				.HeaderContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("DroneAirfoilAsset", "ImportedDataTitle", "Imported Airfoil Data (includes post-stall correction to ±90°)"))
						.Font(IDetailLayoutBuilder::GetDetailFontBold())
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(8, 0, 0, 0)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0, 0, 4, 0)
						[
							SNew(SButton)
							.Text(NSLOCTEXT("DroneAirfoilAsset", "GraphButton", "Graph"))
							.ButtonColorAndOpacity_Lambda([this]() -> FSlateColor
							{
								return *show_raw_as_graph ? FLinearColor(0.3f, 0.5f, 0.8f) : FLinearColor(0.2f, 0.2f, 0.2f);
							})
							.OnClicked_Lambda([this]() -> FReply
							{
								*show_raw_as_graph = true;
								return FReply::Handled();
							})
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.Text(NSLOCTEXT("DroneAirfoilAsset", "TableButton", "Table"))
							.ButtonColorAndOpacity_Lambda([this]() -> FSlateColor
							{
								return !(*show_raw_as_graph) ? FLinearColor(0.3f, 0.5f, 0.8f) : FLinearColor(0.2f, 0.2f, 0.2f);
							})
							.OnClicked_Lambda([this]() -> FReply
							{
								*show_raw_as_graph = false;
								return FReply::Handled();
							})
						]
					]
				]
				.BodyContent()
				[
					SNew(SVerticalBox)
					// Graph view
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.Visibility_Lambda([this]() -> EVisibility
						{
							return *show_raw_as_graph ? EVisibility::Visible : EVisibility::Collapsed;
						})
						[
							create_xfoil_data_graph_widget(airfoil_asset_table->imported_xfoil_data)
						]
					]
					// Table view
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.Visibility_Lambda([this]() -> EVisibility
						{
							return !(*show_raw_as_graph) ? EVisibility::Visible : EVisibility::Collapsed;
						})
						[
							create_xfoil_data_table_widget(airfoil_asset_table->imported_xfoil_data, FText::GetEmpty())
						]
					]
				]
			];
	}
	// Customize for simplified airfoil
	else if (airfoil_asset_simplified.IsValid())
	{
		// Add all properties for the TRIVIALLY SIMPLE model with change callbacks
		TSharedRef<IPropertyHandle> ClKRadProp = detail_builder.GetProperty(GET_MEMBER_NAME_CHECKED(UDroneAirfoilAssetSimplified, cl_k_rad));
		TSharedRef<IPropertyHandle> Cd0Prop = detail_builder.GetProperty(GET_MEMBER_NAME_CHECKED(UDroneAirfoilAssetSimplified, cd_0));
		TSharedRef<IPropertyHandle> CdKProp = detail_builder.GetProperty(GET_MEMBER_NAME_CHECKED(UDroneAirfoilAssetSimplified, cd_k));
		
		// Add change delegates to force layout refresh when values change
		ClKRadProp->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&detail_builder]() { detail_builder.ForceRefreshDetails(); }));
		Cd0Prop->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&detail_builder]() { detail_builder.ForceRefreshDetails(); }));
		CdKProp->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&detail_builder]() { detail_builder.ForceRefreshDetails(); }));
		
		ImportedDataCategory.AddProperty(ClKRadProp);
		ImportedDataCategory.AddProperty(Cd0Prop);
		ImportedDataCategory.AddProperty(CdKProp);

		// Add graph visualization for simplified model
		ImportedDataCategory.AddCustomRow(NSLOCTEXT("DroneAirfoilAsset", "SimplifiedVisualizationSearch", "Model Visualization"))
			.WholeRowContent()
			[
				SNew(SExpandableArea)
				.InitiallyCollapsed(false)
				.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
				.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
				.Padding(8.0f)
				.HeaderContent()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("DroneAirfoilAsset", "SimplifiedGraphTitle", "Aerodynamic Model Graphs"))
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				]
				.BodyContent()
				[
					create_simplified_airfoil_graph_widget(airfoil_asset_simplified.Get())
				]
			];
	}
}

TSharedRef<SWidget> FDroneAirfoilAssetDetails::create_xfoil_data_table_widget(const FRawXfoilData& xfoil_data, const FText& table_title)
{
	// Early return if no data
	if (xfoil_data.reynolds_data.Num() == 0)
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				[
					SNew(STextBlock)
					.Text(table_title)
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("DroneAirfoilAsset", "NoData", "No data available"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			];
	}

	// Build a list of all unique angles of attack (rows)
	TSet<float> unique_angles;
	for (const FReynoldsXfoilData& reynolds_data : xfoil_data.reynolds_data)
	{
		for (const FAngleOfAttackXfoilData& aoa_data : reynolds_data.angle_of_attack_data)
		{
			unique_angles.Add(aoa_data.angle_of_attack);
		}
	}

	TArray<float> sorted_angles = unique_angles.Array();
	sorted_angles.Sort();

	// Sort Reynolds numbers for column headers
	TArray<float> reynolds_numbers;
	for (const FReynoldsXfoilData& reynolds_data : xfoil_data.reynolds_data)
	{
		reynolds_numbers.Add(reynolds_data.reynolds_number);
	}
	reynolds_numbers.Sort();

	// Create shared hover state (shared pointers that track which column and row are hovered)
	TSharedPtr<int32> column_hover_state = MakeShared<int32>(-1);
	TSharedPtr<int32> row_hover_state = MakeShared<int32>(-1);
	
	// Create shared state for show all toggle
	TSharedPtr<bool> show_all_rows = MakeShared<bool>(false);
	
	// Container that will hold the table content (needs to be refreshable)
	TSharedPtr<SVerticalBox> table_content_container;

	// Shared scroll boxes
	TSharedPtr<SScrollBox> header_scroll_box;
	TSharedPtr<SScrollBox> data_scroll_box;
	TSharedPtr<SScrollBox> fixed_aoa_data_scroll_box;
	
	// Lambda to build/rebuild the table based on show_all_rows state
	auto build_table_content = [&xfoil_data, sorted_angles, reynolds_numbers, column_hover_state, row_hover_state, show_all_rows, &header_scroll_box, &data_scroll_box, &fixed_aoa_data_scroll_box]()
	{
		// Limit the number of rows for performance (display every Nth row if too many)
		const int32 MAX_DISPLAY_ROWS = 50;
		int32 row_skip = *show_all_rows ? 1 : FMath::Max(1, sorted_angles.Num() / MAX_DISPLAY_ROWS);
		
		TArray<float> display_angles;
		for (int32 i = 0; i < sorted_angles.Num(); i += row_skip)
		{
			display_angles.Add(sorted_angles[i]);
		}

		struct FTableParts
		{
			TSharedRef<SWidget> fixed_aoa_header;
			TSharedRef<SHorizontalBox> scrollable_header;
			TSharedRef<SVerticalBox> fixed_aoa_data;
			TSharedRef<SVerticalBox> scrollable_data;
		};

		// Build the fixed AoA header
		TSharedRef<SWidget> fixed_aoa_header = SNew(SBox)
			.MinDesiredWidth(70)
			[
				SNew(STableHeaderCell)
				.column_index(0)
				.column_hover_state(column_hover_state)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
					.Padding(4)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("DroneAirfoilAsset", "AoAHeader", "AoA (°)"))
						.Font(IDetailLayoutBuilder::GetDetailFontBold())
					]
				]
			];

		// Build the scrollable header row (Reynolds columns only)
		TSharedRef<SHorizontalBox> scrollable_header_row = SNew(SHorizontalBox);

		// Add a column for each Reynolds number with CL and CD sub-columns
		int32 column_index = 1; // Start at 1 (0 is the AoA column)
		for (float re : reynolds_numbers)
		{
			FText reynolds_text = FText::Format(NSLOCTEXT("DroneAirfoilAsset", "ReynoldsHeader", "Re={0}"),
				FText::AsNumber(FMath::RoundToInt(re)));

			// Create a container for CL and CD columns with hover support
			TSharedRef<SVerticalBox> reynolds_column_box = SNew(SVerticalBox);

			// Reynolds number header (wraps both the main header and sub-headers)
			TSharedRef<STableHeaderCell> reynolds_header_cell = SNew(STableHeaderCell)
				.column_index(column_index)
				.column_hover_state(column_hover_state);

			TSharedRef<SVerticalBox> header_content = SNew(SVerticalBox);

			// Main Reynolds number
			header_content->AddSlot()
				.AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
					.Padding(4)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(reynolds_text)
						.Font(IDetailLayoutBuilder::GetDetailFontBold())
					]
				];

			// Sub-header with CL and CD
			TSharedRef<SHorizontalBox> sub_header_row = SNew(SHorizontalBox);
			
			sub_header_row->AddSlot()
				.FillWidth(1.0f)
				.Padding(0)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryMiddle"))
					.Padding(2)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("DroneAirfoilAsset", "CLHeader", "CL"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				];

			sub_header_row->AddSlot()
				.FillWidth(1.0f)
				.Padding(0)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryMiddle"))
					.Padding(2)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("DroneAirfoilAsset", "CDHeader", "CD"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				];

			header_content->AddSlot()
				.AutoHeight()
				[
					SNew(SBox)
					.MinDesiredWidth(120)
					[
						sub_header_row
					]
				];

			reynolds_header_cell->SetContent(header_content);

			reynolds_column_box->AddSlot()
				.AutoHeight()
				[
					reynolds_header_cell
				];

			scrollable_header_row->AddSlot()
				.AutoWidth()
				.Padding(FMargin(1, 1, 1, 1))
				[
					reynolds_column_box
				];
			
			column_index++;
		}

		// Build the fixed AoA column data and scrollable data rows
		TSharedRef<SVerticalBox> fixed_aoa_data_content = SNew(SVerticalBox);
		TSharedRef<SVerticalBox> scrollable_data_content = SNew(SVerticalBox);

		// Data rows with alternating colors and hover effects
		int32 row_index = 0;
		for (float angle : display_angles)
		{
			bool is_even_row = (row_index % 2) == 0;

			// Fixed AoA column for this row
			fixed_aoa_data_content->AddSlot()
				.AutoHeight()
				[
					SNew(SBox)
					.MinDesiredWidth(70)
					[
						SNew(STableCell)
						.column_index(0)
						.row_index(row_index)
						.is_even_row(is_even_row)
						.column_hover_state(column_hover_state)
						.row_hover_state(row_hover_state)
						[
							SNew(STextBlock)
							.Text(FText::AsNumber(angle, &FNumberFormattingOptions::DefaultNoGrouping()))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
				];

			// Scrollable data row for this angle
			TSharedRef<SHorizontalBox> scrollable_data_row = SNew(SHorizontalBox);

			// Add data for each Reynolds number
			column_index = 1;
			for (float re : reynolds_numbers)
			{
				// Find the data for this Reynolds number and angle
				float cl = 0.0f;
				float cd = 0.0f;
				bool found_data = false;

				for (const FReynoldsXfoilData& reynolds_data : xfoil_data.reynolds_data)
				{
					if (FMath::IsNearlyEqual(reynolds_data.reynolds_number, re, 0.1f))
					{
						// Find the closest angle of attack
						float min_dist = FLT_MAX;
						for (const FAngleOfAttackXfoilData& aoa_data : reynolds_data.angle_of_attack_data)
						{
							float dist = FMath::Abs(aoa_data.angle_of_attack - angle);
							if (dist < min_dist)
							{
								min_dist = dist;
								cl = aoa_data.lift_coefficient;
								cd = aoa_data.drag_coefficient;
							 	found_data = true;
							}
						}
						break;
					}
				}

				TSharedRef<SHorizontalBox> value_box = SNew(SHorizontalBox);

				// CL value
				value_box->AddSlot()
					.FillWidth(1.0f)
					.Padding(0)
					[
						SNew(STableCell)
						.column_index(column_index)
						.row_index(row_index)
						.is_even_row(is_even_row)
						.column_hover_state(column_hover_state)
						.row_hover_state(row_hover_state)
						[
							SNew(STextBlock)
							.Text(found_data ? FText::AsNumber(cl, &FNumberFormattingOptions::DefaultNoGrouping()) : FText::FromString(TEXT("-")))
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.ColorAndOpacity(found_data ? FSlateColor::UseForeground() : FSlateColor::UseSubduedForeground())
						]
					];

				// CD value
				value_box->AddSlot()
					.FillWidth(1.0f)
					.Padding(0)
					[
						SNew(STableCell)
						.column_index(column_index)
						.row_index(row_index)
						.is_even_row(is_even_row)
						.column_hover_state(column_hover_state)
						.row_hover_state(row_hover_state)
						[
							SNew(STextBlock)
							.Text(found_data ? FText::AsNumber(cd, &FNumberFormattingOptions::DefaultNoGrouping()) : FText::FromString(TEXT("-")))
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.ColorAndOpacity(found_data ? FSlateColor::UseForeground() : FSlateColor::UseSubduedForeground())
						]
					];

				scrollable_data_row->AddSlot()
					.AutoWidth()
					.Padding(FMargin(1, 0, 1, 0))
					[
						SNew(SBox)
						.MinDesiredWidth(120)
						[
							value_box
						]
					];
				
				column_index++;
			}

			scrollable_data_content->AddSlot()
				.AutoHeight()
				[
					scrollable_data_row
				];
			
			row_index++;
		}

		// Info text if rows were skipped (spans both columns)
		if (row_skip > 1)
		{
			scrollable_data_content->AddSlot()
				.AutoHeight()
				.Padding(4)
				[
					SNew(STextBlock)
					.Text(FText::Format(NSLOCTEXT("DroneAirfoilAsset", "RowsSkipped", 
						"Showing every {0} rows ({1} of {2} total) for performance."),
						FText::AsNumber(row_skip),
						FText::AsNumber(display_angles.Num()),
						FText::AsNumber(sorted_angles.Num())))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				];
		}

		return FTableParts{ fixed_aoa_header, scrollable_header_row, fixed_aoa_data_content, scrollable_data_content };
	};

	// Initial table build
	auto initial_table_parts = build_table_content();
	
	// Containers for header and data
	TSharedPtr<SBox> fixed_aoa_header_container;
	TSharedPtr<SHorizontalBox> scrollable_header_container;
	TSharedPtr<SVerticalBox> fixed_aoa_data_container;
	
	// Create the table widget with fixed AoA column and header
	TSharedRef<SWidget> table_widget = SNew(SVerticalBox)
		// Header row (fixed AoA + scrollable Reynolds)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			// Fixed AoA header
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(1, 1, 1, 1))
			[
				SAssignNew(fixed_aoa_header_container, SBox)
			]
			// Scrollable Reynolds headers
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(header_scroll_box, SScrollBox)
				.Orientation(Orient_Horizontal)
				.ScrollBarVisibility(EVisibility::Collapsed) // Hide this scrollbar, use the data one
				+ SScrollBox::Slot()
				[
					SAssignNew(scrollable_header_container, SHorizontalBox)
				]
			]
		]
		// Data area (fixed AoA column + scrollable data)
		+ SVerticalBox::Slot()
		.MaxHeight(600.0f) // Fixed height for scrolling
		[
			SNew(SHorizontalBox)
			// Fixed AoA data column
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(1, 0, 1, 0))
			[
				SAssignNew(fixed_aoa_data_scroll_box, SScrollBox)
				.Orientation(Orient_Vertical)
				.ScrollBarVisibility(EVisibility::Collapsed) // Hide scrollbar, sync with data scrollbar
				+ SScrollBox::Slot()
				[
					SAssignNew(fixed_aoa_data_container, SVerticalBox)
				]
			]
			// Scrollable data columns
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SAssignNew(data_scroll_box, SScrollBox)
				.Orientation(Orient_Horizontal)
				.OnUserScrolled_Lambda([header_scroll_box](float scroll_offset)
				{
					// Sync header horizontal scroll with data horizontal scroll
					if (header_scroll_box.IsValid())
					{
						header_scroll_box->SetScrollOffset(scroll_offset);
					}
				})
				+ SScrollBox::Slot()
				[
					SNew(SScrollBox)
					.Orientation(Orient_Vertical)
					.OnUserScrolled_Lambda([fixed_aoa_data_scroll_box](float scroll_offset)
					{
						// Sync fixed AoA column vertical scroll with data vertical scroll
						if (fixed_aoa_data_scroll_box.IsValid())
						{
							fixed_aoa_data_scroll_box->SetScrollOffset(scroll_offset);
						}
					})
					+ SScrollBox::Slot()
					[
						SAssignNew(table_content_container, SVerticalBox)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		.HAlign(HAlign_Center)
		[
			SNew(SButton)
			.Text_Lambda([show_all_rows, sorted_angles]() -> FText
			{
				if (*show_all_rows)
				{
					return NSLOCTEXT("DroneAirfoilAsset", "ShowLess", "Show Less");
				}
				else
				{
					const int32 MAX_DISPLAY_ROWS = 50;
					int32 row_skip = FMath::Max(1, sorted_angles.Num() / MAX_DISPLAY_ROWS);
					if (row_skip > 1)
					{
						return FText::Format(NSLOCTEXT("DroneAirfoilAsset", "ShowAll", "Show All {0} Rows"),
							FText::AsNumber(sorted_angles.Num()));
					}
					else
					{
						return FText::GetEmpty();
					}
				}
			})
			.Visibility_Lambda([show_all_rows, sorted_angles]() -> EVisibility
			{
				const int32 MAX_DISPLAY_ROWS = 50;
				int32 row_skip = FMath::Max(1, sorted_angles.Num() / MAX_DISPLAY_ROWS);
				// Only show button if we have enough rows to warrant it, or if already showing all
				return (*show_all_rows || row_skip > 1) ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.OnClicked_Lambda([show_all_rows, fixed_aoa_header_container, scrollable_header_container, fixed_aoa_data_container, table_content_container, build_table_content]() -> FReply
			{
				*show_all_rows = !(*show_all_rows);
				
				// Rebuild the table with new state
				auto new_table_parts = build_table_content();
				
				// Update fixed AoA header
				fixed_aoa_header_container->SetContent(new_table_parts.fixed_aoa_header);
				
				// Update scrollable header
				scrollable_header_container->ClearChildren();
				for (int32 i = 0; i < new_table_parts.scrollable_header->NumSlots(); ++i)
				{
					const SHorizontalBox::FSlot& slot = new_table_parts.scrollable_header->GetSlot(i);
					scrollable_header_container->AddSlot()
						.AutoWidth()
						.Padding(slot.GetPadding())
						[
							slot.GetWidget()
						];
				}
				
				// Update fixed AoA data
				fixed_aoa_data_container->ClearChildren();
				for (int32 i = 0; i < new_table_parts.fixed_aoa_data->NumSlots(); ++i)
				{
					fixed_aoa_data_container->AddSlot()
						.AutoHeight()
						[
							new_table_parts.fixed_aoa_data->GetSlot(i).GetWidget()
						];
				}
				
				// Update scrollable data content
				table_content_container->ClearChildren();
				for (int32 i = 0; i < new_table_parts.scrollable_data->NumSlots(); ++i)
				{
					table_content_container->AddSlot()
						.AutoHeight()
						[
							new_table_parts.scrollable_data->GetSlot(i).GetWidget()
						];
				}
				
				return FReply::Handled();
			})
		];

	// Populate initial fixed AoA header
	fixed_aoa_header_container->SetContent(initial_table_parts.fixed_aoa_header);

	// Populate initial scrollable header
	for (int32 i = 0; i < initial_table_parts.scrollable_header->NumSlots(); ++i)
	{
		const SHorizontalBox::FSlot& slot = initial_table_parts.scrollable_header->GetSlot(i);
		scrollable_header_container->AddSlot()
			.AutoWidth()
			.Padding(slot.GetPadding())
			[
				slot.GetWidget()
			];
	}

	// Populate initial fixed AoA data
	for (int32 i = 0; i < initial_table_parts.fixed_aoa_data->NumSlots(); ++i)
	{
		fixed_aoa_data_container->AddSlot()
			.AutoHeight()
			[
				initial_table_parts.fixed_aoa_data->GetSlot(i).GetWidget()
			];
	}

	// Populate initial scrollable data content
	for (int32 i = 0; i < initial_table_parts.scrollable_data->NumSlots(); ++i)
	{
		table_content_container->AddSlot()
			.AutoHeight()
			[
				initial_table_parts.scrollable_data->GetSlot(i).GetWidget()
			];
	}

	// Wrap in a border with optional title
	TSharedRef<SVerticalBox> content_box = SNew(SVerticalBox);
	
	// Only add title if it's not empty
	if (!table_title.IsEmpty())
	{
		content_box->AddSlot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(table_title)
				.Font(IDetailLayoutBuilder::GetDetailFontBold())
			];
	}
	
	content_box->AddSlot()
		.FillHeight(1.0f)
		[
			table_widget
		];

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			content_box
		];
}

TSharedRef<SWidget> FDroneAirfoilAssetDetails::create_xfoil_data_graph_widget(const FRawXfoilData& xfoil_data)
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 8)
		[
			SNew(SAirfoilGraphWidget)
			.graph_title(NSLOCTEXT("DroneAirfoilAsset", "LiftGraphTitle", "Lift Coefficient (CL) vs Angle of Attack"))
			.y_axis_label(NSLOCTEXT("DroneAirfoilAsset", "LiftYAxis", "CL"))
			.show_lift(true)
			.show_drag(false)
			.graph_height(300.0f)
			.xfoil_data(&xfoil_data)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SAirfoilGraphWidget)
			.graph_title(NSLOCTEXT("DroneAirfoilAsset", "DragGraphTitle", "Drag Coefficient (CD) vs Angle of Attack"))
			.y_axis_label(NSLOCTEXT("DroneAirfoilAsset", "DragYAxis", "CD"))
			.show_lift(false)
			.show_drag(true)
			.graph_height(300.0f)
			.xfoil_data(&xfoil_data)
		];
}

TSharedRef<SWidget> FDroneAirfoilAssetDetails::create_simplified_airfoil_graph_widget(const UDroneAirfoilAssetSimplified* simplified_asset)
{
	// Create a single graph with both lift and drag curves
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SAirfoilGraphWidget)
			.graph_title(NSLOCTEXT("DroneAirfoilAsset", "SimplifiedGraphTitle", "Aerodynamic Coefficients vs Angle of Attack"))
			.y_axis_label(NSLOCTEXT("DroneAirfoilAsset", "CoefficientsYAxis", "Coefficient"))
			.show_lift(true)
			.show_drag(true)
			.graph_height(400.0f)
			.simplified_asset(simplified_asset)
		];
}

