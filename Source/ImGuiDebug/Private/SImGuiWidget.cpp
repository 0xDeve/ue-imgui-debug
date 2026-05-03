#include "SImGuiWidget.h"
#include "imgui.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"

void SImGuiWidget::Construct(const FArguments& InArgs)
{
	Context = InArgs._Context;
	// Start with input disabled — only rendering, no hit testing
	SetVisibility(EVisibility::SelfHitTestInvisible);
}

void SImGuiWidget::SetInputEnabled(bool bEnabled)
{
	SetVisibility(bEnabled ? EVisibility::Visible : EVisibility::SelfHitTestInvisible);
}

void SImGuiWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Cache the widget's local size so the subsystem can set ImGui DisplaySize to match
	CachedLocalSize = AllottedGeometry.GetLocalSize();

	// Force repaint every frame — ImGui content changes each tick
	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D SImGuiWidget::ComputeDesiredSize(float) const
{
	return FVector2D(3840, 2160);
}

// ── Rendering ──────────────────────────────────────────────────────────────────

int32 SImGuiWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	if (!Context) return LayerId;

	ImGui::SetCurrentContext(Context);
	const ImDrawData* DrawData = ImGui::GetDrawData();
	if (!DrawData || DrawData->CmdListsCount == 0) return LayerId;

	// Transform from ImGui local space → screen (window) space
	const FSlateRenderTransform ImGuiToScreen = AllottedGeometry.GetAccumulatedRenderTransform();
	const FVector2f DisplayPos(DrawData->DisplayPos.x, DrawData->DisplayPos.y);

	for (int32 ListIdx = 0; ListIdx < DrawData->CmdListsCount; ++ListIdx)
	{
		const ImDrawList* CmdList = DrawData->CmdLists[ListIdx];

		// ── Build ALL Slate vertices for this draw list at once ──
		const int32 NumVerts = CmdList->VtxBuffer.Size;
		TArray<FSlateVertex> Vertices;
		Vertices.SetNumUninitialized(NumVerts);

		for (int32 i = 0; i < NumVerts; ++i)
		{
			const ImDrawVert& Src = CmdList->VtxBuffer[i];
			FSlateVertex& Dst = Vertices[i];

			const FVector2f LocalPos(Src.pos.x - DisplayPos.X, Src.pos.y - DisplayPos.Y);
			const FVector2f ScreenPos = ImGuiToScreen.TransformPoint(LocalPos);

			Dst.TexCoords[0] = Src.uv.x;
			Dst.TexCoords[1] = Src.uv.y;
			Dst.TexCoords[2] = 1.f;
			Dst.TexCoords[3] = 1.f;
			Dst.MaterialTexCoords = FVector2f(Src.uv.x, Src.uv.y);
			Dst.Position = ScreenPos;
			Dst.Color = FColor(
				(Src.col >> IM_COL32_R_SHIFT) & 0xFF,
				(Src.col >> IM_COL32_G_SHIFT) & 0xFF,
				(Src.col >> IM_COL32_B_SHIFT) & 0xFF,
				(Src.col >> IM_COL32_A_SHIFT) & 0xFF);
			Dst.SecondaryColor = FColor(0, 0, 0, 0);
			Dst.PixelSize[0] = 1;
			Dst.PixelSize[1] = 1;
		}

		for (int32 CmdIdx = 0; CmdIdx < CmdList->CmdBuffer.Size; ++CmdIdx)
		{
			const ImDrawCmd& DrawCmd = CmdList->CmdBuffer[CmdIdx];

			if (DrawCmd.UserCallback)
			{
				DrawCmd.UserCallback(CmdList, &DrawCmd);
				continue;
			}

			if (DrawCmd.ElemCount == 0) continue;

			const FVector2f ClipMin(DrawCmd.ClipRect.x - DisplayPos.X, DrawCmd.ClipRect.y - DisplayPos.Y);
			const FVector2f ClipMax(DrawCmd.ClipRect.z - DisplayPos.X, DrawCmd.ClipRect.w - DisplayPos.Y);
			const FVector2f ScreenClipMin = ImGuiToScreen.TransformPoint(ClipMin);
			const FVector2f ScreenClipMax = ImGuiToScreen.TransformPoint(ClipMax);
			const FSlateRect ClipRect(ScreenClipMin.X, ScreenClipMin.Y, ScreenClipMax.X, ScreenClipMax.Y);
			OutDrawElements.PushClip(FSlateClippingZone{ ClipRect.IntersectionWith(MyCullingRect) });

			TArray<SlateIndex> Indices;
			Indices.SetNumUninitialized(DrawCmd.ElemCount);
			for (uint32 i = 0; i < DrawCmd.ElemCount; ++i)
			{
				Indices[i] = static_cast<SlateIndex>(
					CmdList->IdxBuffer[DrawCmd.IdxOffset + i] + DrawCmd.VtxOffset);
			}

			const FSlateResourceHandle* ResourceHandle =
				reinterpret_cast<const FSlateResourceHandle*>(DrawCmd.GetTexID());

			if (ResourceHandle && ResourceHandle->IsValid())
			{
				FSlateDrawElement::MakeCustomVerts(
					OutDrawElements,
					LayerId,
					*ResourceHandle,
					Vertices,
					Indices,
					nullptr, 0, 0);
			}

			OutDrawElements.PopClip();
		}
	}

	return LayerId;
}

