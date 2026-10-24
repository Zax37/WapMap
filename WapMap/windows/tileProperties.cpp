#include "tileProperties.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../databanks/tiles.h"
#include "../cInterfaceSheet.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include "coordinateNavigation.h"
#include "tilePropertyGeometry.h"
#include <cstdio>

namespace {
bool validSize(int w, int h, int type) {
    return TilePropertyGeometry::ValidSize(w, h, type == WWD::AttribType_Mask);
}
ImU32 color(WWD::TILE_ATTRIB value) {
    switch (value) {
    case WWD::Attrib_Clear: return 0;
    case WWD::Attrib_Solid: return IM_COL32(0, 0, 255, 119);
    case WWD::Attrib_Ground: return IM_COL32(0, 255, 0, 119);
    case WWD::Attrib_Climb: return IM_COL32(255, 255, 0, 119);
    case WWD::Attrib_Death: return IM_COL32(255, 0, 0, 119);
    default: return IM_COL32(255, 255, 255, 119);
    }
}
}

winTileProperties::winTileProperties() : cWindow(GETL(Lang_TileProperties), 300, 350) {}
winTileProperties::~winTileProperties() = default;

void winTileProperties::load(int id) {
    auto& e = *GV->editState;
    tileID = id;
    e.itpSelectedTile = id;
    e.tftpTileID->setText(std::to_string(id));
    dragging = apply = false;
    inside = outside = 0;
    auto* saved = id >= 0 ? document->GetTileAttribs(id) : nullptr;
    draft.reset();
    if (saved) {
        switch (saved->getType()) {
        case WWD::AttribType_Single:
            draft = std::make_unique<WWD::SingleTileAttrib>(static_cast<WWD::SingleTileAttrib*>(saved));
            inside = static_cast<WWD::SingleTileAttrib*>(saved)->getAttrib(); break;
        case WWD::AttribType_Double:
            draft = std::make_unique<WWD::DoubleTileAttrib>(static_cast<WWD::DoubleTileAttrib*>(saved));
            inside = static_cast<WWD::DoubleTileAttrib*>(saved)->getInsideAttrib();
            outside = static_cast<WWD::DoubleTileAttrib*>(saved)->getOutsideAttrib(); break;
        case WWD::AttribType_Mask:
            draft = std::make_unique<WWD::MaskTileAttrib>(static_cast<WWD::MaskTileAttrib*>(saved)); break;
        }
    }
    if (!draft) {
        auto* main = document->GetMainPlane();
        draft = std::make_unique<WWD::SingleTileAttrib>(main->GetTileWidth(), main->GetTileHeight(), WWD::Attrib_Clear);
    }
    inside = std::clamp(inside, 0, 4); outside = std::clamp(outside, 0, 4);
    const auto set = [&](int i, int value) { std::snprintf(fields[i].data(), fields[i].size(), "%d", value); };
    set(0, id); set(1, draft->getWidth()); set(2, draft->getHeight());
    auto rect = draft->getType() == WWD::AttribType_Double ?
        static_cast<WWD::DoubleTileAttrib*>(draft.get())->getMask() : WWD::Rect{};
    set(3, rect.x1); set(4, rect.y1); set(5, rect.x2); set(6, rect.y2);
    // Keep the retained editor's selected-tile draft synchronized for map preview.
    e.SyncAttribMenuWithTile();
    e.winTileProp->setVisible(false);
}

void winTileProperties::resize(int w, int h, int type) {
    if (!validSize(w, h, type)) return;
    const int oldType = draft->getType();
    if (type == WWD::AttribType_Mask) {
        // TileAttrib::setWidth/setHeight do not resize MaskTileAttrib's buffer.
        auto replacement = std::make_unique<WWD::MaskTileAttrib>(w, h);
        if (oldType == type) {
            auto* old = static_cast<WWD::MaskTileAttrib*>(draft.get());
            const auto* data = old->getData();
            TilePropertyGeometry::PreserveMask(data, old->getWidth(), old->getHeight(), w, h,
                [&](int x, int y, WWD::TILE_ATTRIB value) { replacement->setArea(x, y, x, y, value); });
        }
        draft = std::move(replacement);
    } else if (type == WWD::AttribType_Double) {
        auto rect = oldType == type ? static_cast<WWD::DoubleTileAttrib*>(draft.get())->getMask() : WWD::Rect(0, 0, w - 1, h - 1);
        rect.x1 = std::clamp(rect.x1, 0, w - 1); rect.x2 = std::clamp(rect.x2, rect.x1, w - 1);
        rect.y1 = std::clamp(rect.y1, 0, h - 1); rect.y2 = std::clamp(rect.y2, rect.y1, h - 1);
        if (oldType != type) outside = 0;
        draft = std::make_unique<WWD::DoubleTileAttrib>(w, h, static_cast<WWD::TILE_ATTRIB>(inside), static_cast<WWD::TILE_ATTRIB>(outside), rect);
        const int values[] = {rect.x1, rect.y1, rect.x2, rect.y2};
        for (int i = 0; i < 4; ++i) std::snprintf(fields[i + 3].data(), fields[i + 3].size(), "%d", values[i]);
    } else {
        draft = std::make_unique<WWD::SingleTileAttrib>(w, h, static_cast<WWD::TILE_ATTRIB>(inside));
    }
    dragging = false;
}

