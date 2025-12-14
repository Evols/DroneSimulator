#include "SAirfoilGraphWidget.h"
#include "DroneSimulatorGame/Assets/DroneAirfoilAsset.h"
#include "Rendering/DrawElements.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SAirfoilGraphWidget"

void SAirfoilGraphWidget::Construct(const FArguments& InArgs)
{
	graph_title = InArgs._graph_title;
	y_axis_label = InArgs._y_axis_label;
	const bool show_lift = InArgs._show_lift;
	const bool show_drag = InArgs._show_drag;
	graph_height = InArgs._graph_height;

	curves.Empty();

	// Check if we're building from table data or simplified data
	const FRawXfoilData* in_xfoil_data = InArgs._xfoil_data;
	const UDroneAirfoilAssetSimplified* in_simplified_asset = InArgs._simplified_asset;

	if (in_simplified_asset)
	{
		// Generate simplified model curves
		generate_simplified_curves(in_simplified_asset);
	}
	else if (in_xfoil_data && in_xfoil_data->reynolds_data.Num() > 0)
	{
		// Build curves from table data
		const int32 num_reynolds = in_xfoil_data->reynolds_data.Num();

		for (int32 re_idx = 0; re_idx < num_reynolds; ++re_idx)
		{
			const FReynoldsXfoilData& reynolds_data = in_xfoil_data->reynolds_data[re_idx];
			
			if (reynolds_data.angle_of_attack_data.Num() == 0)
			{
				continue;
			}

			// Create lift curve for this Reynolds number
			if (show_lift)
			{
				FCurveData lift_curve;
				lift_curve.label = FString::Printf(TEXT("CL Re=%.0f"), reynolds_data.reynolds_number);
				lift_curve.color = get_reynolds_color(re_idx, num_reynolds);

				for (const FAngleOfAttackXfoilData& aoa_data : reynolds_data.angle_of_attack_data)
				{
					lift_curve.points.Add(FVector2D(aoa_data.angle_of_attack, aoa_data.lift_coefficient));
				}

				curves.Add(lift_curve);
			}

			// Create drag curve for this Reynolds number
			if (show_drag)
			{
				FCurveData drag_curve;
				drag_curve.label = FString::Printf(TEXT("CD Re=%.0f"), reynolds_data.reynolds_number);
				// Use a different hue for drag curves if both are shown
				FLinearColor base_color = get_reynolds_color(re_idx, num_reynolds);
				if (show_lift && show_drag)
				{
					// Shift hue for drag curves to distinguish from lift
					drag_curve.color = FLinearColor(base_color.B, base_color.R, base_color.G, base_color.A);
				}
				else
				{
					drag_curve.color = base_color;
				}

				for (const FAngleOfAttackXfoilData& aoa_data : reynolds_data.angle_of_attack_data)
				{
					drag_curve.points.Add(FVector2D(aoa_data.angle_of_attack, aoa_data.drag_coefficient));
				}

				curves.Add(drag_curve);
			}
		}
	}

	compute_axis_ranges();

	// Build the Slate widget hierarchy
	graph_canvas = SNew(SGraphCanvas);
	graph_canvas->set_graph_data(graph_title, y_axis_label, &curves, min_x, max_x, min_y, max_y);

	ChildSlot
	[
		SNew(SHorizontalBox)
		
		// Graph canvas
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			graph_canvas.ToSharedRef()
		]
		
		// Legend
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(8.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SBox)
			.WidthOverride(200.0f)
			.HeightOverride(graph_height)
			[
				create_legend_widget()
			]
		]
	];
}

