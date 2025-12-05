#include "DroneSimulatorEditor/Private/Widgets/STimelineTrack.h"

#include "Runtime/SlateCore/Public/Rendering/DrawElements.h"
#include "Runtime/SlateCore/Public/Fonts/FontMeasure.h"
#include "Runtime/SlateCore/Public/Styling/AppStyle.h"

void STimelineTrack::Construct(const FArguments& InArgs)
{
	start_time = InArgs._start_time;
	end_time = InArgs._end_time;
	on_pan_start = InArgs._OnPanStart;
	on_pan = InArgs._OnPan;
	on_pan_end = InArgs._OnPanEnd;
	on_zoom = InArgs._OnZoom;
	is_zoomed = InArgs._is_zoomed;
	event_times = InArgs._event_times;
}

void STimelineTrack::set_time_range(float in_start_time, float in_end_time)
{
	start_time = in_start_time;
	end_time = in_end_time;
}

FVector2D STimelineTrack::ComputeDesiredSize(float) const
{
	return FVector2D(100.0f, 30.0f);
}

float STimelineTrack::calculate_marker_interval(float visible_duration) const
{
	// Choose intervals that give roughly 5-10 markers
	const float target_marker_count = 8.0f;
	float raw_interval = visible_duration / target_marker_count;
	
	// Round to nice numbers (0.1, 0.2, 0.5, 1, 2, 5, 10, etc.)
	float magnitude = FMath::Pow(10.0f, FMath::Floor(FMath::LogX(10.0f, raw_interval)));
	float normalized = raw_interval / magnitude;
	
	float nice_interval;
	if (normalized <= 1.0f)
		nice_interval = 1.0f;
	else if (normalized <= 2.0f)
		nice_interval = 2.0f;
	else if (normalized <= 5.0f)
		nice_interval = 5.0f;
	else
		nice_interval = 10.0f;
	
	return nice_interval * magnitude;
}

int32 STimelineTrack::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D size = AllottedGeometry.GetLocalSize();
	const float width = size.X;
	const float height = size.Y;
	
	if (width <= 0.0f || height <= 0.0f)
	{
		return LayerId;
	}
	
	const bool is_enabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect draw_effects = is_enabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	
	float visible_duration = end_time - start_time;
	if (visible_duration <= 0.0f)
	{
		return LayerId;
	}
	
	// Account for slider thumb padding (standard SSlider has ~8px padding on each side)
	const float slider_thumb_padding = 8.0f;
	const float usable_width = width - (slider_thumb_padding * 2.0f);
	
	// Calculate interval for markers
	float interval = calculate_marker_interval(visible_duration);
	
	// Calculate first marker time (aligned to interval)
	float first_marker = FMath::CeilToFloat(start_time / interval) * interval;
	
	// Font for time labels
	FSlateFontInfo font = FAppStyle::GetFontStyle("SmallFont");
	const TSharedRef<FSlateFontMeasure> font_measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	
	// Draw markers
	for (float marker_time = first_marker; marker_time <= end_time; marker_time += interval)
	{
		// Calculate X position with padding
		float normalized_pos = (marker_time - start_time) / visible_duration;
		float x_pos = slider_thumb_padding + (normalized_pos * usable_width);
		
		if (x_pos < 0.0f || x_pos > width)
			continue;
		
		// Draw vertical tick line
		TArray<FVector2D> tick_points;
		tick_points.Add(FVector2D(x_pos, height - 10.0f));
		tick_points.Add(FVector2D(x_pos, height));
		
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			tick_points,
			draw_effects,
			FLinearColor(0.5f, 0.5f, 0.5f, 0.8f),
			true,
			1.0f
		);
		
		// Draw time label
		FString time_label = FString::Printf(TEXT("%.2f"), marker_time);
		FVector2D text_size = font_measure->Measure(time_label, font);
		
		FVector2D text_pos(x_pos - text_size.X * 0.5f, 2.0f);
		
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(text_pos, text_size),
			time_label,
			font,
			draw_effects,
			FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)
		);
	}
	
	// Draw event tick marks
	TArray<float> events = event_times.Get(TArray<float>());
	for (float event_time : events)
	{
		// Check if event is within visible range
		if (event_time < start_time || event_time > end_time)
			continue;
		
		// Calculate X position with padding
		float normalized_pos = (event_time - start_time) / visible_duration;
		float x_pos = slider_thumb_padding + (normalized_pos * usable_width);
		
		if (x_pos < 0.0f || x_pos > width)
			continue;
		
		// Draw vertical tick line for event (taller and different color)
		TArray<FVector2D> event_tick_points;
		event_tick_points.Add(FVector2D(x_pos, 0.0f));
		event_tick_points.Add(FVector2D(x_pos, height));
		
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(),
			event_tick_points,
			draw_effects,
			FLinearColor(0.0f, 0.7f, 1.0f, 0.6f), // Blue color for events
			true,
			2.0f // Thicker line
		);
	}
	
	return LayerId + 3;
}

FReply STimelineTrack::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Start panning with left mouse button or middle mouse button when zoomed
	bool can_pan = is_zoomed.Get(false);
	
	if (can_pan && (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || 
		MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton))
	{
		is_panning = true;
		last_mouse_position = MouseEvent.GetScreenSpacePosition();
		on_pan_start.ExecuteIfBound();
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	
	return FReply::Unhandled();
}

FReply STimelineTrack::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (is_panning)
	{
		is_panning = false;
		on_pan_end.ExecuteIfBound();
		return FReply::Handled().ReleaseMouseCapture();
	}
	
	return FReply::Unhandled();
}

FReply STimelineTrack::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (is_panning)
	{
		FVector2D current_mouse = MouseEvent.GetScreenSpacePosition();
		FVector2D delta = current_mouse - last_mouse_position;
		
		// Convert pixel delta to time delta
		// Account for slider thumb padding
		const float slider_thumb_padding = 8.0f;
		float visible_duration = end_time - start_time;
		float timeline_width = MyGeometry.GetLocalSize().X;
		float usable_width = timeline_width - (slider_thumb_padding * 2.0f);
		
		if (usable_width > 0.0f)
		{
			float time_delta = -(delta.X / usable_width) * visible_duration;
			on_pan.ExecuteIfBound(time_delta);
		}
		
		last_mouse_position = current_mouse;
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FReply STimelineTrack::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	float wheel_delta = MouseEvent.GetWheelDelta();
	
	if (FMath::Abs(wheel_delta) > 0.0f)
	{
		on_zoom.ExecuteIfBound(wheel_delta);
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FCursorReply STimelineTrack::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	if (is_panning)
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHandClosed);
	}
	else if (is_zoomed.Get(false))
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}
	
	return FCursorReply::Cursor(EMouseCursor::Default);
}

