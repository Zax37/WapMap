#include "mediaBrowser.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../databanks/anims.h"
#include "../databanks/sounds.h"
#include "../databanks/imageSets.h"
#include "../cSoundPlayer.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include "assetBrowserUi.h"
#include "mediaPlayback.h"
#include "../../shared/commonFunc.h"
#include <cctype>
#include <cstdio>

namespace {
const char* text(const wchar_t* key) { return GETL2SV("MediaBrowser", key); }
void at(float x, float y) { ImGui::SetCursorPos({x, y}); }
void label(const char* value, float x, float y) {
    const auto p = ImGui::GetWindowPos();
    WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, {p.x + x, p.y + y}, value);
}
bool iconButton(const char* id, hgeSprite* icon, float x, float y) {
    at(x, y); ImGui::PushID(id);
    const bool hit = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "", {32, 32});
    const auto p = ImGui::GetItemRectMin();
    if (icon) WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, icon, {p.x + (32 - icon->GetWidth()) / 2, p.y + (32 - icon->GetHeight()) / 2},
        IM_COL32(255, 255, 255, static_cast<int>(255 * ImGui::GetStyle().Alpha)));
    ImGui::PopID(); return hit;
}
std::string upper(std::string value) {
    for (auto& ch : value) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    return value;
}
}

winMediaBrowser::winMediaBrowser(bool soundMode)
    : cWindow(soundMode ? GETL2S("WinDatabase", "Tab_Sounds") : GETL2S("WinDatabase", "Tab_Anims"), 950, 600), sounds(soundMode) {}
winMediaBrowser::~winMediaBrowser() { StopSound(); }

