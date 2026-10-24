#include "about.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../version.h"

#ifdef WAPMAP_ENABLE_IMGUI
#include <imgui.h>
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif

extern HGE *hge;

static const int usedLibsCreditsBlockWidth = 507;
static const int usedLibsCreditsBlockLineSpacing = 6;

static const char* const usedLibsCreditLines[] {
        "HGE 1.81 - Copyright (c) 2003-2008 Relish Games.",
        "Guichan 0.8.3 - Copyright (c) 2004-2007 Olof Naessen and Per Larsson. All rights reserved.",
        "libcurl 7.21.6 - Copyright (c) 1996 - 2012, Daniel Stenberg, <daniel@haxx.se>.",
        "hashlib++ 0.3.2 - Copyright (c) 2007-2011 Benjamin Grudelbach.",
        "Lua 5.1.4 - Copyright (c) 1994-2012 Lua.org, PUC-Rio.",
        "SFML 3.0.2 - Copyright (c) 2007-2025 Laurent Gomila.",
        "zlib 1.2.5 - Copyright (c) 1995-2012 Jean-loup Gailly and Mark Adler.",
        "ziplib - Copyright (c) 2013 Petr Benes.",
        "JSON for Modern C++ - Copyright (c) 2013-2019 Niels Lohmann <http://nlohmann.me>."
};

winAbout::winAbout() : cWindow(GETL(Lang_About)) {
    int h = GV->sprLogoBig->GetHeight() + 105 + 22 * 4;
    for (auto line : usedLibsCreditLines) {
        h += GV->fntMyriad16->GetStringBlockHeight(usedLibsCreditsBlockWidth, line) + usedLibsCreditsBlockLineSpacing;
    }
    myWin.setDimension(gcn::Rectangle(0, 0, usedLibsCreditsBlockWidth + 2 * 32, h));
    vp = new WIDG::Viewport(this, 0);
    myWin.add(vp, 0, 0);
}

#ifdef WAPMAP_ENABLE_IMGUI
void winAbout::Open() {
    myWin.setVisible(false);
    imguiOpen = true;
    imguiPositionOnOpen = true;
}

void winAbout::Close() {
    imguiOpen = false;
    cWindow::Close();
}

void winAbout::DrawImGui() {
    if (!imguiOpen) {
        return;
    }

    if (imguiPositionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        imguiPositionOnOpen = false;
    }
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(myWin.getWidth()),
                                    static_cast<float>(myWin.getHeight())),
                             ImGuiCond_Always);
    ImGui::SetNextWindowCollapsed(false);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.f, 0.f));
    // Keep native title-bar drag and close hit-testing. The floating title
    // is painted with the original atlas below; docked tabs stay native.
    const auto label = LegacyText::ToUtf8(GETL(Lang_About), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapAbout";
    const bool visible = ImGui::Begin(label.c_str(), &imguiOpen,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    WapMapImGui::KeepWindowOnScreen();
    if (visible) {
        auto* draw = ImGui::GetWindowDrawList();
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_About));

        const auto* font = GV->fntMyriad16;
        const auto origin = ImGui::GetCursorScreenPos();
        WapMapImGui::Sprite(draw, *hge, GV->sprLogoBig, ImVec2(origin.x - 4.f, origin.y + 13.f));
        WapMapImGui::BitmapText(draw, *hge, *font, ImVec2(origin.x + 92.f, origin.y + 65.f), WA_VERSTRING);
        const float width = std::max(1.f, ImGui::GetContentRegionAvail().x);
        float y = origin.y + GV->sprLogoBig->GetHeight() + 32.f;
        const auto separator = [&]() {
            draw->AddLine(ImVec2(origin.x, y), ImVec2(origin.x + width - 2.f, y), IM_COL32(86, 86, 86, 127));
        };
        separator();
        y += 16.f;
        const std::string authors[] = {
            std::string(GETL2S("WinAuthors", "Func_1")) + " kijanek6 & Zax37.",
            std::string(GETL2S("WinAuthors", "Func_2")) + " Tomalla.",
            std::string(GETL2S("WinAuthors", "Func_3")) + "."
        };
        for (int i = 0; i < 3; ++i) {
            WapMapImGui::BitmapText(draw, *hge, *font, ImVec2(origin.x, y), authors[i].c_str());
            y += i == 2 ? 32.f : 22.f;
        }
        separator();
        ImGui::SetCursorScreenPos(ImVec2(origin.x, y + 16.f));
        for (const auto* line : usedLibsCreditLines) {
            WapMapImGui::WrappedBitmapText(*hge, *font, line, width);
        }
    }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape))
        imguiOpen = false;
    ImGui::End();
    ImGui::PopStyleVar();

    if (!imguiOpen) {
        cWindow::Close();
    }
}
#endif

void winAbout::Think() {

}

void winAbout::Draw(int piCode) {
    unsigned char alpha = myWin.getAlpha();
    int dx, dy, logoW = GV->sprLogoBig->GetWidth();
    myWin.getAbsolutePosition(dx, dy);

    dx += 16;

    GV->RenderLogoWithVersion(dx + logoW / 2 + 12, dy + 77, alpha);

    dx += 18;
    dy += GV->sprLogoBig->GetHeight() + 64;

    hge->Gfx_RenderLine(dx, dy, dx + usedLibsCreditsBlockWidth - 2, dy, SETA(GV->colLineBright, alpha));

    dy += 16;

    GV->fntMyriad16->printf(dx, dy, HGETEXT_LEFT, "%s kijanek6 & Zax37.",
                            0, GETL2S("WinAuthors", "Func_1"));
    dy += 22;
    GV->fntMyriad16->printf(dx, dy, HGETEXT_LEFT, "%s Tomalla.",
                            0, GETL2S("WinAuthors", "Func_2"));
    dy += 22;
    GV->fntMyriad16->printf(dx, dy, HGETEXT_LEFT, "%s.",
                            0, GETL2S("WinAuthors", "Func_3"));
    dy += 32;

    hge->Gfx_RenderLine(dx, dy, dx + usedLibsCreditsBlockWidth - 2, dy, SETA(GV->colLineBright, alpha));

    dy += 16;

    for (auto line : usedLibsCreditLines) {
        GV->fntMyriad16->printfb(dx, dy, usedLibsCreditsBlockWidth, 200, 0, 0, line);
        dy += GV->fntMyriad16->GetStringBlockHeight(usedLibsCreditsBlockWidth, line) + usedLibsCreditsBlockLineSpacing;
    }
}
