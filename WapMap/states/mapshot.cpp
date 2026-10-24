#include "mapshot.h"
#include "../globals.h"
#include "../version.h"
#include <cmath>
#include "../langID.h"
#include "editing_ww.h"
#include "../returncodes.h"
#include "dialog.h"
#include "../../shared/commonFunc.h"
#include "../windows/mapShotValidation.h"
#include "../windows/mapShotImageWriter.h"
#include <stdexcept>
#include <limits>
#include <cstdio>
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif

extern HGE *hge;

State::MapShot::MapShot(State::EditingWW *mainSt, int planeIterator, float scale, const char *dest, int saveoption,
                        DWORD bg, bool drawobj) {
    fScale = scale / 100.0f;
    iPLid = planeIterator;
    stMain = mainSt;
    pl = stMain->hParser->GetPlane(iPLid);
    targTemp = 0;
    szOutput = new char[strlen(dest) + 1];
    strcpy(szOutput, dest);
    iOutputOption = saveoption;
    dwBg = bg;
    bDrawObjects = drawobj;
    targPreview = 0;
    sprPreview = NULL;

    iPreviewW = pl->GetPlaneWidthPx() * fScale;
    if (iPreviewW > 395) {
        iPreviewW = 395;
        fPreviewScale = float(iPreviewW) / (float(pl->GetPlaneWidthPx()) * fScale);
        iPreviewH = (pl->GetPlaneHeightPx() * fScale) * fPreviewScale;
    } else {
        fPreviewScale = fScale;
        iPreviewH = pl->GetPlaneHeightPx() * fScale;
    }
}

bool State::MapShot::Opaque() {
    return 1;
}

void State::MapShot::Init() {
    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);

    winLoad = new SHR::Win(&GV->gcnParts, GETL(Lang_MakingMapshot));
    winLoad->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200,
                                         hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 100, 400, 200));
    winLoad->setMovable(0);
    gui->setTop(winLoad);

    labWhole = new SHR::Lab(GETL(Lang_Progress));
    labWhole->adjustSize();
    winLoad->add(labWhole, 5, 15);

    barWhole = new SHR::ProgressBar(&GV->gcnParts);
    barWhole->setDimension(gcn::Rectangle(0, 0, 380, 31));
    barWhole->setEnd(1000);
    winLoad->add(barWhole, 5, 40);

    labAction = new SHR::Lab(GETL(Lang_ActualOperation));
    labAction->adjustSize();
    winLoad->add(labAction, 5, 80);

    barAction = new SHR::ProgressBar(&GV->gcnParts);
    barAction->setDimension(gcn::Rectangle(0, 0, 380, 31));
    barAction->setEnd(100);
    winLoad->add(barAction, 5, 105);

    labDesc = new SHR::Lab("");
    labDesc->adjustSize();
    winLoad->add(labDesc, 5, 140);

    // Work starts in Think, after the state manager has completed Push/Init.
}

void State::MapShot::Destroy() {
    if (targTemp != 0)
        hge->Target_Free(targTemp);
    delete[] szOutput;

    if (sprPreview != NULL)
        delete sprPreview;
    if (targPreview != 0)
        hge->Target_Free(targPreview);
    delete labDesc;
    delete labAction;
    delete labWhole;
    delete barAction;
    delete barWhole;
    delete winLoad;
    delete gui;
}