void SAirfoilGraphWidget::generate_simplified_curves(const UDroneAirfoilAssetSimplified* simplified_asset)
{
	// Generate curves based on the TRIVIALLY SIMPLE model:
	// Cl = cl_k_rad * angle_of_attack (linear, no stall)
	// Cd = cd_0 + cd_k * Cl^2 (profile drag + induced drag)
	
	if (true) // Always show lift for simplified
	{
		FCurveData lift_curve;
		lift_curve.label = TEXT("CL (Simplified Linear Model)");
		lift_curve.color = FLinearColor(0.3f, 0.7f, 1.0f); // Blue

		// Generate points from -30 to +30 degrees (typical operating range)
		const int32 num_points = 121;
		for (int32 point_idx = 0; point_idx < num_points; ++point_idx)
		{
			double angle_deg = -30.0 + point_idx * 0.5;
			double angle_rad = FMath::DegreesToRadians(angle_deg);
			
			// TRIVIAL: Cl = cl_k_rad * angle_of_attack
			double cl_value = simplified_asset->cl_k_rad * angle_rad;
			
			lift_curve.points.Add(FVector2D(angle_deg, cl_value));
		}

		curves.Add(lift_curve);
	}

	if (true) // Always show drag for simplified
	{
		FCurveData drag_curve;
		drag_curve.label = TEXT("CD (Simplified Model)");
		drag_curve.color = FLinearColor(1.0f, 0.5f, 0.2f); // Orange

		const int32 num_points = 121;
		for (int32 point_idx = 0; point_idx < num_points; ++point_idx)
		{
			double angle_deg = -30.0 + point_idx * 0.5;
			double angle_rad = FMath::DegreesToRadians(angle_deg);
			
			// TRIVIAL: Cl = cl_k_rad * angle_of_attack
			double cl_value = simplified_asset->cl_k_rad * angle_rad;
			
			// TRIVIAL: Cd = cd_0 + cd_k * Cl^2
			double cd_value = simplified_asset->cd_0 + simplified_asset->cd_k * cl_value * cl_value;
			
			drag_curve.points.Add(FVector2D(angle_deg, cd_value));
		}

		curves.Add(drag_curve);
	}
}

void SAirfoilGraphWidget::compute_axis_ranges()
{
	if (curves.Num() == 0)
	{
		min_x = -180.0f;
		max_x = 180.0f;
		min_y = -2.0f;
		max_y = 2.0f;
		return;
	}

	min_x = FLT_MAX;
	max_x = -FLT_MAX;
	min_y = FLT_MAX;
	max_y = -FLT_MAX;

	// Only compute ranges for visible curves
	for (const FCurveData& curve : curves)
	{
		if (!curve.is_visible)
		{
			continue;
		}

		for (const FVector2D& point : curve.points)
		{
			min_x = FMath::Min(min_x, point.X);
			max_x = FMath::Max(max_x, point.X);
			min_y = FMath::Min(min_y, point.Y);
			max_y = FMath::Max(max_y, point.Y);
		}
	}

	// Add padding
	const float x_padding = (max_x - min_x) * 0.05f;
	const float y_padding = (max_y - min_y) * 0.1f;

	min_x -= x_padding;
	max_x += x_padding;
	min_y -= y_padding;
	max_y += y_padding;

	// Ensure reasonable bounds
	if (FMath::Abs(max_y - min_y) < 0.1f)
	{
		min_y -= 0.5f;
		max_y += 0.5f;
	}
}

FReply SAirfoilGraphWidget::on_legend_item_clicked(int32 curve_index)
{
	if (curve_index >= 0 && curve_index < curves.Num())
	{
		curves[curve_index].is_visible = !curves[curve_index].is_visible;
		compute_axis_ranges();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SAirfoilGraphWidget::on_select_all_clicked()
{
	for (FCurveData& curve : curves)
	{
		curve.is_visible = true;
	}
	compute_axis_ranges();
	return FReply::Handled();
}

FReply SAirfoilGraphWidget::on_select_none_clicked()
{
	for (FCurveData& curve : curves)
	{
		curve.is_visible = false;
	}
	compute_axis_ranges();
	return FReply::Handled();
}

FReply SAirfoilGraphWidget::on_toggle_data_points_clicked()
{
	if (graph_canvas.IsValid())
	{
		graph_canvas->set_show_data_points(!graph_canvas->get_show_data_points());
	}
	return FReply::Handled();
}

TSharedRef<SWidget> SAirfoilGraphWidget::create_legend_widget()
{
	// Create the scrollable legend items
	TSharedRef<SVerticalBox> legend_items_box = SNew(SVerticalBox);

	for (int32 i = 0; i < curves.Num(); ++i)
	{
		const FCurveData& curve = curves[i];
		const int32 curve_index = i; // Capture for lambda

		legend_items_box->AddSlot()
		.AutoHeight()
		.Padding(4.0f)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.OnClicked_Lambda([this, curve_index]() -> FReply
			{
				return on_legend_item_clicked(curve_index);
			})
			.Content()
			[
				SNew(SHorizontalBox)
				
				// Color line sample
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(20.0f)
					.HeightOverride(3.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor_Lambda([this, curve_index]() -> FSlateColor
						{
							if (curve_index < curves.Num())
							{
								FLinearColor color = curves[curve_index].color;
								color.A = curves[curve_index].is_visible ? 1.0f : 0.3f;
								return FSlateColor(color);
							}
							return FLinearColor::White;
						})
					]
				]
				
				// Label text
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this, curve_index]() -> FText
					{
						if (curve_index < curves.Num())
						{
							return FText::FromString(curves[curve_index].label);
						}
						return FText::GetEmpty();
					})
					.ColorAndOpacity_Lambda([this, curve_index]() -> FSlateColor
					{
						if (curve_index < curves.Num())
						{
							return curves[curve_index].is_visible ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f);
						}
						return FLinearColor::White;
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				]
			]
		];
	}

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.9f))
		.Padding(4.0f)
		[
			SNew(SVerticalBox)
			
			// Select All / Select None buttons - always visible at top
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 0.0f, 2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AirfoilGraph", "SelectAll", "All"))
					.ToolTipText(NSLOCTEXT("AirfoilGraph", "SelectAllTooltip", "Show all curves"))
					.HAlign(HAlign_Center)
					.OnClicked_Lambda([this]() -> FReply
					{
						return on_select_all_clicked();
					})
				]
				
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(2.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("AirfoilGraph", "SelectNone", "None"))
					.ToolTipText(NSLOCTEXT("AirfoilGraph", "SelectNoneTooltip", "Hide all curves"))
					.HAlign(HAlign_Center)
					.OnClicked_Lambda([this]() -> FReply
					{
						return on_select_none_clicked();
					})
				]
			]
			
			// Toggle data points button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(SButton)
				.Text_Lambda([this]() -> FText
				{
					if (graph_canvas.IsValid() && graph_canvas->get_show_data_points())
					{
						return NSLOCTEXT("AirfoilGraph", "HidePoints", "Hide Data Points");
					}
					return NSLOCTEXT("AirfoilGraph", "ShowPoints", "Show Data Points");
				})
				.ToolTipText(NSLOCTEXT("AirfoilGraph", "TogglePointsTooltip", "Toggle visibility of data point markers"))
				.HAlign(HAlign_Center)
				.OnClicked_Lambda([this]() -> FReply
				{
					return on_toggle_data_points_clicked();
				})
			]
			
			// Scrollable legend items
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				+ SScrollBox::Slot()
				[
					legend_items_box
				]
			]
		];
}

