#pragma once

#ifdef WAPMAP_ENABLE_IMGUI
#include <imgui.h>
#include <hgefont.h>
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>

namespace WapMapImGui {

inline bool IsKeyboardFocused() {
    // Item focus also follows mouse clicks. NavVisible tracks whether ImGui
    // is currently displaying keyboard/gamepad navigation feedback.
    return ImGui::IsItemFocused() && ImGui::GetIO().NavVisible &&
           !ImGui::IsMouseDown(ImGuiMouseButton_Left);
}

inline ImVec4 Grey(int value, float alpha = 1.f) {
    const float channel = value / 255.f;
    return {channel, channel, channel, alpha};
}

inline ImVec2 ClampWindowPositionToMonitorWorkArea(ImVec2 position, ImVec2 size) {
    const auto& monitors = ImGui::GetPlatformIO().Monitors;
    const ImGuiPlatformMonitor* bestMonitor = nullptr;
    float bestIntersection = 0.f;
    for (int i = 0; i < monitors.Size; ++i) {
        const auto& monitor = monitors[i];
        const float right = monitor.WorkPos.x + monitor.WorkSize.x;
        const float bottom = monitor.WorkPos.y + monitor.WorkSize.y;
        const float windowRight = position.x + size.x;
        const float windowBottom = position.y + size.y;
        if (position.x >= monitor.WorkPos.x && position.y >= monitor.WorkPos.y &&
            windowRight <= right && windowBottom <= bottom) {
            return position;
        }
        const float intersectionWidth = std::max(0.f, std::min(windowRight, right) - std::max(position.x, monitor.WorkPos.x));
        const float intersectionHeight = std::max(0.f, std::min(windowBottom, bottom) - std::max(position.y, monitor.WorkPos.y));
        const float intersection = intersectionWidth * intersectionHeight;
        if (intersection > bestIntersection) {
            bestIntersection = intersection;
            bestMonitor = &monitor;
        }
    }

    ImVec2 workPosition;
    ImVec2 workSize;
    if (bestMonitor) {
        workPosition = bestMonitor->WorkPos;
        workSize = bestMonitor->WorkSize;
    } else {
        const auto* viewport = ImGui::GetMainViewport();
        workPosition = viewport->WorkPos;
        workSize = viewport->WorkSize;
    }
    const float maxX = workPosition.x + std::max(0.f, workSize.x - size.x);
    const float maxY = workPosition.y + std::max(0.f, workSize.y - size.y);
    return {std::clamp(position.x, workPosition.x, maxX),
            std::clamp(position.y, workPosition.y, maxY)};
}

inline void KeepWindowOnScreen() {
    if (ImGui::IsWindowDocked()) return;

    const auto position = ImGui::GetWindowPos();
    const auto clampedPosition = ClampWindowPositionToMonitorWorkArea(position, ImGui::GetWindowSize());
    if (clampedPosition.x != position.x || clampedPosition.y != position.y) {
        ImGui::SetWindowPos(clampedPosition, ImGuiCond_Always);
        auto* viewport = ImGui::GetWindowViewport();
        if (viewport != ImGui::GetMainViewport()) {
            const ImVec2 offset{clampedPosition.x - viewport->Pos.x,
                                clampedPosition.y - viewport->Pos.y};
            viewport->Pos = clampedPosition;
            viewport->WorkPos.x += offset.x;
            viewport->WorkPos.y += offset.y;
        }
    }
}

// Legacy forms already include their own margins in absolute control positions.
// Only their list/grid children should scroll, never the enclosing form.
inline bool BeginFixedWindow(const char* title, bool* open, ImGuiWindowFlags flags,
                             bool dockable = true) {
    ImGui::SetNextWindowScroll({0.f, 0.f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
    const auto dockingFlags = dockable ? ImGuiWindowFlags_None : ImGuiWindowFlags_NoDocking;
    const bool visible = ImGui::Begin(title, open, flags | dockingFlags |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();
    KeepWindowOnScreen();
    return visible;
}

inline void ApplyTheme() {
    auto& style = ImGui::GetStyle();
    ImGui::GetIO().ConfigWindowsResizeFromEdges = true;
    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg]=ImVec4(0.f,0.f,0.f,90.f/255.f);
    style.FontSizeBase = 16.f;
    style.WindowPadding = {8.f, 8.f};
    style.FramePadding = {4.f, 4.f};
    style.ItemSpacing = {6.f, 6.f};
    style.WindowRounding = style.ChildRounding = style.PopupRounding = 0.f;
    style.FrameRounding = style.ScrollbarRounding = style.GrabRounding = style.TabRounding = 0.f;
    style.WindowBorderSize = style.ChildBorderSize = style.PopupBorderSize = 1.f;
    style.FrameBorderSize = 1.f;
    style.WindowTitleAlign = {0.5f, 0.5f};
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ScrollbarSize = 12.f;
    style.DisabledAlpha = 119.f / 255.f;
    auto* colors = style.Colors;
    colors[ImGuiCol_Text] = Grey(0xe1);
    colors[ImGuiCol_TextDisabled] = Grey(0xe1, 0.46f);
    colors[ImGuiCol_WindowBg] = colors[ImGuiCol_PopupBg] = Grey(0x2f);
    colors[ImGuiCol_ChildBg] = Grey(0x2f);
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive] =
        colors[ImGuiCol_TitleBgCollapsed] = Grey(0x37);
    colors[ImGuiCol_Border] = Grey(0x58);
    colors[ImGuiCol_BorderShadow] = Grey(0x07, 0.5f);
    colors[ImGuiCol_FrameBg] = Grey(0x27);
    colors[ImGuiCol_FrameBgHovered] = Grey(0x3e);
    colors[ImGuiCol_FrameBgActive] = Grey(0x4d);
    colors[ImGuiCol_Button] = colors[ImGuiCol_Header] = Grey(0x3e);
    colors[ImGuiCol_ButtonHovered] = colors[ImGuiCol_HeaderHovered] = Grey(0x4d);
    colors[ImGuiCol_ButtonActive] = colors[ImGuiCol_HeaderActive] = Grey(0x58);
    colors[ImGuiCol_Separator] = Grey(0x56, 0.5f);
    colors[ImGuiCol_SeparatorHovered] = colors[ImGuiCol_SeparatorActive] = Grey(0x58);
    colors[ImGuiCol_ScrollbarBg] = Grey(0x27);
    colors[ImGuiCol_ScrollbarGrab] = Grey(0x4d);
    colors[ImGuiCol_ScrollbarGrabHovered] = Grey(0x58);
    colors[ImGuiCol_ScrollbarGrabActive] = Grey(0x68);
    colors[ImGuiCol_CheckMark] = colors[ImGuiCol_SliderGrabActive] = {0x15 / 255.f, 0x85 / 255.f, 0xe2 / 255.f, 1.f};
    colors[ImGuiCol_SliderGrab] = Grey(0x68);
    colors[ImGuiCol_Tab] = Grey(0x37);
    colors[ImGuiCol_TabHovered] = Grey(0x4d);
    colors[ImGuiCol_TabSelected] = Grey(0x58);
    colors[ImGuiCol_TabDimmed] = Grey(0x2f);
    colors[ImGuiCol_TabDimmedSelected] = Grey(0x3e);
}

// Submit the existing atlas to ImGui, rather than drawing through HGE: these
// commands travel with the window's viewport and work in detached OS windows.
inline void Sprite(ImDrawList* draw, HGE& renderer, const hgeSprite* sprite,
                   ImVec2 position, ImU32 color = IM_COL32_WHITE) {
    if (!sprite) return;
    const auto texture = sprite->GetTexture();
    const auto handle = renderer.Texture_GetNativeHandle(texture);
    const float width = static_cast<float>(renderer.Texture_GetWidth(texture));
    const float height = static_cast<float>(renderer.Texture_GetHeight(texture));
    if (!handle || width <= 0 || height <= 0) return;
    float x, y, w, h, hotX, hotY;
    sprite->GetTextureRect(&x, &y, &w, &h);
    sprite->GetHotSpot(&hotX, &hotY);
    draw->AddImage(static_cast<ImTextureID>(handle),
                   {position.x - hotX, position.y - hotY},
                   {position.x - hotX + w, position.y - hotY + h},
                   {x / width, y / height}, {(x + w) / width, (y + h) / height}, color);
}

inline void StretchSprite(ImDrawList* draw, HGE& renderer, const hgeSprite* sprite,
                          ImVec2 minimum, ImVec2 maximum, ImU32 color) {
    if (!sprite) return;
    const auto texture = sprite->GetTexture();
    const float tw = static_cast<float>(renderer.Texture_GetWidth(texture));
    const float th = static_cast<float>(renderer.Texture_GetHeight(texture));
    const auto handle = renderer.Texture_GetNativeHandle(texture);
    if (!handle || tw <= 0 || th <= 0) return;
    float x, y, w, h;
    sprite->GetTextureRect(&x, &y, &w, &h);
    draw->AddImage(static_cast<ImTextureID>(handle), minimum, maximum,
                   {x / tw, y / th}, {(x + w) / tw, (y + h) / th}, color);
}

// Keep the original glyph pixels AND advances in native editable ImGui fields.
// Import before NewFrame, then upload the completed atlas through imgui-sfml.
inline ImFont* ImportBitmapFont(HGE& renderer, const hgeFont& source, UINT codePage = 1252) {
    auto* atlas = ImGui::GetIO().Fonts;
    ImFontConfig config;
    config.SizePixels = source.GetHeight();
    auto* font = atlas->AddFontDefault(&config);
    unsigned char* rgba;
    int atlasWidth, atlasHeight;
    atlas->GetTexDataAsRGBA32(&rgba, &atlasWidth, &atlasHeight);
    struct Glyph { ImFontAtlasRectId rect; std::vector<DWORD> pixels; int width; int height; };
    std::vector<Glyph> glyphs;
    for (int code = 32; code < 256; ++code) {
        const char byte = static_cast<char>(code);
        wchar_t unicode{};
        if (MultiByteToWideChar(codePage, 0, &byte, 1, &unicode, 1) != 1 ||
            unicode < 32 || (unicode >= 127 && unicode < 160)) continue;
        // The custom-rect API discards an existing glyph before replacement.
        // ImGui forbids discarding its cached fallback and ellipsis glyphs.
        if (unicode == font->FallbackChar || unicode == font->EllipsisChar) continue;
        const auto* sprite = source.GetSprite(static_cast<char>(code));
        if (!sprite) continue;
        float x, y, w, h, hotX, hotY;
        sprite->GetTextureRect(&x, &y, &w, &h);
        sprite->GetHotSpot(&hotX, &hotY);
        const int width = static_cast<int>(w), height = static_cast<int>(h);
        if (width <= 0 || height <= 0) continue;
        auto* pixels = renderer.Texture_Lock(sprite->GetTexture(), true);
        if (!pixels) continue;
        Glyph glyph{ImFontAtlasRectId_Invalid, {}, width, height};
        const int stride = renderer.Texture_GetWidth(sprite->GetTexture());
        for (int row = 0; row < height; ++row)
            for (int column = 0; column < width; ++column)
                glyph.pixels.push_back(pixels[(static_cast<int>(y) + row) * stride + static_cast<int>(x) + column]);
        renderer.Texture_Unlock(sprite->GetTexture());
        const char text[] = {static_cast<char>(code), 0};
        glyph.rect = atlas->AddCustomRectFontGlyphForSize(font, config.SizePixels,
            static_cast<ImWchar>(unicode), width, height, source.GetStringWidth(text, false, true),
            ImVec2(source.GetPreWidth(text[0]) - hotX, -hotY));
        font->GetFontBaked(config.SizePixels)->Glyphs.back().Colored = false;
        glyphs.push_back(std::move(glyph));
    }
    // Packing may grow the atlas: resolve rectangles only after all additions.
    atlas->GetTexDataAsRGBA32(&rgba, &atlasWidth, &atlasHeight);
    for (const auto& glyph : glyphs) {
        ImFontAtlasRect rectangle;
        if (!atlas->GetCustomRect(glyph.rect, &rectangle)) continue;
        for (int row = 0; row < glyph.height; ++row) {
            for (int column = 0; column < glyph.width; ++column) {
                const auto color = glyph.pixels[row * glyph.width + column];
                const int offset = ((rectangle.y + row) * atlasWidth + rectangle.x + column) * 4;
                rgba[offset] = GETR(color); rgba[offset + 1] = GETG(color);
                rgba[offset + 2] = GETB(color); rgba[offset + 3] = GETA(color);
            }
        }
    }
    return font;
}

inline void BitmapText(ImDrawList* draw, HGE& renderer, const hgeFont& font,
                       ImVec2 position, const char* text, ImU32 color = IM_COL32(225, 225, 225, 255)) {
    position.x = std::floor(position.x);
    position.y = std::floor(position.y);
    const float startX = position.x;
    for (; *text; ++text) {
        if (*text == '\r') continue;
        if (*text == '\n') {
            position.x = startX;
            position.y += font.GetHeight();
            continue;
        }
        const char character = font.GetSprite(*text) ? *text : '?';
        Sprite(draw, renderer, font.GetSprite(character),
               {position.x + font.GetPreWidth(character), position.y}, color);
        const char glyph[] = {character, '\0'};
        position.x += font.GetStringWidth(glyph, false, true);
    }
}

inline void WindowChrome(HGE& renderer, const hgeFont& font, const hgeSprite* closeSprite, const char* title) {
    auto* draw = ImGui::GetWindowDrawList();
    const auto position = ImGui::GetWindowPos();
    const auto size = ImGui::GetWindowSize();
    if (!ImGui::IsWindowDocked()) {
        draw->PushClipRect(position, ImVec2(position.x + size.x, position.y + size.y), false);
        draw->AddRectFilled(ImVec2(position.x + 1.f, position.y + 1.f),
                            ImVec2(position.x + size.x - 1.f, position.y + 23.f), IM_COL32(55, 55, 55, 255));
        draw->AddLine(ImVec2(position.x + 1.f, position.y + 1.f),
                      ImVec2(position.x + size.x - 1.f, position.y + 1.f), IM_COL32(88, 88, 88, 255));
        draw->AddLine(ImVec2(position.x, position.y + 23.f),
                      ImVec2(position.x + size.x, position.y + 23.f), IM_COL32(39, 39, 39, 255));
        draw->AddLine(ImVec2(position.x + 1.f, position.y + size.y - 1.f),
                      ImVec2(position.x + size.x - 1.f, position.y + size.y - 1.f), IM_COL32(42, 42, 42, 255));
        const float titleWidth = font.GetStringWidth(title);
        WapMapImGui::BitmapText(draw, renderer, font,
            ImVec2(position.x + (size.x - titleWidth) * 0.5f, position.y + 3.f),
            title, IM_COL32_WHITE);
        const bool closeHovered = ImGui::IsWindowHovered() &&
            ImGui::IsMouseHoveringRect(ImVec2(position.x + size.x - 24.f, position.y),
                                      ImVec2(position.x + size.x, position.y + 24.f), false);
        WapMapImGui::Sprite(draw, renderer, closeSprite,
            ImVec2(position.x + size.x - 15.f, position.y + 6.f),
            IM_COL32(255, 255, 255, closeHovered ? 255 : 130));
        draw->PopClipRect();
    }

}

template <typename Skin>
inline bool BitmapCheckbox(HGE& renderer, const hgeFont& font, const Skin& skin, const char* label, bool& selected) {
    ImGui::PushID(label);
    const bool clicked = ImGui::InvisibleButton("##checkbox",
        ImVec2(20.f + font.GetStringWidth(label), 17.f), ImGuiButtonFlags_EnableNav);
    ImGui::PopID();
    const auto minimum = ImGui::GetItemRectMin();
    auto* draw = ImGui::GetWindowDrawList();
    const bool enabled = ImGui::GetStyle().Alpha >= 0.99f;
    const bool hovered = enabled && ImGui::IsItemHovered();
    Sprite(draw, renderer, skin.sprControl[1][enabled ? 2 : 4], minimum);
    if (enabled && (selected || hovered)) Sprite(draw, renderer, skin.sprControl[1][3], minimum);
    // Selection already uses the hover sprite at full opacity. Add a small
    // brightness lift beneath the check mark so hovering remains visible.
    if (selected && hovered)
        draw->AddRectFilled({minimum.x + 1.f, minimum.y + 1.f},
                            {minimum.x + 14.f, minimum.y + 14.f},
                            IM_COL32(255, 255, 255, 28), 2.f);
    if (selected) Sprite(draw, renderer, skin.sprControl[1][enabled ? 0 : 1], minimum);
    if (IsKeyboardFocused()) Sprite(draw, renderer, skin.sprControl[1][5], {minimum.x - 1.f, minimum.y - 1.f});
    BitmapText(draw, renderer, font, {minimum.x + 20.f, minimum.y}, label,
        IM_COL32(hovered ? 255 : 225, hovered ? 255 : 225, hovered ? 255 : 225,
                 static_cast<int>(255.f * ImGui::GetStyle().Alpha)));
    if (clicked) selected = !selected;
    return clicked;
}

template <typename Skin>
inline bool BitmapRadio(HGE& renderer, const hgeFont& font, const Skin& skin, const char* label, int& value, int choice) {
    ImGui::PushID(label);
    ImGui::PushID(choice);
    const bool clicked = ImGui::InvisibleButton("##select",
        ImVec2(21.f + font.GetStringWidth(label), 17.f), ImGuiButtonFlags_EnableNav);
    ImGui::PopID();
    ImGui::PopID();
    const auto minimum = ImGui::GetItemRectMin();
    const bool enabled = ImGui::GetStyle().Alpha >= 0.99f;
    auto* draw = ImGui::GetWindowDrawList();
    Sprite(draw, renderer, skin.sprControl[0][enabled ? 2 : 4], minimum);
    if (enabled && (value == choice || ImGui::IsItemHovered()))
        Sprite(draw, renderer, skin.sprControl[0][3], minimum);
    if (value == choice) Sprite(draw, renderer, skin.sprControl[0][enabled ? 0 : 1], minimum);
    if (IsKeyboardFocused()) Sprite(draw, renderer, skin.sprControl[0][5], {minimum.x - 1.f, minimum.y - 1.f});
    BitmapText(ImGui::GetWindowDrawList(), renderer, font,
        ImVec2(minimum.x + 21.f, minimum.y), label,
        IM_COL32(225, 225, 225, static_cast<int>(255.f * ImGui::GetStyle().Alpha)));
    if (clicked) value = choice;
    return clicked;
}

template <typename Skin>
inline bool BitmapButton(HGE& renderer, const hgeFont& font, const Skin& skin, const char* label, ImVec2 size) {
    ImGui::PushID(label);
    const bool clicked = ImGui::InvisibleButton("##button", size, ImGuiButtonFlags_EnableNav);
    ImGui::PopID();
    const auto minimum = ImGui::GetItemRectMin();
    const auto maximum = ImGui::GetItemRectMax();
    auto* draw = ImGui::GetWindowDrawList();
    const bool enabled = ImGui::GetStyle().Alpha >= 0.99f;
    const int state = enabled ? 2 : 0;
    const float xs[] = {minimum.x, minimum.x + 4.f, maximum.x - 4.f, maximum.x};
    const float ys[] = {minimum.y, minimum.y + 4.f, maximum.y - 4.f, maximum.y};
    const auto paint = [&](int skinState, int alpha) {
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 3; ++column)
                StretchSprite(draw, renderer, skin.sprButton[skinState][row * 3 + column],
                    {xs[column], ys[row]}, {xs[column + 1], ys[row + 1]}, IM_COL32(255, 255, 255, alpha));
    };
    paint(state, 255);
    if (enabled && (ImGui::IsItemHovered() || IsKeyboardFocused() || ImGui::IsItemActive()))
        paint(3, ImGui::IsItemActive() ? 255 : 125);
    BitmapText(ImGui::GetWindowDrawList(), renderer, font,
        ImVec2(minimum.x + (maximum.x - minimum.x - font.GetStringWidth(label)) * 0.5f,
               minimum.y + (maximum.y - minimum.y - font.GetHeight()) * 0.5f - 2.f), label,
        IM_COL32(225, 225, 225, static_cast<int>(255.f * ImGui::GetStyle().Alpha)));
    return clicked;
}

inline void WrappedBitmapText(HGE& renderer, const hgeFont& font, const char* text, float width) {
    std::string line;
    const auto origin = ImGui::GetCursorScreenPos();
    float y = origin.y;
    const auto emit = [&]() {
        BitmapText(ImGui::GetWindowDrawList(), renderer, font, {origin.x, y}, line.c_str());
        y += font.GetHeight();
        line.clear();
    };
    for (const char* cursor = text; *cursor;) {
        if (*cursor == '\n') { emit(); ++cursor; continue; }
        if (*cursor == ' ' || *cursor == '\r') { ++cursor; continue; }
        const char* start = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\n' && *cursor != '\r') ++cursor;
        const std::string word(start, cursor);
        const auto candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && font.GetStringWidth(candidate.c_str(), false, true) > width) emit();
        if (!line.empty()) line += ' ';
        line += word;
    }
    emit();
    ImGui::Dummy({width, y - origin.y});
}

