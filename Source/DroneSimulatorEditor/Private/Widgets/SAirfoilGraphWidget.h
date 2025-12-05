#pragma once

#include "CoreMinimal.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"
#include "Runtime/SlateCore/Public/Widgets/SCompoundWidget.h"
#include "Runtime/SlateCore/Public/Widgets/SLeafWidget.h"
#include "Runtime/SlateCore/Public/Rendering/DrawElements.h"

struct FRawXfoilData;
class UDroneAirfoilAssetSimplified;

/**
 * Widget that displays airfoil coefficient curves as 2D line graphs
 * Shows multiple Reynolds number curves on the same plot
 */
class SAirfoilGraphWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAirfoilGraphWidget)
		: _graph_title()
		, _y_axis_label()
		, _show_lift(true)
		, _show_drag(false)
		, _graph_height(300.0f)
		, _xfoil_data(nullptr)
		, _simplified_asset(nullptr)
		{}
		SLATE_ARGUMENT(FText, graph_title)
		SLATE_ARGUMENT(FText, y_axis_label)
		SLATE_ARGUMENT(bool, show_lift)
		SLATE_ARGUMENT(bool, show_drag)
		SLATE_ARGUMENT(float, graph_height)
		SLATE_ARGUMENT(const FRawXfoilData*, xfoil_data)
		SLATE_ARGUMENT(const UDroneAirfoilAssetSimplified*, simplified_asset)
	SLATE_END_ARGS()

	/** Constructs this widget */
	void Construct(const FArguments& InArgs);

private:
	/** Data for one curve */
	struct FCurveData
	{
		FString label;
		TArray<FVector2D> points; // X = angle in degrees, Y = coefficient
		FLinearColor color;
		bool is_visible = true; // Whether the curve is currently visible
	};

	/** Custom widget for drawing the graph canvas (axes, grid, curves) */
	class SGraphCanvas : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SGraphCanvas) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);
		
		void set_graph_data(const FText& in_graph_title, const FText& in_y_axis_label,
			const TArray<FCurveData>* in_curves, float in_min_x, float in_max_x, float in_min_y, float in_max_y);

		void set_show_data_points(bool show) { show_data_points = show; }
		bool get_show_data_points() const { return show_data_points; }

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
			bool bParentEnabled) const override;

		virtual FVector2D ComputeDesiredSize(float) const override;

	private:
		FText graph_title;
		FText y_axis_label;
		const TArray<FCurveData>* curves;
		float min_x, max_x, min_y, max_y;
		bool show_data_points = true;

		FVector2D transform_point_to_screen(const FVector2D& data_point, const FGeometry& geometry) const;
		void draw_grid(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const;
		void draw_axes(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const;
		void draw_curves(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const;
		void draw_data_points(const FGeometry& allotted_geometry, FSlateWindowElementList& out_draw_elements, int32 layer_id) const;
	};

	/** Graph configuration */
	FText graph_title;
	FText y_axis_label;
	float graph_height;

	/** Curve data to display */
	TArray<FCurveData> curves;

	/** Axis ranges (computed from data) */
	float min_x, max_x, min_y, max_y;
	
	/** Reference to the graph canvas widget */
	TSharedPtr<SGraphCanvas> graph_canvas;

	/** Helper functions */
	void compute_axis_ranges();
	FReply on_legend_item_clicked(int32 curve_index);
	FReply on_select_all_clicked();
	FReply on_select_none_clicked();
	FReply on_toggle_data_points_clicked();
	TSharedRef<class SWidget> create_legend_widget();
	
	/** Generate colors for Reynolds number curves */
	static FLinearColor get_reynolds_color(int32 index, int32 total_count);
	
	/** Generate simplified model curves */
	void generate_simplified_curves(const UDroneAirfoilAssetSimplified* simplified_asset);
};