bool State::MapShot::Think() {
    if (cancelRequested) { _popMe({ReturnCodeType::MapShot, 0}); return false; }
    if (firstFrame) { firstFrame = false; return false; }
    try {
        if (phase == Phase::Allocate) {
            const auto size = MapShotValidation::Dimensions(pl->GetPlaneWidthPx(), pl->GetPlaneHeightPx(), fScale * 100.0);
            if (!size) throw std::runtime_error("Invalid mapshot dimensions");
            totalWidth = size->first; totalHeight = size->second;
            const auto plan = MapShotValidation::Plan(totalWidth, totalHeight);
            if (!plan) throw std::runtime_error("Invalid mapshot chunk layout");
            chunkWidth = plan->chunkWidth; chunkHeight = plan->chunkHeight;
            columns = plan->columns; rows = plan->rows;
            gdDest = sf::Image({static_cast<unsigned>(totalWidth), static_cast<unsigned>(totalHeight)}, sf::Color::Transparent);
            targTemp = hge->Target_Create(chunkWidth, chunkHeight, false);
            if (!targTemp) throw std::runtime_error("Cannot allocate mapshot target");
            fPreviewScale = std::min({1.f, 380.f / totalWidth, 220.f / totalHeight});
            iPreviewW = std::max(1, static_cast<int>(totalWidth * fPreviewScale));
            iPreviewH = std::max(1, static_cast<int>(totalHeight * fPreviewScale));
            targPreview = hge->Target_Create(iPreviewW, iPreviewH, false);
            if (targPreview) {
                sprPreview = new hgeSprite(hge->Target_GetTexture(targPreview), 0, 0, iPreviewW, iPreviewH);
                if (hge->Gfx_BeginScene(targPreview)) { hge->Gfx_Clear(0xFF3e3e3e); hge->Gfx_EndScene(); }
            }
            labWhole->setCaption(GETL2("Mapshot", Lang_Ms_Drawing));
            barWhole->setEnd(3); barWhole->setProgress(1);
            barAction->setEnd(columns * rows);
            phase = Phase::Draw;
        } else if (phase == Phase::Draw) {
            RenderChunk();
            ++nextChunk;
            barAction->setProgress(nextChunk);
            if (nextChunk == columns * rows) {
                phase = Phase::Save;
                labWhole->setCaption(GETL2("Mapshot", Lang_Ms_Saving));
                barWhole->setProgress(2);
            }
        } else if (phase == Phase::Save) {
            if (!MapShotImageWriter::Save(gdDest, szOutput, iOutputOption)) throw std::runtime_error("Cannot save mapshot");
            phase = Phase::Finished;
            _popMe({ReturnCodeType::MapShot, 1});
        }
    } catch (const std::exception& error) {
        GV->Console->Printf("~r~Mapshot: %s", error.what());
        phase = Phase::Finished;
        GV->StateMgr->Push(new State::Dialog(GETL2("Mapshot", Lang_Ms_Error), GETL2S("MapshotNative", "ExportFailed"), ST_DIALOG_ICON_ERROR));
    }
    return false;
}

void State::MapShot::RenderChunk() {
    const int column = nextChunk % columns, row = nextChunk / columns;
    char caption[160];
    std::snprintf(caption, sizeof(caption), "%s %d/%d", GETL2("Mapshot", Lang_Ms_Chunk), nextChunk + 1, columns * rows);
    labAction->setCaption(caption);
    struct RestoreEditor {
        EditingWW& editor;
        float x, y, zoom;
        DWORD background;
        int vx, vy, width, height;
        WWD::Plane* plane;
        int moveX, moveY;
        bool alpha;
        bool grid, boundary, attributes;
        PlaneData* planeData;
        cLayerRenderBuffer* renderBuffer;
        WWD::Plane* mainPlane;
        std::vector<std::pair<bool, bool>> planes;
        RestoreEditor(EditingWW& e, WWD::Plane* selected, int selectedIndex) : editor(e), x(e.fCamX), y(e.fCamY), zoom(e.fZoom), background(e.dwBG),
            vx(e.vPort->GetX()), vy(e.vPort->GetY()), width(e.vPort->GetWidth()), height(e.vPort->GetHeight()),
            plane(selected), moveX(selected->GetMoveModX()), moveY(selected->GetMoveModY()), alpha(GV->bAlphaHigherPlanes),
            grid(e.hPlaneData[selectedIndex]->bDrawGrid), boundary(e.hPlaneData[selectedIndex]->bDrawBoundary), attributes(e.bDrawTileProperties),
            planeData(e.hPlaneData[selectedIndex]), renderBuffer(planeData->hRB), mainPlane(e.plMain) {
            for (int i = 0; i < e.hParser->GetPlanesCount(); ++i) planes.emplace_back(e.hPlaneData[i]->bDraw, e.hPlaneData[i]->bDrawObjects);
        }
        ~RestoreEditor() {
            hge->Gfx_EndScene();
            editor.fCamX = x; editor.fCamY = y; editor.fZoom = zoom; editor.dwBG = background;
            plane->SetMoveModX(moveX); plane->SetMoveModY(moveY);
            GV->bAlphaHigherPlanes = alpha;
            planeData->bDrawGrid = grid; planeData->bDrawBoundary = boundary; planeData->hRB = renderBuffer;
            editor.bDrawTileProperties = attributes; editor.plMain = mainPlane;
            editor.vPort->SetRenderBounds(vx, vy, width, height);
            for (size_t i = 0; i < planes.size(); ++i) {
                editor.hPlaneData[i]->bDraw = planes[i].first;
                editor.hPlaneData[i]->bDrawObjects = planes[i].second;
            }
            editor.vPort->MarkToRedraw();
        }
    } restore(*stMain, pl, iPLid);
    // Export plane coordinates, independently of its editor parallax/centering
    // and the display-only fade used for planes above the active one.
    pl->SetMoveModX(100); pl->SetMoveModY(100);
    GV->bAlphaHigherPlanes = false;
    stMain->hPlaneData[iPLid]->bDrawGrid = false;
    stMain->hPlaneData[iPLid]->bDrawBoundary = false;
    stMain->hPlaneData[iPLid]->hRB = nullptr;
    stMain->bDrawTileProperties = false;
    stMain->plMain = pl;
    for (int i = 0; i < stMain->hParser->GetPlanesCount(); ++i) {
        stMain->hPlaneData[i]->bDraw = i == iPLid;
        stMain->hPlaneData[i]->bDrawObjects = i == iPLid && !stMain->hPlaneData[i]->ObjectData.bEmpty && bDrawObjects;
    }
    stMain->vPort->SetRenderBounds(0, 0, chunkWidth, chunkHeight);
    stMain->fZoom = fScale; stMain->dwBG = dwBg;
    stMain->fCamX = column * chunkWidth / fScale; stMain->fCamY = row * chunkHeight / fScale;
    if (!hge->Gfx_BeginScene(targTemp)) throw std::runtime_error("Cannot begin mapshot scene");
    hge->Gfx_Clear(dwBg);
    stMain->RenderToViewportBuffer(true);
    if (nextChunk == 0) {
        char mark[96]; std::snprintf(mark, sizeof(mark), "Generated by WapMap v%s", WA_VERPURE);
        auto* font = GV->fntMyriad20;
        const DWORD color = font->GetColor();
        font->SetColor(0xFF000000); font->Render(7, 7, HGETEXT_LEFT, mark);
        font->SetColor(0xFFFFFFFF); font->Render(5, 5, HGETEXT_LEFT, mark);
        font->SetColor(color);
    }
    hge->Gfx_EndScene();
    const DWORD* pixels = hge->Surface_Lock(hge->Target_GetSurface(targTemp));
    if (!pixels) throw std::runtime_error("Cannot read mapshot pixels");
    const auto bounds = MapShotValidation::Plan(totalWidth, totalHeight)->Bounds(nextChunk);
    const int copyWidth = bounds[2], copyHeight = bounds[3];
    for (int y = 0; y < copyHeight; ++y) for (int x = 0; x < copyWidth; ++x) {
        const DWORD color = pixels[y * chunkWidth + x];
        gdDest.setPixel({static_cast<unsigned>(column * chunkWidth + x), static_cast<unsigned>(row * chunkHeight + y)}, sf::Color(GETR(color), GETG(color), GETB(color)));
    }
    if (targPreview && hge->Gfx_BeginScene(targPreview)) {
        hgeSprite chunk(hge->Target_GetTexture(targTemp), 0, 0, copyWidth, copyHeight);
        chunk.RenderEx(column * chunkWidth * fPreviewScale, row * chunkHeight * fPreviewScale, 0, fPreviewScale);
        hge->Gfx_EndScene();
    }
    // Target_GetTexture returns a borrowed texture. Only Target_Free owns it.
}

