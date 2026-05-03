#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

struct ImGuiContext;
enum ImGuiKey : int;

class SImGuiWidget : public SLeafWidget
{
	SLATE_BEGIN_ARGS(SImGuiWidget)
		: _Context(nullptr)
	{}
		SLATE_ARGUMENT(ImGuiContext*, Context)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Enable/disable input processing. When disabled, the widget is hit-test invisible. */
	void SetInputEnabled(bool bEnabled);

	// SWidget overrides
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual bool SupportsKeyboardFocus() const override { return true; }

	// Mouse input
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	// Keyboard input
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override;

	/** Cached widget local size from last Tick, used by subsystem to set ImGui DisplaySize. */
	FVector2D CachedLocalSize = FVector2D::ZeroVector;

private:
	static int32 ToImGuiMouseButton(const FKey& Button);
	static ImGuiKey ToImGuiKey(const FKey& Key);

	ImGuiContext* Context = nullptr;
};
