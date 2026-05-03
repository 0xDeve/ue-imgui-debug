#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ImGuiDebugSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnImGuiDraw);

struct ImGuiContext;
struct FSlateBrush;
struct FSlateDynamicImageBrush;
class SImGuiWidget;

UCLASS()
class IMGUIDEBUG_API UImGuiDebugSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Bind to this delegate to draw ImGui UI each frame. */
	FOnImGuiDraw OnImGuiDraw;

	/** Get the cached font atlas resource handle (used by the rendering widget). */
	const FSlateResourceHandle& GetFontAtlasResourceHandle() const { return FontAtlasResourceHandle; }

	/** Get the ImGui widget (as SWidget) for input focus targeting. */
	TSharedPtr<SWidget> GetWidget() const;

	/** Enable or disable ImGui input processing (mouse, keyboard). */
	void SetInputEnabled(bool bEnabled);

private:
	bool Tick(float DeltaTime);
	void CreateFontAtlasTexture();
	void EnsureViewportOverlay();

	ImGuiContext* ImGuiCtx = nullptr;

	TSharedPtr<FSlateDynamicImageBrush> FontAtlasBrush;
	FSlateResourceHandle FontAtlasResourceHandle;

	TSharedPtr<SImGuiWidget> ImGuiWidget;
	FTSTicker::FDelegateHandle TickDelegateHandle;
	bool bOverlayAdded = false;
};
