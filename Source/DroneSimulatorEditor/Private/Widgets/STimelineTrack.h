#pragma once

#include "CoreMinimal.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"
#include "Runtime/SlateCore/Public/Widgets/SLeafWidget.h"

/**
 * Custom timeline track widget that draws time markers
 */
class STimelineTrack : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(STimelineTrack)
		: _start_time(0.0f)
		, _end_time(10.0f)
		, _is_zoomed(false)
		{}
		SLATE_ARGUMENT(float, start_time)
		SLATE_ARGUMENT(float, end_time)
		SLATE_ATTRIBUTE(bool, is_zoomed)
		SLATE_ATTRIBUTE(TArray<float>, event_times)
		SLATE_EVENT(FSimpleDelegate, OnPanStart)
		SLATE_EVENT(FOnFloatValueChanged, OnPan)
		SLATE_EVENT(FSimpleDelegate, OnPanEnd)
		SLATE_EVENT(FOnFloatValueChanged, OnZoom)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void set_time_range(float in_start_time, float in_end_time);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:
	float start_time;
	float end_time;

	/** Calculate appropriate interval for time markers based on visible duration */
	float calculate_marker_interval(float visible_duration) const;

	/** Pan state */
	bool is_panning = false;
	FVector2D last_mouse_position;

	/** Event delegates */
	FSimpleDelegate on_pan_start;
	FOnFloatValueChanged on_pan;
	FSimpleDelegate on_pan_end;
	FOnFloatValueChanged on_zoom;
	TAttribute<bool> is_zoomed;
	TAttribute<TArray<float>> event_times;
};

