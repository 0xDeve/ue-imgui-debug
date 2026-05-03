# ImGuiDebug — Unreal Engine Plugin

An Unreal Engine plugin that renders a [Dear ImGui](https://github.com/ocornut/imgui) overlay directly into the game viewport using Slate. Designed for in-game debug tooling in editor and PIE sessions.

## Features

- **GameInstance subsystem** — zero boilerplate setup; the overlay activates automatically in PIE
- **Slate-native rendering** — draws via a custom `SLeafWidget`, no RHI or UTexture2D required
- **Delegate-driven draw calls** — bind to `OnImGuiDraw` to push any ImGui windows each frame
- **Input forwarding** — mouse, keyboard, and mouse-wheel events are routed to ImGui; call `SetInputEnabled` to toggle focus
- **Vietnamese / extended Latin font** — loads Segoe UI with a custom glyph range covering Vietnamese combining diacritics; falls back to the built-in ImGui default font
- **Editor-only by default** — `ShouldCreateSubsystem` returns `false` outside `WITH_EDITOR`, so the overlay is stripped from packaged builds

## Requirements

- Unreal Engine 5.x
- Windows (font path resolves to `C:\Windows\Fonts\segoeui.ttf`; override in `CreateFontAtlasTexture` for other platforms)

## Installation

1. Copy the `ImGuiDebug` folder into your project's `Plugins/` directory.
2. Regenerate project files.
3. Enable the plugin in your `.uproject`:

```json
{
  "Plugins": [
    { "Name": "ImGuiDebug", "Enabled": true }
  ]
}
```

4. Add `"ImGuiDebug"` to your module's `PublicDependencyModuleNames` or `PrivateDependencyModuleNames` in your `.Build.cs`.

## Usage

Obtain the subsystem and bind to the draw delegate:

```cpp
#include "ImGuiDebugSubsystem.h"
#include "imgui.h"

// Inside BeginPlay or Initialize:
if (UImGuiDebugSubsystem* ImGuiSub = GetGameInstance()->GetSubsystem<UImGuiDebugSubsystem>())
{
    ImGuiSub->OnImGuiDraw.AddUObject(this, &UMyClass::DrawImGui);
}

// Your draw function:
void UMyClass::DrawImGui()
{
    ImGui::Begin("My Debug Window");
    ImGui::Text("Hello from ImGuiDebug!");
    ImGui::End();
}
```

To capture keyboard/mouse input (e.g. when a debug window is open):

```cpp
ImGuiSub->SetInputEnabled(true);   // ImGui receives input
ImGuiSub->SetInputEnabled(false);  // input passes through to the game
```

## Project Structure

```
ImGuiDebug/
├── ImGuiDebug.uplugin
├── Source/ImGuiDebug/
│   ├── ImGuiDebug.Build.cs
│   ├── Public/
│   │   ├── ImGuiDebugModule.h
│   │   └── ImGuiDebugSubsystem.h   ← main API surface
│   └── Private/
│       ├── ImGuiDebugModule.cpp
│       ├── ImGuiDebugSubsystem.cpp
│       ├── SImGuiWidget.h/.cpp     ← Slate rendering + input widget
│       └── ImGuiSource.cpp         ← unity build of Dear ImGui sources
└── ThirdParty/imgui/               ← Dear ImGui (vendored)
```

## Third-party

Dear ImGui is vendored under `ThirdParty/imgui/`. See its [LICENSE](ThirdParty/imgui/LICENSE.txt) for terms (MIT).

## License

MIT — see [LICENSE](LICENSE).