FLinearColor SAirfoilGraphWidget::get_reynolds_color(int32 index, int32 total_count)
{
	if (total_count == 1)
	{
		return FLinearColor(0.3f, 0.7f, 1.0f); // Blue
	}

	// Generate colors using HSV to get distinct hues
	const float hue = (static_cast<float>(index) / total_count) * 300.0f; // 0-300 degrees (avoid red wrapping to magenta)
	const float saturation = 0.8f;
	const float value = 1.0f;

	return FLinearColor::MakeFromHSV8(
		static_cast<uint8>(hue * 255.0f / 360.0f),
		static_cast<uint8>(saturation * 255.0f),
		static_cast<uint8>(value * 255.0f)
	);
}

//////////////////////////////////////////////////////////////////////////
// SGraphCanvas Implementation

void SAirfoilGraphWidget::SGraphCanvas::Construct(const FArguments& InArgs)
{
	curves = nullptr;
	min_x = -180.0f;
	max_x = 180.0f;
	min_y = -2.0f;
	max_y = 2.0f;
}

void SAirfoilGraphWidget::SGraphCanvas::set_graph_data(const FText& in_graph_title, const FText& in_y_axis_label,
	const TArray<FCurveData>* in_curves, float in_min_x, float in_max_x, float in_min_y, float in_max_y)
{
	graph_title = in_graph_title;
	y_axis_label = in_y_axis_label;
	curves = in_curves;
	min_x = in_min_x;
	max_x = in_max_x;
	min_y = in_min_y;
	max_y = in_max_y;
}

int32 SAirfoilGraphWidget::SGraphCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// Draw background
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush("ToolPanel.GroupBorder"),
		ESlateDrawEffect::None,
		FLinearColor(0.02f, 0.02f, 0.02f, 1.0f)
	);

	// Draw grid
	draw_grid(AllottedGeometry, OutDrawElements, LayerId++);

	// Draw axes
	draw_axes(AllottedGeometry, OutDrawElements, LayerId++);

	// Draw curves
	draw_curves(AllottedGeometry, OutDrawElements, LayerId++);

	// Draw data points on top of curves
	draw_data_points(AllottedGeometry, OutDrawElements, LayerId++);

	// Draw title
	if (!graph_title.IsEmpty())
	{
		const TSharedRef<FSlateFontMeasure> font_measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FSlateFontInfo title_font = FCoreStyle::GetDefaultFontStyle("Bold", 12);
		const FVector2D title_size = font_measure->Measure(graph_title, title_font);

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(
				title_size,
				FSlateLayoutTransform(FVector2D((AllottedGeometry.GetLocalSize().X - title_size.X) * 0.5f, 10.0f))
			),
			graph_title,
			title_font,
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}

	return LayerId;
}

