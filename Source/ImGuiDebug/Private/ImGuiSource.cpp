// Wrapper to compile Dear ImGui source files within UE5's build system.
// imgui source lives in ThirdParty/imgui/ (outside Source/) so UBT won't
// auto-compile them. We #include them here as a single translation unit.

#include "CoreMinimal.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244) // type conversion, possible loss of data
#pragma warning(disable: 4267) // size_t to int conversion
#pragma warning(disable: 4127) // conditional expression is constant
#endif

#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