cAsset* winMediaBrowser::Resolve() {
    auto& e = *GV->editState;
    if (!document || document != e.hParser || selectedName.empty()) return nullptr;
    if ((sounds && !e.hSndBank) || (!sounds && !e.hAniBank)) return nullptr;
    return sounds ? static_cast<cAsset*>(e.hSndBank->GetAssetByID(selectedName.c_str())) : static_cast<cAsset*>(e.hAniBank->GetAssetByID(selectedName.c_str()));
}
ANI::Animation* winMediaBrowser::Animation() {
    auto* asset = sounds ? nullptr : static_cast<cAniBankAsset*>(Resolve());
    if (!asset || !asset->IsLoaded()) return nullptr;
    auto* animation = asset->GetAni();
    return animation && animation->Valid() ? animation : nullptr;
}
void winMediaBrowser::StopSound() {
    if (channel) hge->Channel_Stop(channel);
    channel = nullptr; paused = false; position = 0;
}
void winMediaBrowser::PlaySound() {
    auto* asset = sounds ? static_cast<cSndBankAsset*>(Resolve()) : nullptr;
    if (!asset || !asset->IsLoaded() || !asset->GetSound()) return;
    if (channel && paused) hge->Channel_Resume(channel);
    else {
        if (channel) hge->Channel_Stop(channel);
        channel = hge->Effect_PlayEx(asset->GetSound(), volume);
        if (channel) {
            duration = hge->Channel_GetLength(channel);
            if (position > 0 && position < duration) hge->Channel_SetPos(channel, position);
            else position = 0;
        }
    }
    paused = false;
}
void winMediaBrowser::Select(cAsset* asset, bool autoPlay) {
    StopSound(); elapsed = 0; frame = 0; playing = false; duration = 0;
    selectedName = asset ? asset->GetName() : "";
    selectedHash = asset ? asset->GetHash() : ""; selectedInstance = asset;
    metadata.clear();
    if (!asset) return;
    metadata.emplace_back(text(L"Name"), asset->GetName());
    metadata.emplace_back(GETL2S("Win_ImageSetBrowser", "FrameFilePath"), asset->GetFile().strPath);
    metadata.emplace_back(GETL2S("Win_ImageSetBrowser", "FrameChecksum"), asset->GetHash());
    metadata.emplace_back(text(L"FileSize"), std::to_string(asset->GetFileSize()) + " B");
    auto* feed = asset->GetFile().hFeed;
    if (feed) {
        char* date = SHR::FormatTimeFromUnix(feed->GetFileModTime(asset->GetFile().strPath.c_str()));
        metadata.emplace_back(GETL2S("Win_ImageSetBrowser", "FrameModDate"), date); delete[] date;
        const char* origin = feed == GV->editState->hDataCtrl->GetFeed(DB_FEED_REZ) ? GETL2S("Win_ImageSetBrowser", "OriginREZ") :
            feed == GV->editState->hDataCtrl->GetFeed(DB_FEED_CUSTOM) ? GETL2S("Win_ImageSetBrowser", "OriginCustom") : GETL2S("Win_ImageSetBrowser", "OriginClaw");
        metadata.emplace_back(GETL2S("Win_ImageSetBrowser", "FrameOrigin"), origin);
    }
    if (!sounds) {
        auto* ani = Animation();
        if (ani) metadata.emplace_back(text(L"FrameCount"), std::to_string(ani->GetFramesCount()));
        const auto set = MediaPlayback::ImageSet(asset->GetName(), ani && ani->GetImageset() ? ani->GetImageset() : "",
            document->GetGame() == WWD::Game_Gruntz, [](const std::string& name) { return GV->editState->SprBank->GetAssetByID(name.c_str()) != nullptr; });
        const auto utf8 = LegacyText::ToUtf8(set, LegacyText::CodePage(GV->Lang->GetCode()));
        std::snprintf(imageSet.data(), imageSet.size(), "%s", utf8.c_str());
    } else {
        effect = static_cast<cSndBankAsset*>(asset)->GetSound();
        if (autoPlay) PlaySound();
    }
}
void winMediaBrowser::Open() {
    document = GV->editState->hParser;
    if (!document) return;
    GV->editState->winDB->setVisible(false);
    GV->editState->spdbSounds->SetAsset(nullptr);
    open = positionOnOpen = true;
    if (auto* asset = Resolve()) Select(asset, false);
    else if (!sounds && GV->editState->hAniBank && GV->editState->hAniBank->getNumberOfElements() > 0)
        Select(GV->editState->hAniBank->GetAssetByIterator(0), false);
    else Select(nullptr, false);
}
void winMediaBrowser::Close() { open = playing = false; StopSound(); cWindow::Close(); }
void winMediaBrowser::OnDocumentChange() {
    Close(); document = nullptr; selectedName.clear(); selectedInstance = nullptr; metadata.clear();
    filter.fill(0); imageSet.fill(0);
}
void winMediaBrowser::Think() {
    if (!open) return;
    if (document != GV->editState->hParser) { OnDocumentChange(); return; }
    auto* asset = Resolve();
    if (!asset) { if (!selectedName.empty()) Select(nullptr, false); return; }
    if (asset != selectedInstance || asset->GetHash() != selectedHash ||
        (sounds && static_cast<cSndBankAsset*>(asset)->GetSound() != effect)) Select(asset, false);
    if (!GV->editState->_isOnTop()) return;
    if (sounds) {
        if (channel) {
            position = hge->Channel_GetPos(channel);
            if (!paused && !hge->Channel_IsPlaying(channel)) {
                hge->Channel_Stop(channel); channel = nullptr; position = duration;
            }
        }
    } else if (auto* ani = Animation()) {
        MediaPlayback::Advance(ani->GetFramesCount(), frame, elapsed, playing, loop,
            hge->Timer_GetDelta() * (speed + 1) * 250.0, [&](int i) { return static_cast<int>(ani->GetFrame(i)->GetDuration()); });
    }
}