FVector2D SAirfoilGraphWidget::SGraphCanvas::ComputeDesiredSize(float) const
{
	return FVector2D(600.0f, 300.0f);
}

FVector2D SAirfoilGraphWidget::SGraphCanvas::transform_point_to_screen(const FVector2D& data_point, const FGeometry& geometry) const
{
	const float margin_left = 60.0f;
	const float margin_right = 20.0f;
	const float margin_top = 40.0f;
	const float margin_bottom = 40.0f;

	const FVector2D size = geometry.GetLocalSize();
	const float plot_width = size.X - margin_left - margin_right;
	const float plot_height = size.Y - margin_top - margin_bottom;

	const float normalized_x = (data_point.X - min_x) / (max_x - min_x);
	const float normalized_y = (data_point.Y - min_y) / (max_y - min_y);

	const float screen_x = margin_left + normalized_x * plot_width;
	const float screen_y = margin_top + (1.0f - normalized_y) * plot_height; // Flip Y

	return FVector2D(screen_x, screen_y);
}

void SAirfoilGraphWidget::SGraphCanvas::draw_grid(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const
{
	const float margin_left = 60.0f;
	const float margin_right = 20.0f;
	const float margin_top = 40.0f;
	const float margin_bottom = 40.0f;

	const FVector2D size = allotted_geometry.GetLocalSize();
	const float plot_width = size.X - margin_left - margin_right;
	const float plot_height = size.Y - margin_top - margin_bottom;

	const FLinearColor grid_color(0.1f, 0.1f, 0.1f, 1.0f);

	// Vertical grid lines (X axis) - must match num_x_labels in draw_axes
	const int32 num_vertical_lines = 9;
	for (int32 line_idx = 0; line_idx <= num_vertical_lines; ++line_idx)
	{
		const float alpha = static_cast<float>(line_idx) / num_vertical_lines;
		const float screen_x = margin_left + alpha * plot_width;

		TArray<FVector2D> line_points;
		line_points.Add(FVector2D(screen_x, margin_top));
		line_points.Add(FVector2D(screen_x, margin_top + plot_height));

		FSlateDrawElement::MakeLines(
			out_draw_elements,
			layer_id,
			allotted_geometry.ToPaintGeometry(),
			line_points,
			ESlateDrawEffect::None,
			grid_color,
			true,
			1.0f
		);
	}

	// Horizontal grid lines (Y axis) - must match num_y_labels in draw_axes
	const int32 num_horizontal_lines = 6;
	for (int32 line_idx = 0; line_idx <= num_horizontal_lines; ++line_idx)
	{
		const float alpha = static_cast<float>(line_idx) / num_horizontal_lines;
		const float screen_y = margin_top + alpha * plot_height;

		TArray<FVector2D> line_points;
		line_points.Add(FVector2D(margin_left, screen_y));
		line_points.Add(FVector2D(margin_left + plot_width, screen_y));

		FSlateDrawElement::MakeLines(
			out_draw_elements,
			layer_id,
			allotted_geometry.ToPaintGeometry(),
			line_points,
			ESlateDrawEffect::None,
			grid_color,
			true,
			1.0f
		);
	}
}

void SAirfoilGraphWidget::SGraphCanvas::draw_axes(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const
{
	const float margin_left = 60.0f;
	const float margin_right = 20.0f;
	const float margin_top = 40.0f;
	const float margin_bottom = 40.0f;

	const FVector2D size = allotted_geometry.GetLocalSize();
	const float plot_width = size.X - margin_left - margin_right;
	const float plot_height = size.Y - margin_top - margin_bottom;

	const FSlateFontInfo axis_font = FCoreStyle::GetDefaultFontStyle("Regular", 9);
	const TSharedRef<FSlateFontMeasure> font_measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	// X-axis labels
	const int32 num_x_labels = 9;
	for (int32 label_idx = 0; label_idx <= num_x_labels; ++label_idx)
	{
		const float alpha = static_cast<float>(label_idx) / num_x_labels;
		const float data_x = min_x + alpha * (max_x - min_x);
		const FVector2D screen_pos = transform_point_to_screen(FVector2D(data_x, min_y), allotted_geometry);

		const FString label_text = FString::Printf(TEXT("%.0f°"), data_x);
		const FVector2D text_size = font_measure->Measure(label_text, axis_font);

		FSlateDrawElement::MakeText(
			out_draw_elements,
			layer_id,
			allotted_geometry.ToPaintGeometry(
				text_size,
				FSlateLayoutTransform(FVector2D(screen_pos.X - text_size.X * 0.5f, size.Y - margin_bottom + 5.0f))
			),
			label_text,
			axis_font,
			ESlateDrawEffect::None,
			FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)
		);
	}

	// X-axis label (Angle of Attack)
	{
		const FText x_axis_label = LOCTEXT("XAxisLabel", "Angle of Attack");
		const FVector2D text_size = font_measure->Measure(x_axis_label, axis_font);
		FSlateDrawElement::MakeText(
			out_draw_elements,
			layer_id,
			allotted_geometry.ToPaintGeometry(
				text_size,
				FSlateLayoutTransform(FVector2D((size.X - text_size.X) * 0.5f, size.Y - 15.0f))
			),
			x_axis_label,
			axis_font,
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}

	// Y-axis labels
	const int32 num_y_labels = 6;
	for (int32 label_idx = 0; label_idx <= num_y_labels; ++label_idx)
	{
		const float alpha = static_cast<float>(label_idx) / num_y_labels;
		const float data_y = min_y + alpha * (max_y - min_y);
		const FVector2D screen_pos = transform_point_to_screen(FVector2D(min_x, data_y), allotted_geometry);

		const FString label_text = FString::Printf(TEXT("%.2f"), data_y);
		const FVector2D text_size = font_measure->Measure(label_text, axis_font);

		FSlateDrawElement::MakeText(
			out_draw_elements,
			layer_id,
			allotted_geometry.ToPaintGeometry(
				text_size,
				FSlateLayoutTransform(FVector2D(margin_left - text_size.X - 5.0f, screen_pos.Y - text_size.Y * 0.5f))
			),
			label_text,
			axis_font,
			ESlateDrawEffect::None,
			FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)
		);
	}

	// Y-axis label
	if (!y_axis_label.IsEmpty())
	{
		const FVector2D text_size = font_measure->Measure(y_axis_label, axis_font);
		
		// Draw rotated text for Y-axis label (we'll just draw it horizontally for simplicity)
		FSlateDrawElement::MakeText(
			out_draw_elements,
			layer_id,
			allotted_geometry.ToPaintGeometry(
				FVector2D(text_size.Y, text_size.X),
				FSlateLayoutTransform(FVector2D(5.0f, (size.Y - text_size.X) * 0.5f))
			),
			y_axis_label,
			axis_font,
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}
}

void SAirfoilGraphWidget::SGraphCanvas::draw_curves(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const
{
	if (!curves)
	{
		return;
	}

	for (const FCurveData& curve : *curves)
	{
		// Skip hidden curves
		if (!curve.is_visible || curve.points.Num() < 2)
		{
			continue;
		}

		TArray<FVector2D> screen_points;
		screen_points.Reserve(curve.points.Num());

		for (const FVector2D& point : curve.points)
		{
			screen_points.Add(transform_point_to_screen(point, allotted_geometry));
		}

		FSlateDrawElement::MakeLines(
			out_draw_elements,
			layer_id,
			allotted_geometry.ToPaintGeometry(),
			screen_points,
			ESlateDrawEffect::None,
			curve.color,
			true,
			2.0f
		);
	}
}

void SAirfoilGraphWidget::SGraphCanvas::draw_data_points(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const
{
	if (!curves || !show_data_points)
	{
		return;
	}

	const float dash_half_height = 4.0f;

	for (const FCurveData& curve : *curves)
	{
		// Skip hidden curves
		if (!curve.is_visible || curve.points.Num() == 0)
		{
			continue;
		}

		// Draw a vertical dash at each data point
		for (const FVector2D& data_point : curve.points)
		{
			const FVector2D screen_pos = transform_point_to_screen(data_point, allotted_geometry);
			
			// Create vertical dash
			TArray<FVector2D> dash_points;
			dash_points.Add(FVector2D(screen_pos.X, screen_pos.Y - dash_half_height));
			dash_points.Add(FVector2D(screen_pos.X, screen_pos.Y + dash_half_height));
			
			// Draw the vertical dash
			FSlateDrawElement::MakeLines(
				out_draw_elements,
				layer_id,
				allotted_geometry.ToPaintGeometry(),
				dash_points,
				ESlateDrawEffect::None,
				curve.color,
				true,
				1.0f
			);
		}
	}
}

#undef LOCTEXT_NAMESPACE