bool State::MapShot::Render() {
#ifdef WAPMAP_ENABLE_IMGUI
    const auto* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({420, 225.f + (sprPreview ? iPreviewH : 0)}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL(Lang_MakingMapshot), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapExportProgress";
    if (WapMapImGui::BeginFixedWindow(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse, false)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, nullptr, GETL(Lang_MakingMapshot));
        ImGui::SetCursorPos({12, 36});
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), labWhole->getCaption().c_str());
        ImGui::SetCursorPos({12, 60});
        const float overall = phase == Phase::Allocate ? 0.f : phase == Phase::Draw ? (1.f + static_cast<float>(nextChunk) / (columns * rows)) / 3.f : phase == Phase::Save ? 2.f / 3.f : 1.f;
        ImGui::ProgressBar(overall, {396, 28});
        ImGui::SetCursorPos({12, 102});
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), labAction->getCaption().c_str());
        ImGui::SetCursorPos({12, 126});
        ImGui::ProgressBar(columns && rows ? static_cast<float>(nextChunk) / (columns * rows) : 0.f, {396, 28});
        if (sprPreview) {
            const auto origin = ImGui::GetWindowPos();
            WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, sprPreview, {origin.x + (420 - iPreviewW) / 2.f, origin.y + 166});
        }
        ImGui::SetCursorPos({308, 181.f + (sprPreview ? iPreviewH : 0)});
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_Cancel), {100, 33})) cancelRequested = true;
        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)) cancelRequested = true;
    }
    ImGui::End();
    return false;
#endif
    try {
        gui->draw();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    if (sprPreview != NULL)
        sprPreview->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2, hge->System_GetState(HGE_SCREENHEIGHT) / 2 + 80);
    GV->Console->Render();
    return 0;
}

void State::MapShot::GainFocus(const ReturnCode& code, bool bFlipped) {
    _popMe({ ReturnCodeType::MapShot, 0 });
}