void winMediaBrowser::DrawImGui() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !document) { OnDocumentChange(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus(); positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({950, 600}, ImGuiCond_Always);
    const char* caption = sounds ? GETL2S("WinDatabase", "Tab_Sounds") : GETL2S("WinDatabase", "Tab_Anims");
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(caption, cp) + (sounds ? "###WapMapSounds" : "###WapMapAnimations");
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, caption);
        label(text(sounds ? L"LoadedSounds" : L"LoadedAnimations"), 8, 32);
        at(8, 52); ImGui::SetNextItemWidth(300);
        const auto hint = LegacyText::ToUtf8(text(L"Filter"), cp);
        ImGui::InputTextWithHint("##filter", hint.c_str(), filter.data(), filter.size());
        const auto needle = upper(LegacyText::FromUtf8(filter.data(), cp, 255).value_or(""));
        std::vector<cAsset*> assets;
        const int count = sounds ? e.hSndBank->getNumberOfElements() : e.hAniBank->getNumberOfElements();
        for (int i = 0; i < count; ++i) {
            cAsset* asset = sounds ? static_cast<cAsset*>(e.hSndBank->GetAssetByIterator(i)) : static_cast<cAsset*>(e.hAniBank->GetAssetByIterator(i));
            if (needle.empty() || upper(asset->GetName()).find(needle) != std::string::npos) assets.push_back(asset);
        }
        at(8, 80); ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0}); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
        if (ImGui::BeginChild("assets", {300, 510})) {
            const float rowHeight = sounds ? 68.f : 90.f;
            ImGuiListClipper clipper; clipper.Begin(static_cast<int>(assets.size()), rowHeight);
            while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto* asset = assets[i]; ImGui::PushID(asset); const auto p = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("##asset", {ImGui::GetContentRegionAvail().x, rowHeight}, ImGuiButtonFlags_EnableNav)) Select(asset, sounds);
                auto* draw = ImGui::GetWindowDrawList();
                if (selectedName == asset->GetName() || ImGui::IsItemHovered() || ImGui::IsItemFocused())
                    draw->AddRectFilled(p, {p.x + ImGui::GetContentRegionAvail().x, p.y + rowHeight}, selectedName == asset->GetName() ? IM_COL32(23, 150, 255, 51) : IM_COL32(161, 161, 161, 51));
                hgeSprite* thumbnail = sounds ? GV->sprIcons[Icon_Sound] : GV->sprSmiley;
                std::string detail = std::to_string(asset->GetFileSize()) + " B";
                if (!sounds && asset->IsLoaded()) {
                    auto* ani = static_cast<cAniBankAsset*>(asset)->GetAni();
                    if (ani && ani->Valid()) {
                        detail = std::string(text(L"FrameCount")) + " " + std::to_string(ani->GetFramesCount());
                        const auto name = MediaPlayback::ImageSet(asset->GetName(), ani->GetImageset() ? ani->GetImageset() : "",
                            document->GetGame() == WWD::Game_Gruntz, [&](const std::string& id) { return e.SprBank->GetAssetByID(id.c_str()) != nullptr; });
                        auto* set = e.SprBank->GetAssetByID(name.c_str());
                        auto* image = set && ani->GetFramesCount() ? set->GetIMGByID(ani->GetFrame(0)->GetImageID()) : nullptr;
                        if (image && image->IsLoaded() && image->GetSprite()) thumbnail = image->GetSprite();
                    }
                }
                if (sounds) WapMapImGui::Sprite(draw, *hge, thumbnail, {p.x + 10, p.y + 18});
                else AssetBrowserUi::Preview(*hge, thumbnail, {p.x + 10, p.y + 10}, 70, false, false);
                const float textX = sounds ? 55.f : 100.f;
                draw->PushClipRect({p.x + textX, p.y}, {p.x + ImGui::GetContentRegionAvail().x, p.y + rowHeight}, true);
                WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, {p.x + textX, p.y + rowHeight / 2 - 14}, asset->GetName().c_str());
                WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, {p.x + textX, p.y + rowHeight / 2 + 3}, detail.c_str());
                draw->PopClipRect(); ImGui::PopID();
                if (ImGui::IsItemHovered()) {
                    const auto fullName = LegacyText::ToUtf8(asset->GetName(), cp);
                    ImGui::SetTooltip("%s", fullName.c_str());
                }
            }
        }
        ImGui::EndChild(); ImGui::PopStyleVar(2); ImGui::PopStyleColor();
        if (auto* selected = Resolve()) {
            const auto origin = ImGui::GetWindowPos();
            auto* draw = ImGui::GetWindowDrawList();
            draw->PushClipRect({origin.x + 330, origin.y + 26}, {origin.x + 940, origin.y + 172}, true);
            float y = 32;
            for (const auto& row : metadata) { label(row.first.c_str(), 330, y); label(row.second.c_str(), 460, y); y += 20; }
            draw->PopClipRect();
            if (sounds) DrawSound(); else DrawAnimation();
        } else label(text(L"NoSelection"), 335, 40);
        auto* draw = ImGui::GetWindowDrawList(); const auto p = ImGui::GetWindowPos();
        draw->AddLine({p.x + 320, p.y + 25}, {p.x + 320, p.y + 600}, IM_COL32(86, 86, 86, 255));
        draw->AddLine({p.x + 320, p.y + 172}, {p.x + 950, p.y + 172}, IM_COL32(86, 86, 86, 255));
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End(); if (!visible) Close();
}