// ── Input ──────────────────────────────────────────────────────────────────────

int32 SImGuiWidget::ToImGuiMouseButton(const FKey& Button)
{
	if (Button == EKeys::LeftMouseButton)   return 0;
	if (Button == EKeys::RightMouseButton)  return 1;
	if (Button == EKeys::MiddleMouseButton) return 2;
	return -1;
}

ImGuiKey SImGuiWidget::ToImGuiKey(const FKey& Key)
{
	// Navigation
	if (Key == EKeys::Tab) return ImGuiKey_Tab;
	if (Key == EKeys::Left) return ImGuiKey_LeftArrow;
	if (Key == EKeys::Right) return ImGuiKey_RightArrow;
	if (Key == EKeys::Up) return ImGuiKey_UpArrow;
	if (Key == EKeys::Down) return ImGuiKey_DownArrow;
	if (Key == EKeys::PageUp) return ImGuiKey_PageUp;
	if (Key == EKeys::PageDown) return ImGuiKey_PageDown;
	if (Key == EKeys::Home) return ImGuiKey_Home;
	if (Key == EKeys::End) return ImGuiKey_End;
	if (Key == EKeys::Insert) return ImGuiKey_Insert;
	if (Key == EKeys::Delete) return ImGuiKey_Delete;
	if (Key == EKeys::BackSpace) return ImGuiKey_Backspace;
	if (Key == EKeys::SpaceBar) return ImGuiKey_Space;
	if (Key == EKeys::Enter) return ImGuiKey_Enter;
	if (Key == EKeys::Escape) return ImGuiKey_Escape;

	// Modifiers
	if (Key == EKeys::LeftControl) return ImGuiKey_LeftCtrl;
	if (Key == EKeys::RightControl) return ImGuiKey_RightCtrl;
	if (Key == EKeys::LeftShift) return ImGuiKey_LeftShift;
	if (Key == EKeys::RightShift) return ImGuiKey_RightShift;
	if (Key == EKeys::LeftAlt) return ImGuiKey_LeftAlt;
	if (Key == EKeys::RightAlt) return ImGuiKey_RightAlt;

	// Letters (needed for Ctrl+A/C/V/X/Z shortcuts)
	if (Key == EKeys::A) return ImGuiKey_A;
	if (Key == EKeys::B) return ImGuiKey_B;
	if (Key == EKeys::C) return ImGuiKey_C;
	if (Key == EKeys::D) return ImGuiKey_D;
	if (Key == EKeys::E) return ImGuiKey_E;
	if (Key == EKeys::F) return ImGuiKey_F;
	if (Key == EKeys::G) return ImGuiKey_G;
	if (Key == EKeys::H) return ImGuiKey_H;
	if (Key == EKeys::I) return ImGuiKey_I;
	if (Key == EKeys::J) return ImGuiKey_J;
	if (Key == EKeys::K) return ImGuiKey_K;
	if (Key == EKeys::L) return ImGuiKey_L;
	if (Key == EKeys::M) return ImGuiKey_M;
	if (Key == EKeys::N) return ImGuiKey_N;
	if (Key == EKeys::O) return ImGuiKey_O;
	if (Key == EKeys::P) return ImGuiKey_P;
	if (Key == EKeys::Q) return ImGuiKey_Q;
	if (Key == EKeys::R) return ImGuiKey_R;
	if (Key == EKeys::S) return ImGuiKey_S;
	if (Key == EKeys::T) return ImGuiKey_T;
	if (Key == EKeys::U) return ImGuiKey_U;
	if (Key == EKeys::V) return ImGuiKey_V;
	if (Key == EKeys::W) return ImGuiKey_W;
	if (Key == EKeys::X) return ImGuiKey_X;
	if (Key == EKeys::Y) return ImGuiKey_Y;
	if (Key == EKeys::Z) return ImGuiKey_Z;

	return ImGuiKey_None;
}

