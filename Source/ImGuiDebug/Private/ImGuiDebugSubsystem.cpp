#include "ImGuiDebugSubsystem.h"
#include "SImGuiWidget.h"
#include "imgui.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SWeakWidget.h"

bool UImGuiDebugSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if !UE_BUILD_SHIPPING
	return Super::ShouldCreateSubsystem(Outer);
#else
	return false;
#endif
}

void UImGuiDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ImGuiCtx = ImGui::CreateContext();
	ImGui::SetCurrentContext(ImGuiCtx);

	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Dark theme with slight transparency
	ImGui::StyleColorsDark();
	ImGuiStyle& Style = ImGui::GetStyle();
	Style.WindowRounding = 4.0f;
	Style.FrameRounding = 2.0f;
	Style.Alpha = 0.92f;
	Style.Colors[ImGuiCol_WindowBg].w = 0.92f;

	CreateFontAtlasTexture();

	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UImGuiDebugSubsystem::Tick));
}

void UImGuiDebugSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

	if (bOverlayAdded && ImGuiWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ImGuiWidget.ToSharedRef());
	}
	ImGuiWidget.Reset();
	bOverlayAdded = false;

	FontAtlasResourceHandle = FSlateResourceHandle();

	if (FontAtlasBrush.IsValid())
	{
		FontAtlasBrush->ReleaseResource();
		FontAtlasBrush.Reset();
	}

	if (ImGuiCtx)
	{
		ImGui::DestroyContext(ImGuiCtx);
		ImGuiCtx = nullptr;
	}

	Super::Deinitialize();
}

bool UImGuiDebugSubsystem::Tick(float DeltaTime)
{
	if (!ImGuiCtx) return true;

	ImGui::SetCurrentContext(ImGuiCtx);
	EnsureViewportOverlay();

	ImGuiIO& IO = ImGui::GetIO();
	IO.DeltaTime = FMath::Max(DeltaTime, SMALL_NUMBER);

	// Use the widget's actual local size for DisplaySize so ImGui coordinates
	// match the Slate widget's local coordinate space. This is critical because
	// the widget's local size may differ from GetViewportSize() in the editor
	// (PIE viewport overlay can be larger than the game viewport).
	if (ImGuiWidget.IsValid() && ImGuiWidget->CachedLocalSize.X > 0 && ImGuiWidget->CachedLocalSize.Y > 0)
	{
		IO.DisplaySize = ImVec2(ImGuiWidget->CachedLocalSize.X, ImGuiWidget->CachedLocalSize.Y);
	}
	else if (GEngine && GEngine->GameViewport)
	{
		// Fallback for the first frame before the widget has ticked
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const float DPIScale = GEngine->GameViewport->GetDPIScale();
		IO.DisplaySize = ImVec2(ViewportSize.X / DPIScale, ViewportSize.Y / DPIScale);
	}

	ImGui::NewFrame();
	OnImGuiDraw.Broadcast();
	ImGui::Render();

	return true;
}

void UImGuiDebugSubsystem::CreateFontAtlasTexture()
{
	ImGuiIO& IO = ImGui::GetIO();

	// Load a system font with Vietnamese glyph support
	static const ImWchar GlyphRanges[] = {
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0100, 0x024F, // Latin Extended-A + Latin Extended-B (Ơ, Ư, etc.)
		0x0300, 0x036F, // Combining Diacritical Marks
		0x1EA0, 0x1EF9, // Latin Extended Additional (Vietnamese)
		0x2000, 0x206F, // General Punctuation
		0,
	};

	const char* FontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
	ImFont* Font = IO.Fonts->AddFontFromFileTTF(FontPath, 16.0f, nullptr, GlyphRanges);
	if (!Font)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ImGui] Failed to load %hs, falling back to default font"), FontPath);
		IO.Fonts->AddFontDefault();
	}

	unsigned char* Pixels;
	int Width, Height;
	IO.Fonts->GetTexDataAsRGBA32(&Pixels, &Width, &Height);

	UE_LOG(LogTemp, Warning, TEXT("[ImGui] Font atlas: %dx%d"), Width, Height);

	// Convert RGBA (ImGui) to BGRA (Slate native format)
	const int32 NumPixels = Width * Height;
	TArray<uint8> AtlasData;
	AtlasData.SetNum(NumPixels * 4);
	for (int32 i = 0; i < NumPixels; ++i)
	{
		AtlasData[i * 4 + 0] = Pixels[i * 4 + 2]; // B
		AtlasData[i * 4 + 1] = Pixels[i * 4 + 1]; // G
		AtlasData[i * 4 + 2] = Pixels[i * 4 + 0]; // R
		AtlasData[i * 4 + 3] = Pixels[i * 4 + 3]; // A
	}

	// Create texture directly in Slate's rendering system — bypasses UTexture2D entirely
	static int32 AtlasCounter = 0;
	FName AtlasName = *FString::Printf(TEXT("ImGuiFontAtlas_%d"), AtlasCounter++);

	FontAtlasBrush = FSlateDynamicImageBrush::CreateWithImageData(
		AtlasName,
		FVector2D(Width, Height),
		AtlasData);

	UE_LOG(LogTemp, Warning, TEXT("[ImGui] Dynamic brush created: %s"),
		FontAtlasBrush.IsValid() ? TEXT("YES") : TEXT("NO"));

	// Cache the resource handle for rendering
	if (FontAtlasBrush.IsValid() && FSlateApplication::IsInitialized())
	{
		FontAtlasResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*FontAtlasBrush);
	}

	UE_LOG(LogTemp, Warning, TEXT("[ImGui] Resource handle valid: %s"),
		FontAtlasResourceHandle.IsValid() ? TEXT("YES") : TEXT("NO"));

	// Store resource handle pointer as ImGui texture ID
	IO.Fonts->SetTexID(reinterpret_cast<ImTextureID>(&FontAtlasResourceHandle));
}

TSharedPtr<SWidget> UImGuiDebugSubsystem::GetWidget() const
{
	return ImGuiWidget;
}

void UImGuiDebugSubsystem::SetInputEnabled(bool bEnabled)
{
	if (ImGuiWidget.IsValid())
	{
		ImGuiWidget->SetInputEnabled(bEnabled);
	}
}

void UImGuiDebugSubsystem::EnsureViewportOverlay()
{
	if (bOverlayAdded || !GEngine || !GEngine->GameViewport) return;

	SAssignNew(ImGuiWidget, SImGuiWidget).Context(ImGuiCtx);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(ImGuiWidget),
		100
	);

	bOverlayAdded = true;
}
