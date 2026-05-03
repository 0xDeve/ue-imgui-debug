# ImGuiDebug Plugin - Architecture

> Auto-maintained via `/update-architecture`

## Purpose

In-game ImGui debug overlay for runtime visualization and debugging.

## Module: ImGuiDebug (Runtime)

### Dependencies

**Public**: Core, CoreUObject, Engine, InputCore, RenderCore, Slate, SlateCore
**Third-party**: Dear ImGui (`ThirdParty/imgui/`)

### File Structure

```
Plugins/ImGuiDebug/
├── ImGuiDebug.uplugin
├── Source/ImGuiDebug/
│   ├── ImGuiDebug.Build.cs
│   ├── Public/
│   │   ├── ImGuiDebugModule.h          # Module interface (IModuleInterface)
│   │   └── ImGuiDebugSubsystem.h       # Main subsystem
│   ├── Private/
│   │   ├── ImGuiDebugModule.cpp
│   │   └── ImGuiDebugSubsystem.cpp
│   └── ThirdParty/imgui/               # Dear ImGui headers
│       ├── imgui.h
│       ├── imgui.cpp
│       ├── imgui_draw.cpp
│       ├── imgui_tables.cpp
│       ├── imgui_widgets.cpp
│       └── imgui_demo.cpp
└── ARCHITECTURE.md
```

### Class: UImGuiDebugSubsystem

**Type**: UGameInstanceSubsystem
**Lifecycle**: Auto-created per game instance, survives level transitions

**Responsibilities**:
- Initialize/teardown ImGui context and font atlas
- Provide `OnImGuiDraw` multicast delegate for custom UI rendering
- Manage viewport overlay widget
- Handle input forwarding to ImGui

**Key API**:
- `OnImGuiDraw` - Delegate: bind to render custom ImGui panels
- `SetInputEnabled(bool)` - Toggle ImGui input capture
- `Initialize()` / `Deinitialize()` - Lifecycle (automatic)

### Integration

Used by `AVRPlayerPawn::DrawImGuiDebugPanel()` to render conversation logs, NPC state, and debug info in-game.
