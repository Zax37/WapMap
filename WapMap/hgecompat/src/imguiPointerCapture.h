#pragma once
#include <imgui_internal.h>
#include "../../imguiTheme.h"

namespace WapMapImGui {
// Legacy Think precedes NewFrame so synchronous dialogs can re-enter the loop.
// WantCaptureMouse still describes the previous pointer position at that point.
inline bool CapturePointerAt(ImVec2 position, bool legacyDragging, bool nativeDragging) {
    auto& context = *ImGui::GetCurrentContext();
    if (context.OpenPopupStack.Size > 0 || nativeDragging) return true;
    if (legacyDragging) return false;
    for (int i = context.Windows.Size - 1; i >= 0; --i) {
        const auto* window = context.Windows[i];
        // Between frames, Active describes the just-rendered window. WasActive
        // is only advanced by the next NewFrame and misses newly opened panels.
        if (!window->Active || window->Hidden || (window->Flags & ImGuiWindowFlags_NoMouseInputs)) continue;
        // The pass-through dockspace host must not block legacy welcome-screen input.
        const auto dockspaceHostFlags =
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;
        if ((window->Flags & dockspaceHostFlags) == dockspaceHostFlags) continue;
        // ImGui also creates an internal dock-node host. Honor its central
        // pass-through hole, just as ImGui's own hovered-window lookup does.
        if (window->HitTestHoleSize.x != 0) {
            const ImVec2 holeMin{window->Pos.x + window->HitTestHoleOffset.x,
                                 window->Pos.y + window->HitTestHoleOffset.y};
            const ImVec2 holeMax{holeMin.x + window->HitTestHoleSize.x,
                                 holeMin.y + window->HitTestHoleSize.y};
            if (ImRect(holeMin, holeMax).Contains(position)) continue;
        }
        auto bounds = window->OuterRectClipped;
        bounds.Expand(context.Style.TouchExtraPadding);
        if (!(window->Flags & ImGuiWindowFlags_NoResize)) bounds.Expand(context.Style.WindowBorderHoverPadding);
        if (bounds.Contains(position)) return true;
    }
    return false;
}

inline void ClampMovingWindowToMonitorWorkArea() {
    auto& context = *ImGui::GetCurrentContext();
    if (!context.MovingWindow) return;

    auto* window = context.MovingWindow->RootWindowDockTree;
    if (window->DockNode || !window->ViewportOwned) return;

    const auto position = window->Pos;
    const auto clampedPosition = ClampWindowPositionToMonitorWorkArea(position, window->Size);
    if (clampedPosition.x == position.x && clampedPosition.y == position.y) return;

    ImGui::SetWindowPos(window, clampedPosition, ImGuiCond_Always);
    auto* viewport = static_cast<ImGuiViewportP*>(window->Viewport);
    viewport->Pos = clampedPosition;
    viewport->UpdateWorkRect();
}
}
