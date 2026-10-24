#include "hge_internal.h"
#include "../../WapMap/imguiTheme.h"
#include "../../WapMap/legacyText.h"
#include "../../WapMap/hgecompat/src/imguiPointerCapture.h"
#include "../../WapMap/objEdit/nativeTextInput.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

HGE* hge;
HGE::HGE() : impl_(std::make_unique<Impl>()) {}
HGE::~HGE() = default;
bool HGE::System_GetState(hgeBoolState) { return false; }
void* HGE::Resource_Load(const char*, DWORD*) { return nullptr; }
void HGE::Resource_Free(void*) {}
void HGE::System_Log(const char*, ...) {}
void HGE::Release() {}
extern "C" HGE* HGE_CALL hgeCreate(int) { return hge; }

static void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

int main() {
#ifdef _MSC_VER
    _set_error_mode(_OUT_TO_STDERR);
#endif
    try {
        HGE renderer;
        hge = &renderer;
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        WapMapImGui::ApplyTheme();
        auto texture = renderer.Texture_Create(4, 6);
        auto* pixels = renderer.Texture_Lock(texture);
        for (int i = 0; i < 24; ++i) pixels[i] = 0x7FFFFFFF;
        renderer.Texture_Unlock(texture);
        std::string definition = "csv\nh[16] o[0 0 0 0]\n";
        for (int code = 32; code < 256; ++code)
            definition += "#" + std::to_string(code) + " r[0 0 4 6] o[0 0] w[5]\n";
        {
            hgeFont source(definition.data(), static_cast<int>(definition.size()), texture, false, 0, 0);
            auto* font = WapMapImGui::ImportBitmapFont(renderer, source, 1250);
            auto* baked = font->GetFontBaked(16.f);
            const auto* fallback = baked->FindGlyphNoFallback(font->FallbackChar);
            check(fallback && fallback->PackId != ImFontAtlasRectId_Invalid,
                  "Protected fallback glyph must retain its packed pixels");
            check(fallback == &baked->Glyphs[baked->FallbackGlyphIndex],
                  "Fallback lookup must retain the original glyph");
            const auto* glyph = font->GetFontBaked(16.f)->FindGlyph('0');
            check(glyph && glyph->AdvanceX == 5.f && glyph->X1 - glyph->X0 == 4.f &&
                  glyph->Y1 - glyph->Y0 == 6.f, "Preserve original glyph size and advance");
            check(!glyph->Colored, "Glyph must accept disabled and selection text tint");
            unsigned char* rgba;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&rgba, &width, &height);
            const int x = static_cast<int>(glyph->U0 * width + 0.5f);
            const int y = static_cast<int>(glyph->V0 * height + 0.5f);
            check(rgba[(y * width + x) * 4 + 3] == 127, "Preserve original bitmap alpha");
            io.FontDefault = font;
            ImGui::GetStyle().FontSizeBase = font->LegacySize;
            io.Fonts->SetTexID(static_cast<ImTextureID>(1));
            io.DisplaySize = ImVec2(320, 240);
            io.DeltaTime = 1.f / 60.f;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            for (int frame = 0; frame < 3; ++frame) {
                ImGui::NewFrame();
                ImGui::Begin("Font import regression");
                char coordinate[32] = "000";
                ImGui::InputText("X", coordinate, sizeof(coordinate));
                check(ImGui::CalcTextSize("000").x == 15.f, "Native input text uses bitmap advances");
                check(ImGui::CalcTextSize("\xC5\x82").x == 5.f, "Polish UTF-8 uses the mapped legacy bitmap glyph");
                ImGui::SetCursorPos({20, 40});
                auto* markupDraw = ImGui::GetWindowDrawList();
                const int beforeMarkup = markupDraw->VtxBuffer.Size;
                WapMapImGui::WrappedBitmapMarkupText(renderer, source, "~g~AB~w~~n~C", 40);
                check(ImGui::GetItemRectSize().y == 32.f, "Legacy markup newline contributes to wrapped layout height");
                bool hasGreen = false, hasWhite = false;
                for (int v = beforeMarkup; v < markupDraw->VtxBuffer.Size; ++v) {
                    hasGreen |= markupDraw->VtxBuffer[v].col == IM_COL32(76, 214, 143, 255);
                    hasWhite |= markupDraw->VtxBuffer[v].col == IM_COL32(225, 225, 225, 255);
                }
                check(hasGreen && hasWhite, "Legacy text colors survive native dialog rendering");
                struct { hgeSprite* sprButton[4][9]{}; hgeSprite* sprControl[2][6]{}; } skin;
                WapMapImGui::BitmapButton(renderer, source, skin, "Browse", {60, 20});
                const auto browseId = ImGui::GetItemID();
                WapMapImGui::BitmapButton(renderer, source, skin, "Save", {60, 20});
                check(ImGui::GetItemID() != browseId, "Skinned buttons have distinct IDs in one window");
                bool selected = false;
                WapMapImGui::BitmapCheckbox(renderer, source, skin, "First", selected);
                const auto checkboxId = ImGui::GetItemID();
                WapMapImGui::BitmapCheckbox(renderer, source, skin, "Second", selected);
                check(ImGui::GetItemID() != checkboxId, "Skinned checkboxes have distinct IDs");
                int choice = 0;
                WapMapImGui::BitmapRadio(renderer, source, skin, "", choice, 0);
                const auto radioId = ImGui::GetItemID();
                WapMapImGui::BitmapRadio(renderer, source, skin, "", choice, 1);
                check(ImGui::GetItemID() != radioId, "Unlabelled radio choices have distinct IDs");
                ImGui::End();
                ImGui::SetNextWindowSize({160, 228}, ImGuiCond_Always);
                WapMapImGui::BeginFixedWindow("Fixed form regression", nullptr, ImGuiWindowFlags_NoResize);
                ImGui::SetCursorPos({90, 196});
                ImGui::Button("Go", {60, 30});
                if (frame > 0) {
                    check(ImGui::GetScrollMaxY() == 0, "Fixed form has no padding-induced vertical overflow");
                    check(ImGui::GetScrollY() == 0, "Fixed form stays at its original scroll position");
                }
                ImGui::SetCursorPos({5, 40});
                if (ImGui::BeginChild("Scrollable list", {140, 100})) {
                    ImGui::Dummy({100, 400});
                    if (frame > 0) check(ImGui::GetScrollMaxY() > 0, "Lists inside fixed forms retain genuine scrolling");
                }
                ImGui::EndChild();
                ImGui::End();
                ImGui::SetNextWindowSize({160, 228}, ImGuiCond_Always);
                ImGui::Begin("Legacy padding overflow fixture", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
                ImGui::SetCursorPos({90, 196});
                ImGui::Button("Go", {60, 30});
                if (frame > 0) check(ImGui::GetScrollMaxY() > 0, "Regression fixture reproduces hidden padding overflow");
                ImGui::End();
                ImGui::Render();
            }
            bool first = false, second = false, disabled = false;
            struct { hgeSprite* sprButton[4][9]{}; hgeSprite* sprControl[2][6]{}; } navSkin;
            for (int tick = 0; tick < 20; ++tick) {
                const int frame = tick - 2; // Let the new window settle before injecting input.
                io.AddMousePosEvent(25.f, 45.f);
                if (frame == 1) io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
                if (frame == 2) io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
                if (frame == 3 || frame == 5 || frame == 9 || frame == 15) io.AddKeyEvent(ImGuiKey_Tab, true);
                if (frame == 4 || frame == 6 || frame == 10 || frame == 16) io.AddKeyEvent(ImGuiKey_Tab, false);
                if (frame == 7) io.AddKeyEvent(ImGuiKey_Space, true);
                if (frame == 8) io.AddKeyEvent(ImGuiKey_Space, false);
                if (frame == 9) io.AddKeyEvent(ImGuiMod_Shift, true);
                if (frame == 10) io.AddKeyEvent(ImGuiMod_Shift, false);
                if (frame == 11) io.AddKeyEvent(ImGuiKey_DownArrow, true);
                if (frame == 12) io.AddKeyEvent(ImGuiKey_DownArrow, false);
                if (frame == 13) io.AddKeyEvent(ImGuiKey_Enter, true);
                if (frame == 14) io.AddKeyEvent(ImGuiKey_Enter, false);
                ImGui::NewFrame();
                ImGui::SetNextWindowPos({0, 0});
                ImGui::SetNextWindowSize({280, 200});
                if (tick == 0) ImGui::SetNextWindowFocus();
                ImGui::Begin("Keyboard regression", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                ImGui::SetCursorPos({20, 40});
                WapMapImGui::BitmapCheckbox(renderer, source, navSkin, "First", first);
                if (frame == 2) {
                    check(first, "Mouse click toggles checkbox");
                    check(!WapMapImGui::IsKeyboardFocused(), "Mouse focus does not display keyboard outline");
                }
                if (frame == 4) check(WapMapImGui::IsKeyboardFocused(), "First Tab enters keyboard navigation");
                if (frame == 10) check(WapMapImGui::IsKeyboardFocused(), "Shift+Tab returns keyboard focus to previous checkbox");
                if (frame == 16) check(WapMapImGui::IsKeyboardFocused(), "Tab skips disabled controls and wraps");
                ImGui::SetCursorPos({20, 70});
                WapMapImGui::BitmapCheckbox(renderer, source, navSkin, "Second", second);
                if (frame == 6) check(WapMapImGui::IsKeyboardFocused(), "Tab moves keyboard focus");
                if (frame == 12) check(WapMapImGui::IsKeyboardFocused(), "Arrow keys move keyboard focus");
                if (frame == 8) check(second, "Space activates focused checkbox");
                if (frame == 14) check(!second, "Enter activates focused checkbox");
                ImGui::SetCursorPos({20, 100});
                ImGui::BeginDisabled();
                WapMapImGui::BitmapCheckbox(renderer, source, navSkin, "Disabled", disabled);
                ImGui::EndDisabled();
                check(!disabled, "Disabled checkbox stays unchanged during navigation");
                ImGui::End();
                ImGui::Render();
            }
            // A settings sidebar must share navigation with the surrounding
            // form, rather than trapping Tab inside its child window.
            int category = 0;
            bool field = false;
            struct { hgeSprite* sprControl[2][6]{}; } sidebarSkin;
            for (int tick = 0; tick < 18; ++tick) {
                const int frame = tick - 2;
                io.AddMousePosEvent(25, 45);
                if (frame == 1) io.AddMouseButtonEvent(0, true);
                if (frame == 2) io.AddMouseButtonEvent(0, false);
                if (frame == 3 || frame == 7 || frame == 9 || frame == 13) io.AddKeyEvent(ImGuiKey_Tab, true);
                if (frame == 4 || frame == 8 || frame == 10 || frame == 14) io.AddKeyEvent(ImGuiKey_Tab, false);
                if (frame == 5 || frame == 11) io.AddKeyEvent(ImGuiKey_DownArrow, true);
                if (frame == 6 || frame == 12) io.AddKeyEvent(ImGuiKey_DownArrow, false);
                if (frame == 9) io.AddKeyEvent(ImGuiMod_Shift, true);
                if (frame == 10) io.AddKeyEvent(ImGuiMod_Shift, false);
                ImGui::NewFrame();
                ImGui::SetNextWindowPos({0, 0});
                ImGui::SetNextWindowSize({320, 220});
                if (tick == 0) ImGui::SetNextWindowFocus();
                ImGui::Begin("Settings sidebar regression", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                ImGui::SetCursorPos({20, 40});
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
                if (ImGui::BeginChild("categories", {140, 160}, ImGuiChildFlags_NavFlattened)) {
                    for (int i = 0; i < 3; ++i) {
                        ImGui::PushID(i);
                        ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, i != category);
                        const bool activated = ImGui::InvisibleButton("##category", {140, 30}, ImGuiButtonFlags_EnableNav);
                        ImGui::PopItemFlag();
                        if (activated || WapMapImGui::IsKeyboardFocused()) category = i;
                        if (frame == 10 && i == 1) check(WapMapImGui::IsKeyboardFocused(), "Shift+Tab returns to selected category");
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::SetCursorPos({180, 40});
                WapMapImGui::BitmapCheckbox(renderer, source, sidebarSkin, "Field", field);
                if (frame == 6) check(category == 1, "Down arrow selects next settings category");
                if (frame == 8) check(WapMapImGui::IsKeyboardFocused(), "Tab leaves settings sidebar for form");
                if (frame == 12) check(category == 2, "Category navigation still works after returning from form");
                if (frame == 14) check(WapMapImGui::IsKeyboardFocused(), "Tab leaves sidebar from another category");
                ImGui::End();
                ImGui::Render();
            }
        }
        // Native modal popups must block the legacy-style InvisibleButton
        // controls too, including pointer clicks outside the popup.
        bool backgroundActivated = false;
        for (int frame = 0; frame < 8; ++frame) {
            io.AddMousePosEvent(25, 45);
            if (frame == 3) io.AddMouseButtonEvent(0, true);
            if (frame == 4) io.AddMouseButtonEvent(0, false);
            ImGui::NewFrame();
            ImGui::SetNextWindowPos({0, 0});
            ImGui::SetNextWindowSize({320, 240});
            ImGui::Begin("Modal exclusion regression", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::SetCursorPos({20, 40});
            backgroundActivated |= ImGui::InvisibleButton("background", {100, 25}, ImGuiButtonFlags_EnableNav);
            if (frame == 1) ImGui::OpenPopup("Modal test");
            ImGui::SetNextWindowPos({160, 100});
            ImGui::SetNextWindowSize({150, 100});
            if (ImGui::BeginPopupModal("Modal test", nullptr, ImGuiWindowFlags_NoResize)) {
                ImGui::Button("Confirm");
                if (frame == 6) ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            if (frame == 4) check(io.WantCaptureMouse, "Native modal captures mouse outside its bounds");
            check(!backgroundActivated, "Native modal prevents background control activation");
            ImGui::End();
            ImGui::Render();
        }
        for (int frame = 0; frame < 3; ++frame) {
            io.AddMousePosEvent(frame == 2 ? 90.f : 5.f, frame == 2 ? 110.f : 5.f);
            ImGui::NewFrame();
            ImGui::SetNextWindowPos({60, 60}); ImGui::SetNextWindowSize({160, 120});
            ImGui::Begin("Fresh pointer capture regression", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Button("Panel control");
            ImGui::End();
            ImGui::SetNextWindowPos({0, 0}); ImGui::SetNextWindowSize({30, 30});
            ImGui::Begin("Noninteractive backdrop", nullptr, ImGuiWindowFlags_NoInputs);
            ImGui::End();
            ImGui::Render();
            check(WapMapImGui::CapturePointerAt({90, 110}, false, false), "Newly rendered panel captures pointer before next NewFrame");
            check(!WapMapImGui::CapturePointerAt({5, 5}, false, false), "Pointer can immediately leave panel; noninteractive backdrop does not capture");
            check(!WapMapImGui::CapturePointerAt({90, 110}, true, false), "Legacy drag retains ownership when crossing native panel");
            check(WapMapImGui::CapturePointerAt({5, 5}, false, true), "Native drag retains ownership outside its panel");
            if (frame == 1) check(!io.WantCaptureMouse, "Fixture starts outside panel to reproduce entering capture lag");
            if (frame == 2) check(io.WantCaptureMouse, "Fixture ends inside panel to reproduce leaving capture lag");
        }
        for (int frame = 0; frame < 3; ++frame) {
            ImGui::NewFrame();
            ImGui::SetNextWindowPos({0, 0});
            ImGui::SetNextWindowSize({300, 240});
            ImGui::Begin("Dock host pass-through regression", nullptr,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), {10, 40}, {280, 190});
            ImGui::End();
            ImGui::Render();
            check(!WapMapImGui::CapturePointerAt({150, 120}, false, false),
                  "Empty dock central area passes pointer to welcome links");
            check(WapMapImGui::CapturePointerAt({150, 20}, false, false),
                  "Dock host outside central hole still captures pointer");
        }
        bool tallFormFooterReached = false;
        float tallFormScroll = 0;
        for (int frame = 0; frame < 100; ++frame) {
            io.AddMousePosEvent(5, 5);
            if (frame >= 2) io.AddKeyEvent(ImGuiKey_Tab, frame % 2 == 0);
            ImGui::NewFrame();
            ImGui::SetNextWindowPos({0, 0}); ImGui::SetNextWindowSize({260, 180});
            if (frame == 0) ImGui::SetNextWindowFocus();
            WapMapImGui::BeginFixedWindow("Tall object form regression", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::SetCursorPos({0, 24});
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
            if (ImGui::BeginChild("form", {260, 100}, ImGuiChildFlags_NavFlattened)) {
                for (int i = 0; i < 20; ++i) {
                    ImGui::PushID(i);
                    ImGui::SetCursorPos({20, static_cast<float>(i * 24)});
                    if (frame == 0 && i == 0) ImGui::SetKeyboardFocusHere();
                    ImGui::InvisibleButton("field", {100, 20}, ImGuiButtonFlags_EnableNav);
                    ImGui::PopID();
                }
                tallFormScroll = std::max(tallFormScroll, ImGui::GetScrollY());
            }
            ImGui::EndChild(); ImGui::PopStyleVar();
            ImGui::SetCursorPos({20, 140});
            ImGui::InvisibleButton("Save", {100, 30}, ImGuiButtonFlags_EnableNav);
            tallFormFooterReached |= WapMapImGui::IsKeyboardFocused();
            check(ImGui::GetScrollMaxY() == 0, "Fixed form root has no ghost scroll range");
            ImGui::End(); ImGui::Render();
        }
        io.AddKeyEvent(ImGuiKey_Tab, false);
        check(tallFormScroll > 0 && tallFormFooterReached, "Tab scrolls tall form content and reaches fixed footer");
        bool inventorySourceRemoved = false;
        int inventoryDelivered = -1;
        for (int frame = 0; frame < 10; ++frame) {
            io.AddMousePosEvent(frame < 3 ? 25.f : frame == 3 ? 45.f : 170.f, frame < 3 ? 45.f : 60.f);
            if (frame == 2) io.AddMouseButtonEvent(0, true);
            if (frame == 6) io.AddMouseButtonEvent(0, false);
            ImGui::NewFrame();
            ImGui::SetNextWindowPos({0, 0}); ImGui::SetNextWindowSize({320, 200});
            if (frame == 0) ImGui::SetNextWindowFocus();
            ImGui::Begin("Inventory transfer regression", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            if (!inventorySourceRemoved) {
                ImGui::SetCursorPos({20, 40});
                ImGui::InvisibleButton("source", {50, 50});
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
                    const int item = 23;
                    ImGui::SetDragDropPayload("WapMapInventoryItem", &item, sizeof(item), ImGuiCond_Once);
                    inventorySourceRemoved = true;
                    ImGui::EndDragDropSource();
                }
            }
            ImGui::SetCursorPos({150, 40});
            ImGui::InvisibleButton("destination", {50, 50});
            if (ImGui::BeginDragDropTarget()) {
                if (const auto* payload = ImGui::AcceptDragDropPayload("WapMapInventoryItem"))
                    inventoryDelivered = *static_cast<const int*>(payload->Data);
                ImGui::EndDragDropTarget();
            }
            if (frame == 5) check(ImGui::GetDragDropPayload() != nullptr, "Inventory payload survives a removed/compacted source slot");
            ImGui::End(); ImGui::Render();
        }
        check(inventorySourceRemoved && inventoryDelivered == 23, "Inventory transfer delivers after source widget disappears");
        std::string multiline;
        bool multilineSubmitted = false;
        const std::string longLine(2000, 'a');
        for (int frame = 0; frame < 9; ++frame) {
            io.AddMousePosEvent(5, 5);
            if (frame == 2) io.AddInputCharactersUTF8(longLine.c_str());
            if (frame == 4) io.AddKeyEvent(ImGuiKey_Enter, true);
            if (frame == 5) io.AddKeyEvent(ImGuiKey_Enter, false);
            if (frame == 6) io.AddInputCharactersUTF8("B");
            ImGui::NewFrame();
            ImGui::SetNextWindowPos({0, 0}); ImGui::SetNextWindowSize({320, 200});
            if (frame == 0) ImGui::SetNextWindowFocus();
            ImGui::Begin("Multiline object text regression", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            if (frame == 0) ImGui::SetKeyboardFocusHere();
            MapTextInput::Multiline("text", multiline, {280, 120});
            multilineSubmitted |= ImGui::InvisibleButton("Save", {100, 25}, ImGuiButtonFlags_EnableNav);
            ImGui::End(); ImGui::Render();
        }
        check(multiline == longLine + "\nB" && !multilineSubmitted, "Multiline input grows dynamically and Enter inserts a line without submitting the form");
        renderer.Texture_Free(texture);
        ImGui::DestroyContext();
        const std::string polishByte(1, static_cast<char>(0xB3));
        check(LegacyText::ToUtf8(polishByte, 1250) == "\xC5\x82", "Decode legacy Polish metadata");
        check(LegacyText::FromUtf8("\xC5\x82", 1250, 1) == polishByte, "Count encoded bytes, not UTF-8 bytes");
        check(!LegacyText::FromUtf8(std::string(59, 'a'), 1250, 58), "Enforce level-name byte limit");
        check(!LegacyText::FromUtf8("\xF0\x9F\x98\x80", 1250, 63), "Reject lossy conversion");
        check(!LegacyText::FromUtf8("\xC5", 1250, 63), "Reject incomplete UTF-8");
        check(LegacyText::FromUtf8("\xC5\x82", CP_UTF8, 2) == "\xC5\x82", "UTF-8 system code pages preserve path text");
        check(!LegacyText::FromUtf8("\xC5\x82", CP_UTF8, 1), "UTF-8 path byte limits remain enforced");
        check(LegacyText::FromUtf8("C:\\Games\\Claw", CP_ACP, 4096) == "C:\\Games\\Claw", "System-code-page game path conversion");
        std::cout << "Bitmap font pixel, metric and ImGui frame checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