inline void WrappedBitmapMarkupText(HGE& renderer, const hgeFont& font, const char* text, float width) {
    const auto origin = ImGui::GetCursorScreenPos();
    auto* draw = ImGui::GetWindowDrawList();
    float x = 0, y = 0;
    ImU32 color = IM_COL32(225, 225, 225, 255);
    while (*text) {
        if (*text == '\n' || std::strncmp(text, "~n~", 3) == 0) {
            text += *text == '\n' ? 1 : 3;
            x = 0; y += font.GetHeight(); continue;
        }
        if (*text == '\r') { ++text; continue; }
        if (*text == ' ') {
            if (x > 0) x += font.GetStringWidth(" ", false, true);
            ++text; continue;
        }
        const char* end = text;
        while (*end && *end != ' ' && *end != '\n' && *end != '\r' && std::strncmp(end, "~n~", 3) != 0) ++end;
        const std::string word(text, end);
        if (x > 0 && x + font.GetStringWidth(word.c_str(), false) > width) { x = 0; y += font.GetHeight(); }
        while (text < end) {
            if (end - text >= 3 && text[0] == '~' && text[2] == '~') {
                switch (text[1]) {
                    case 'a': color = IM_COL32(119, 119, 119, 255); break;
                    case 'b': color = IM_COL32(0, 0, 255, 255); break;
                    case 'g': color = IM_COL32(76, 214, 143, 255); break;
                    case 'l': color = IM_COL32(0, 0, 0, 255); break;
                    case 'p': color = IM_COL32(255, 0, 255, 255); break;
                    case 'r': color = IM_COL32(255, 0, 0, 255); break;
                    case 'w': color = IM_COL32(225, 225, 225, 255); break;
                    case 'y': color = IM_COL32(255, 255, 0, 255); break;
                    default: break;
                }
                text += 3;
            } else {
                const char glyph[] = {*text++, '\0'};
                BitmapText(draw, renderer, font, {origin.x + x, origin.y + y}, glyph, color);
                x += font.GetStringWidth(glyph, false, true);
            }
        }
    }
    ImGui::Dummy({width, y + font.GetHeight()});
}

} // namespace WapMapImGui
#endif