void winTileProperties::Open() {
    auto& e = *GV->editState;
    if (!e.hParser || !e.hParser->GetMainPlane()) return;
    document = e.hParser;
    load(e.itpSelectedTile);
    open = positionOnOpen = true;
}
void winTileProperties::Close() {
    open = apply = dragging = false;
    document = nullptr;
    draft.reset();
    cWindow::Close();
}

void winTileProperties::DrawImGui() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !document || !document->GetMainPlane()) { Close(); return; }
    if (!e._isOnTop()) return;
    if (tileID != e.itpSelectedTile) load(e.itpSelectedTile);
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus(); positionOnOpen = false;
    }
    int type = draft->getType();
    ImGui::SetNextWindowSize({300.f, type == WWD::AttribType_Double ? 350.f : 260.f}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL(Lang_TileProperties), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapTileProperties";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_TileProperties));
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) dragging = false;
        const auto origin = ImGui::GetWindowPos();
        const auto area = myWin.getChildrenArea();
        const auto position = [&](float x, float y) { ImGui::SetCursorPos({area.x + x, area.y + y}); };
        auto* draw = ImGui::GetWindowDrawList();
        const auto label = [&](const char* text, float x, float y) { WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, {origin.x + x, origin.y + y}, text); };
        const auto button = [&](SHR::But* b, const char* id) {
            ImGui::PushID(id); position(static_cast<float>(b->getX()), static_cast<float>(b->getY()));
            const bool hit = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "", {static_cast<float>(b->getWidth()), static_cast<float>(b->getHeight())});
            const auto p = ImGui::GetItemRectMin(); auto* icon = b->getIcon();
            if (icon) WapMapImGui::Sprite(draw, *hge, icon, {p.x + (b->getWidth() - icon->GetWidth()) / 2, p.y + (b->getHeight() - icon->GetHeight()) / 2});
            ImGui::PopID(); return hit;
        };
        auto* main = document->GetMainPlane();
        auto* set = main->GetImageSetsCount() ? e.hTileset->GetSet(main->GetImageSet(0)) : nullptr;
        const auto navigate = [&](int step) {
            if (!set || !set->GetTilesCount()) return;
            int index = step > 0 ? -1 : 0;
            for (int i = 0; i < set->GetTilesCount(); ++i)
                if (set->GetTileByIterator(i)->GetID() == tileID) { index = i; break; }
            index = (index + step + set->GetTilesCount()) % set->GetTilesCount();
            load(set->GetTileByIterator(index)->GetID());
        };
        ImGui::BeginDisabled(!set || !set->GetTilesCount());
        if (button(e.buttpPrev, "previous")) navigate(-1);
        if (button(e.buttpNext, "next")) navigate(1);
        ImGui::EndDisabled();
        if (button(e.buttpZoom, "zoom")) e.btpZoomTile = !e.btpZoomTile;
        if (button(e.buttpShow, "show")) e.hAppMenu->GetContext(AppMenu_View)->EmulateClickID(APPMEN_VIEW_TILEPROP);
        type = draft->getType();
        SHR::TextField* targets[] = {e.tftpTileID, e.tftpW, e.tftpH, e.tftpX1, e.tftpY1, e.tftpX2, e.tftpY2};
        bool resizeRequested = false;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {5.f, std::max(0.f, (20.f - GV->fntMyriad16->GetHeight()) / 2)});
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, WapMapImGui::Grey(0x23));
        for (int i = 0; i < (type == WWD::AttribType_Double ? 7 : 3); ++i) {
            ImGui::PushID(i); position(static_cast<float>(targets[i]->getX()), static_cast<float>(targets[i]->getY()));
            ImGui::SetNextItemWidth(static_cast<float>(targets[i]->getWidth()));
            const bool entered = ImGui::InputText("##value", fields[i].data(), i == 0 ? 32 : 5,
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue);
            if (i == 0 && (entered || ImGui::IsItemDeactivatedAfterEdit())) {
                const auto id = CoordinateNavigation::Parse(fields[0].data());
                if (id && *id >= -1 && *id <= 32767) load(*id);
            }
            if ((i == 1 || i == 2) && (entered || ImGui::IsItemDeactivatedAfterEdit())) resizeRequested = true;
            ImGui::PopID();
        }
        ImGui::PopStyleColor(); ImGui::PopStyleVar(3);
        auto w = CoordinateNavigation::Parse(fields[1].data()), h = CoordinateNavigation::Parse(fields[2].data());
        bool valid = w && h && validSize(*w, *h, type);
        // Intermediate text (e.g. 64 -> 6 -> 65) must not crop the mask.
        if (valid && resizeRequested && (*w != draft->getWidth() || *h != draft->getHeight())) resize(*w, *h, type);
        label((std::string(GETL(Lang_Width)) + ':').c_str(), 145, 66);
        label((std::string(GETL(Lang_Height)) + ':').c_str(), 145, 91);
        label((std::string(GETL(Lang_Type)) + ':').c_str(), 145, 114);
        SHR::RadBut* types[] = {e.rbtpSingle, e.rbtpDouble, e.rbtpMask};
        ImGui::BeginDisabled(!valid);
        for (int i = 0; i < 3; ++i) {
            ImGui::PushID(i + 20); position(145, 120.f + i * 20);
            if (WapMapImGui::BitmapRadio(*hge, *GV->fntMyriad16, *GV->hGfxInterface, types[i]->getCaption().c_str(), type, i + 1))
                resize(*w, *h, type);
            ImGui::PopID();
        }
        ImGui::EndDisabled();
        type = draft->getType();
        const int captions[] = {Lang_PropClear, Lang_PropSolid, Lang_PropGround, Lang_PropClimb, Lang_PropDeath};
        for (int group = 0; group < (type == WWD::AttribType_Double ? 2 : 1); ++group) {
            label((std::string(GETL(group ? Lang_AttribOutside : (type == WWD::AttribType_Mask ? Lang_AttribMaskType : Lang_AttribInside))) + ':').c_str(), 8, 208.f + group * 60);
            for (int i = 0; i < 5; ++i) {
                const float x = i < 2 ? 107.f + i * 100 : 7.f + (i - 2) * 100;
                const float y = (i < 2 ? 192.f : 222.f) + group * 60;
                ImGui::PushID(40 + group * 5 + i); position(x, y);
                WapMapImGui::BitmapRadio(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(captions[i]), group ? outside : inside, i);
                ImGui::PopID();
            }
        }
        if (type == WWD::AttribType_Single) static_cast<WWD::SingleTileAttrib*>(draft.get())->setAttrib(static_cast<WWD::TILE_ATTRIB>(inside));
        if (type == WWD::AttribType_Double) {
            auto* a = static_cast<WWD::DoubleTileAttrib*>(draft.get());
            a->setInsideAttrib(static_cast<WWD::TILE_ATTRIB>(inside)); a->setOutsideAttrib(static_cast<WWD::TILE_ATTRIB>(outside));
            int values[4]{};
            for (int i = 0; i < 4; ++i) {
                auto v = CoordinateNavigation::Parse(fields[i + 3].data());
                if (!v) valid = false; else values[i] = *v;
            }
            if (valid) {
                values[0] = std::clamp(values[0], 0, *w - 1); values[2] = std::clamp(values[2], values[0], *w - 1);
                values[1] = std::clamp(values[1], 0, *h - 1); values[3] = std::clamp(values[3], values[1], *h - 1);
                a->setMask(values[0], values[1], values[2], values[3]);
            }
            label(GETL(Lang_Mask), 65, 327); label("x", 150, 327); label("-", 202, 325); label("x", 250, 327);
        }
        const auto requestedID = CoordinateNavigation::Parse(fields[0].data());
        ImGui::BeginDisabled(!valid || !requestedID || *requestedID != tileID || tileID < 0 || tileID >= document->GetTileAttribsCount());
        if (button(e.buttpApply, "apply")) {
            if (*w != draft->getWidth() || *h != draft->getHeight()) resize(*w, *h, type);
            apply = true;
        }
        ImGui::EndDisabled();
        const float size = e.btpZoomTile ? 128.f : 64.f;
        const ImVec2 p{origin.x + (e.btpZoomTile ? 10.f : 42.f), origin.y + (e.btpZoomTile ? 65.f : 97.f)};
        draw->AddRect({p.x - 2, p.y - 2}, {p.x + size + 2, p.y + size + 2}, IM_COL32(19, 19, 19, 255));
        draw->AddRect({p.x - 1, p.y - 1}, {p.x + size + 1, p.y + size + 1}, IM_COL32(86, 86, 86, 255));
        auto* tile = set ? set->GetTile(tileID) : nullptr;
        if (tile) WapMapImGui::StretchSprite(draw, *hge, tile->GetImage(), p, {p.x + size, p.y + size}, IM_COL32_WHITE);
        const float sx = size / std::max(1, main->GetTileWidth()), sy = size / std::max(1, main->GetTileHeight());
        position(p.x - origin.x - area.x, p.y - origin.y - area.y);
        ImGui::InvisibleButton("##preview", {size, size});
        const auto cell = [&]() {
            const auto mouse = ImGui::GetIO().MousePos;
            return std::pair{std::clamp(static_cast<int>((mouse.x - p.x) / sx), 0, std::max(0, draft->getWidth() - 1)),
                             std::clamp(static_cast<int>((mouse.y - p.y) / sy), 0, std::max(0, draft->getHeight() - 1))};
        };
        if (valid && type != WWD::AttribType_Single && ImGui::IsItemActivated()) {
            const auto [x, y] = cell(); dragX = x; dragY = y; dragging = true;
        }
        if (dragging) {
            const auto [x, y] = cell();
            const WWD::Rect rect(std::min(x, dragX), std::min(y, dragY), std::max(x, dragX), std::max(y, dragY));
            if (type == WWD::AttribType_Double) {
                static_cast<WWD::DoubleTileAttrib*>(draft.get())->setMask(rect.x1, rect.y1, rect.x2, rect.y2);
                const int values[] = {rect.x1, rect.y1, rect.x2, rect.y2};
                for (int i = 0; i < 4; ++i) std::snprintf(fields[i + 3].data(), fields[i + 3].size(), "%d", values[i]);
            } else {
                draw->AddRect({p.x + rect.x1 * sx, p.y + rect.y1 * sy}, {p.x + (rect.x2 + 1) * sx, p.y + (rect.y2 + 1) * sy}, IM_COL32_WHITE);
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    static_cast<WWD::MaskTileAttrib*>(draft.get())->setArea(rect.x1, rect.y1, rect.x2, rect.y2, static_cast<WWD::TILE_ATTRIB>(inside));
            }
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) dragging = false;
        }
        draw->PushClipRect(p, {p.x + size, p.y + size}, true);
        const auto paint = [&](int x1, int y1, int x2, int y2, WWD::TILE_ATTRIB value) {
            if (value != WWD::Attrib_Clear && x2 > x1 && y2 > y1)
                draw->AddRectFilled({p.x + x1 * sx, p.y + y1 * sy}, {p.x + x2 * sx, p.y + y2 * sy}, color(value));
        };
        const int dw = draft->getWidth(), dh = draft->getHeight();
        if (type == WWD::AttribType_Single) paint(0, 0, dw, dh, static_cast<WWD::SingleTileAttrib*>(draft.get())->getAttrib());
        else if (type == WWD::AttribType_Double) {
            auto* a = static_cast<WWD::DoubleTileAttrib*>(draft.get()); const auto r = a->getMask();
            paint(r.x1, r.y1, r.x2 + 1, r.y2 + 1, a->getInsideAttrib());
            paint(0, 0, dw, r.y1, a->getOutsideAttrib()); paint(0, r.y2 + 1, dw, dh, a->getOutsideAttrib());
            paint(0, r.y1, r.x1, r.y2 + 1, a->getOutsideAttrib()); paint(r.x2 + 1, r.y1, dw, r.y2 + 1, a->getOutsideAttrib());
        } else if (validSize(dw, dh, type)) {
            const auto* data = static_cast<WWD::MaskTileAttrib*>(draft.get())->getData();
            // Merge horizontal runs to keep a painted mask from producing one draw per pixel.
            for (int y = 0; data && y < std::min(dh, main->GetTileHeight()); ++y)
                for (int x = 0; x < std::min(dw, main->GetTileWidth());) {
                    const int start = x; const auto value = data[y * dw + x++];
                    while (x < std::min(dw, main->GetTileWidth()) && data[y * dw + x] == value) ++x;
                    paint(start, y, x, y + 1, value);
                }
        }
        draw->PopClipRect();
        for (float y : {58.f, 202.f, 259.f, 320.f}) {
            if (y > 202.f && type != WWD::AttribType_Double) continue;
            draw->AddLine({origin.x, origin.y + y}, {origin.x + 300, origin.y + y}, IM_COL32(19, 19, 19, 255));
            draw->AddLine({origin.x, origin.y + y + 1}, {origin.x + 300, origin.y + y + 1}, IM_COL32(86, 86, 86, 255));
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) Close();
}

void winTileProperties::Think() {
    if (!apply || !open) return;
    apply = false;
    auto& e = *GV->editState;
    if (document != e.hParser || !document || e.itpSelectedTile != tileID) { Close(); return; }
    if (tileID < 0 || tileID >= document->GetTileAttribsCount() || !draft) return;
    document->SetTileAttribs(tileID, draft.get());
    e.SyncAttribMenuWithTile();
    e.vPort->MarkToRedraw();
}
#endif