void winMediaBrowser::DrawSound() {
    auto* asset = static_cast<cSndBankAsset*>(Resolve());
    const bool available = asset && asset->IsLoaded() && asset->GetSound();
    ImGui::BeginDisabled(!available);
    const bool active = channel && !paused && hge->Channel_IsPlaying(channel);
    if (iconButton("play", GV->sprIcons[active ? Icon_Pause : Icon_Play], 350, 310)) {
        if (active) { hge->Channel_Pause(channel); paused = true; }
        else PlaySound();
    }
    if (iconButton("stop", GV->sprIcons[Icon_Stop], 390, 310)) StopSound();
    char clock[64]; std::snprintf(clock, sizeof(clock), "%02d:%02d / %02d:%02d", static_cast<int>(position) / 60,
        static_cast<int>(position) % 60, static_cast<int>(duration) / 60, static_cast<int>(duration) % 60);
    label(clock, 440, 320);
    label(text(L"Position"), 350, 250); at(350, 275); ImGui::SetNextItemWidth(560);
    ImGui::BeginDisabled(duration <= 0);
    float cursor = position;
    if (ImGui::SliderFloat("##position", &cursor, 0, std::max(0.001f, duration), "%.2f s")) {
        position = cursor; if (channel) hge->Channel_SetPos(channel, cursor);
    }
    ImGui::EndDisabled();
    label(text(L"Volume"), 740, 320); at(810, 315); ImGui::SetNextItemWidth(100);
    if (ImGui::SliderInt("##volume", &volume, 0, 100, "%d%%") && channel) hge->Channel_SetVolume(channel, volume);
    ImGui::EndDisabled();
    if (!available) label(text(L"Unavailable"), 350, 190);
}