FReply SImGuiWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!Context) return FReply::Unhandled();
	ImGui::SetCurrentContext(Context);

	const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	ImGui::GetIO().AddMousePosEvent(LocalPos.X, LocalPos.Y);

	// Always handle when we have mouse captured (dragging), otherwise let ImGui decide
	if (HasMouseCapture() || ImGui::GetIO().WantCaptureMouse)
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SImGuiWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!Context) return FReply::Unhandled();
	ImGui::SetCurrentContext(Context);

	// Update mouse position so ImGui knows where the click is
	const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	ImGui::GetIO().AddMousePosEvent(LocalPos.X, LocalPos.Y);

	const int32 Btn = ToImGuiMouseButton(MouseEvent.GetEffectingButton());
	if (Btn >= 0)
	{
		ImGui::GetIO().AddMouseButtonEvent(Btn, true);
	}

	if (ImGui::GetIO().WantCaptureMouse)
	{
		// Capture mouse so drags/scrolls stay with this widget, and take keyboard focus
		return FReply::Handled()
			.CaptureMouse(SharedThis(this))
			.SetUserFocus(SharedThis(this), EFocusCause::Mouse);
	}

	return FReply::Unhandled();
}

FReply SImGuiWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!Context) return FReply::Unhandled();
	ImGui::SetCurrentContext(Context);

	const int32 Btn = ToImGuiMouseButton(MouseEvent.GetEffectingButton());
	if (Btn >= 0)
	{
		ImGui::GetIO().AddMouseButtonEvent(Btn, false);
	}

	// Always release mouse capture on button up
	if (HasMouseCapture())
	{
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Handled();
}

FReply SImGuiWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!Context) return FReply::Unhandled();
	ImGui::SetCurrentContext(Context);

	ImGui::GetIO().AddMouseWheelEvent(0.0f, MouseEvent.GetWheelDelta());

	return ImGui::GetIO().WantCaptureMouse ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (!Context) return FReply::Unhandled();
	ImGui::SetCurrentContext(Context);
	ImGuiIO& IO = ImGui::GetIO();

	// Forward modifier state
	IO.AddKeyEvent(ImGuiMod_Ctrl, InKeyEvent.IsControlDown());
	IO.AddKeyEvent(ImGuiMod_Shift, InKeyEvent.IsShiftDown());
	IO.AddKeyEvent(ImGuiMod_Alt, InKeyEvent.IsAltDown());

	// Forward the actual key
	const ImGuiKey MappedKey = ToImGuiKey(InKeyEvent.GetKey());
	if (MappedKey != ImGuiKey_None)
	{
		IO.AddKeyEvent(MappedKey, true);
	}

	return IO.WantCaptureKeyboard ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiWidget::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (!Context) return FReply::Unhandled();
	ImGui::SetCurrentContext(Context);
	ImGuiIO& IO = ImGui::GetIO();

	IO.AddKeyEvent(ImGuiMod_Ctrl, InKeyEvent.IsControlDown());
	IO.AddKeyEvent(ImGuiMod_Shift, InKeyEvent.IsShiftDown());
	IO.AddKeyEvent(ImGuiMod_Alt, InKeyEvent.IsAltDown());

	const ImGuiKey MappedKey = ToImGuiKey(InKeyEvent.GetKey());
	if (MappedKey != ImGuiKey_None)
	{
		IO.AddKeyEvent(MappedKey, false);
	}

	return IO.WantCaptureKeyboard ? FReply::Handled() : FReply::Unhandled();
}

FReply SImGuiWidget::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
	if (!Context) return FReply::Unhandled();
	ImGui::SetCurrentContext(Context);

	const TCHAR Char = InCharacterEvent.GetCharacter();
	if (Char > 0 && Char < 0x10000)
	{
		ImGui::GetIO().AddInputCharacter(static_cast<unsigned int>(Char));
	}

	return ImGui::GetIO().WantCaptureKeyboard ? FReply::Handled() : FReply::Unhandled();
}