void winMediaBrowser::DrawAnimation() {
    auto* ani = Animation();
    if (!ani || ani->GetFramesCount() <= 0) { label(text(L"Unavailable"), 335, 190); return; }
    frame = std::clamp(frame, 0, ani->GetFramesCount() - 1);
    bool* values[] = {&border, &playing, &loop, &mirror};
    const char* captions[] = {GETL(Lang_Border), GETL(Lang_Animate), GETL2("WinDatabase", Lang_Db_Loop), GETL(Lang_Mirror)};
    float x = 335;
    for (int i = 0; i < 4; ++i) {
        ImGui::PushID(i); at(x, 184);
        WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, captions[i], *values[i]);
        x += 28 + GV->fntMyriad16->GetStringWidth(captions[i]); ImGui::PopID();
    }
    const char* speedCaption = GETL2("WinDatabase", Lang_Db_AnimSpeed);
    const float speedX = 343.f + GV->fntMyriad16->GetStringWidth(speedCaption);
    label(speedCaption, 335, 214); at(speedX, 208); ImGui::SetNextItemWidth(80);
    const auto speedText = std::to_string((speed + 1) * 25) + "%";
    if (ImGui::BeginCombo("##speed", speedText.c_str())) {
        for (int i = 0; i < 8; ++i) {
            const auto caption = std::to_string((i + 1) * 25) + "%";
            if (ImGui::Selectable(caption.c_str(), speed == i)) speed = i;
        }
        ImGui::EndCombo();
    }
    const char* imagesCaption = GETL2("WinDatabase", Lang_Db_AniImageset);
    const float imagesX = std::max(530.f, speedX + 100.f);
    const float imagesFieldX = imagesX + GV->fntMyriad16->GetStringWidth(imagesCaption) + 8.f;
    label(imagesCaption, imagesX, 214); at(imagesFieldX, 208); ImGui::SetNextItemWidth(std::max(80.f, 935.f - imagesFieldX));
    ImGui::InputText("##imageset", imageSet.data(), imageSet.size());
    const auto setName = LegacyText::FromUtf8(imageSet.data(), LegacyText::CodePage(GV->Lang->GetCode()), 255);
    auto* set = setName ? GV->editState->SprBank->GetAssetByID(setName->c_str()) : nullptr;
    const auto sprite = [&](int i) -> hgeSprite* {
        auto* image = set ? set->GetIMGByID(ani->GetFrame(i)->GetImageID()) : nullptr;
        return image && image->IsLoaded() ? image->GetSprite() : nullptr;
    };
    auto* draw = ImGui::GetWindowDrawList(); const auto origin = ImGui::GetWindowPos();
    const ImVec2 start{origin.x + 330, origin.y + 240}, end{origin.x + 940, origin.y + 405};
    draw->PushClipRect(start, end, true);
    const float checkW = std::max(1.f, GV->sprCheckboard->GetWidth()), checkH = std::max(1.f, GV->sprCheckboard->GetHeight());
    for (float y = start.y; y < end.y; y += checkH)
        for (float x = start.x; x < end.x; x += checkW) WapMapImGui::Sprite(draw, *hge, GV->sprCheckboard, {x, y});
    if (auto* spr = sprite(frame)) {
        // Map rendering may have flipped this shared sprite's hotspot. Use
        // immutable image metadata so the browser has its own stable origin.
        const auto info = set->GetIMGByID(ani->GetFrame(frame)->GetImageID())->GetImageInfo();
        const float hx = static_cast<float>(info.iWidth / 2 - info.iOffsetX);
        const float hy = static_cast<float>(info.iHeight / 2 - info.iOffsetY);
        float tx, ty, tw, th; spr->GetTextureRect(&tx, &ty, &tw, &th);
        const float w = spr->GetWidth(), h = spr->GetHeight();
        const float scale = std::min(1.f, std::min(594.f / std::max(1.f, 2 * std::max(std::abs(hx), std::abs(w - hx))),
            149.f / std::max(1.f, 2 * std::max(std::abs(hy), std::abs(h - hy)))));
        const ImVec2 anchor{(start.x + end.x) / 2, (start.y + end.y) / 2};
        const ImVec2 p{anchor.x - (mirror ? w - hx : hx) * scale, anchor.y - hy * scale};
        const auto texture = spr->GetTexture(); const auto native = hge->Texture_GetNativeHandle(texture);
        const float textureW = static_cast<float>(hge->Texture_GetWidth(texture)), textureH = static_cast<float>(hge->Texture_GetHeight(texture));
        if (native && textureW > 0 && textureH > 0) draw->AddImage(static_cast<ImTextureID>(native), p, {p.x + w * scale, p.y + h * scale},
            {(tx + (mirror ? tw : 0)) / textureW, ty / textureH}, {(tx + (mirror ? 0 : tw)) / textureW, (ty + th) / textureH});
        if (border) {
            draw->AddRect(p, {p.x + w * scale, p.y + h * scale}, IM_COL32(255, 0, 0, 190));
            draw->AddLine({anchor.x - 3, anchor.y}, {anchor.x + 3, anchor.y}, IM_COL32(255, 0, 0, 255));
            draw->AddLine({anchor.x, anchor.y - 3}, {anchor.x, anchor.y + 3}, IM_COL32(255, 0, 0, 255));
        }
    } else label(text(L"MissingImage"), 350, 310);
    draw->PopClipRect();
    char info[128]; std::snprintf(info, sizeof(info), "%s %d / %d   |   ID: %d   |   %d ms", GETL2("WinDatabase", Lang_Db_Frame), frame + 1,
        ani->GetFramesCount(), ani->GetFrame(frame)->GetImageID(), ani->GetFrame(frame)->GetDuration());
    label(info, 335, 415);
    at(330, 440); ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0}); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
    if (ImGui::BeginChild("frames", {610, 150})) {
        const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / 100));
        const auto startPos = ImGui::GetCursorPos();
        ImGuiListClipper clipper; clipper.Begin((ani->GetFramesCount() + columns - 1) / columns, 120);
        while (clipper.Step()) for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
            for (int col = 0; col < columns; ++col) {
                const int i = row * columns + col; if (i >= ani->GetFramesCount()) break;
                ImGui::SetCursorPos({startPos.x + col * 100.f, startPos.y + row * 120.f});
                ImGui::PushID(i); const auto p = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("##frame", {100, 120}, ImGuiButtonFlags_EnableNav)) { frame = i; elapsed = 0; playing = false; }
                AssetBrowserUi::Preview(*hge, sprite(i) ? sprite(i) : GV->sprSmiley, {p.x + 5, p.y + 5}, 90, false, true, frame == i, ImGui::IsItemHovered());
                const auto caption = std::to_string(i + 1);
                WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, {p.x + 45, p.y + 102}, caption.c_str());
                if (ImGui::IsItemHovered()) {
                    const char* key = ani->GetFrame(i)->GetKeyFrameName();
                    const auto utf8 = LegacyText::ToUtf8(key ? key : "", LegacyText::CodePage(GV->Lang->GetCode()));
                    ImGui::SetTooltip("ID: %d\n%d ms%s%s", ani->GetFrame(i)->GetImageID(), ani->GetFrame(i)->GetDuration(), key ? "\n" : "", utf8.c_str());
                }
                ImGui::PopID();
            }
            ImGui::SetCursorPos({startPos.x, startPos.y + (row + 1) * 120.f});
        }
    }
    ImGui::EndChild(); ImGui::PopStyleVar(2);
}
#endif
