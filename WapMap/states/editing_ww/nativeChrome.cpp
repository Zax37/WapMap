#include "../editing_ww.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../../cAppMenu.h"
#include "../../cModeMenuBar.h"
#include "../../cInterfaceSheet.h"
#include "../../imguiTheme.h"
#include "../../legacyText.h"
#include "../../nativeContextMenu.h"
#include "../../langID.h"
#include "../../../shared/gcnWidgets/wComboButton.h"
#include <algorithm>
#include <cstdio>
#include <cmath>

extern HGE* hge;
struct State::EditingWW::NativeChrome {
    SHR::Container* models = nullptr;
    std::vector<gcn::Widget*> widgets;
    gcn::Widget* action = nullptr;
    WWD::Parser* document = nullptr;
    float value = 0;
    bool busy = false;
    SHR::Context* context = nullptr;
    SHR::Context* contextAction = nullptr;
    int contextActionId = -1;
    WWD::Parser* contextDocument = nullptr;
};
namespace {
bool BeginStrip(const char* name,int y,int height) {
    const auto* viewport=ImGui::GetMainViewport();
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowPos({viewport->Pos.x,viewport->Pos.y+y},ImGuiCond_Always);
    ImGui::SetNextWindowSize({viewport->Size.x,static_cast<float>(height)},ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize,{1,1});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,0);
    const bool shown=ImGui::Begin(name,nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
        ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|
        ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoBringToFrontOnFocus);
    if(shown) {
        const auto p=ImGui::GetWindowPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            p, {p.x+viewport->Size.x, p.y+static_cast<float>(height)},
            IM_COL32(0x2f,0x2f,0x2f,0xff));
        ImGui::GetWindowDrawList()->AddLine({p.x+0.5f,p.y+0.5f},
            {p.x+viewport->Size.x-0.5f,p.y+0.5f},IM_COL32(20,20,20,255));
    }
    return shown;
}
void EndStrip(bool interactive) {
    if(interactive && ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() &&
       !ImGui::IsPopupOpen(nullptr,ImGuiPopupFlags_AnyPopupId) &&
       (ImGui::IsMouseReleased(ImGuiMouseButton_Left)||ImGui::IsKeyPressed(ImGuiKey_Escape))) ImGui::SetWindowFocus(nullptr);
    ImGui::End(); ImGui::PopStyleVar(3);
}
void Tooltip(gcn::Widget* widget) {
    if(widget->GetTooltip() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        const auto text=LegacyText::ToUtf8(widget->GetTooltip(),LegacyText::CodePage(GV->Lang->GetCode()));
        ImGui::SetTooltip("%s",text.c_str());
    }
}
bool DrawToolButton(SHR::But* button,int offsetY,bool interactive) {
    if(!button->isLocallyVisible()) return false;
    ImGui::PushID(button);
    ImGui::SetCursorPos({static_cast<float>(button->getX()),static_cast<float>(button->getY()-offsetY)});
    ImGui::BeginDisabled(!interactive || !button->isLocallyEnabled());
    const ImVec2 size{static_cast<float>(button->getWidth()),static_cast<float>(button->getHeight())};
    const bool hit=button->isRenderingBG()?WapMapImGui::BitmapButton(*hge,*GV->fntMyriad16,*GV->hGfxInterface,"",size):ImGui::InvisibleButton("##button",size,ImGuiButtonFlags_EnableNav);
    auto* draw=ImGui::GetWindowDrawList();
    const auto p=ImGui::GetItemRectMin(),end=ImGui::GetItemRectMax();
    if(button->isHighlighted() || ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused() || ImGui::IsItemActive())
        draw->AddRectFilled(p,end,IM_COL32(255,255,255,button->isHighlighted()?42:22));
    if(auto* icon=button->getIcon()) {
        const auto color=button->getIconColor();
        WapMapImGui::Sprite(draw,*hge,icon,{p.x+(size.x-icon->GetWidth())/2,p.y+(size.y-icon->GetHeight())/2},
            IM_COL32(GETR(color),GETG(color),GETB(color),static_cast<int>(GETA(color)*ImGui::GetStyle().Alpha)));
    }
    if(WapMapImGui::IsKeyboardFocused()) draw->AddRect(p,end,ImGui::GetColorU32(ImGuiCol_NavCursor));
    Tooltip(button);
    ImGui::EndDisabled(); ImGui::PopID();
    return hit;
}
}
void State::EditingWW::InitNativeChrome() {
    if(nativeChrome) return;
    nativeChrome=new NativeChrome;
    auto& n=*nativeChrome;
    n.models=new SHR::Container;
    n.models->setVisible(false);
    conMain->add(n.models,0,0);
    const auto mainBounds=conMain->getDimension();
    n.widgets={cbutActiveMode,hmbTile->ddActivePlane,hmbTile->butIconSelect,hmbTile->butIconPencil,hmbTile->butIconBrush,
        hmbTile->butIconFill,hmbTile->butIconWriteID,hmbTile->butZoom,butMicroTileCB,butMicroObjectCB,sliZoom};
    for(auto* button:hmbObject->vButtons) n.widgets.push_back(button);
    for(auto* widget:n.widgets) {
        const auto position=widget->getDimension();
        conMain->remove(widget); n.models->add(widget,position.x,position.y);
    }
    conMain->setDimension(mainBounds);
}
void State::EditingWW::DestroyNativeChrome() {
    if(!nativeChrome) return;
    for(auto* widget:nativeChrome->widgets) {
        const auto position=widget->getDimension();
        nativeChrome->models->remove(widget); conMain->add(widget,position.x,position.y);
    }
    conMain->remove(nativeChrome->models);
    delete nativeChrome->models;
    delete nativeChrome; nativeChrome=nullptr;
}
bool State::EditingWW::ThinkNativeChrome() {
    InitNativeChrome();
    auto& n=*nativeChrome;
    if (MDI) {
        MDI->RouteImGuiInput();
        MDI->ProcessImGuiActions();
    }
    if(n.busy) return true;
    if(n.contextAction) {
        auto* source=n.contextAction;
        n.contextAction=nullptr;
        if(!_isOnTop() || !hParser || n.contextDocument!=hParser) return false;
        auto* entry=source->GetElementByID(n.contextActionId);
        if(!entry || !entry->IsEnabled()) return false;
        n.busy=true;
        source->EmulateClickID(n.contextActionId);
        n.busy=false;
        return true;
    }
    if(!n.action) return false;
    auto* source=n.action; n.action=nullptr;
    if(std::find(n.widgets.begin(),n.widgets.end(),source)==n.widgets.end()) return false;
    if(!_isOnTop() || n.document!=hParser || !hParser || !std::isfinite(n.value) || !source->isLocallyEnabled() || !source->isLocallyVisible()) return false;
    if(source==cbutActiveMode) cbutActiveMode->setSelectedEntryID(static_cast<int>(n.value));
    else if(source==hmbTile->ddActivePlane) {
        const int plane=static_cast<int>(n.value);
        if(plane<0 || plane>=hParser->GetPlanesCount()) return false;
        hmbTile->ddActivePlane->setSelected(plane,false);
    } else if(source==sliZoom) sliZoom->setValue(n.value);
    n.busy=true;
    source->simulateAction();
    n.busy=false;
    return source!=sliZoom;
}
void State::EditingWW::DrawNativeChrome() {
    if (!hParser || !MDI || MDI->GetDocsCount() == 0)
        hge->Input_SetKeyboardCaptureOverride(false);
    InitNativeChrome();
    auto& n=*nativeChrome;
    const bool interactive=_isOnTop() && !n.busy && !n.action;
    // A modal backdrop keeps its normal contrast; one dim layer is enough.
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha,_isOnTop()?ImGui::GetStyle().DisabledAlpha:1.f);
    const auto queue=[&](gcn::Widget* source,float value=0) {
        if(interactive) { n.action=source; n.value=value; n.document=hParser; }
    };
    const int width=hge->System_GetState(HGE_SCREENWIDTH),height=hge->System_GetState(HGE_SCREENHEIGHT);
    const auto cp=LegacyText::CodePage(GV->Lang->GetCode());
    const auto* viewport=ImGui::GetMainViewport();
    const bool toolbarShown=hParser && BeginStrip("###WapMapToolbar",LAY_MODEBAR_Y,LAY_MODEBAR_H);
    if(toolbarShown) {
        if(hParser) {
            ImGui::PushID(cbutActiveMode);
            int x=cbutActiveMode->getX();
            const auto& entries=cbutActiveMode->getEntries();
            for(size_t i=0;i<entries.size();++i) {
                ImGui::PushID(static_cast<int>(i));
                const ImVec2 size{static_cast<float>(entries[i].GetWidth()),static_cast<float>(cbutActiveMode->getHeight())};
                const ImVec2 position{viewport->Pos.x+static_cast<float>(x),
                                      viewport->Pos.y+static_cast<float>(cbutActiveMode->getY())};
                ImGui::SetCursorScreenPos(position);
                const bool selected=cbutActiveMode->getSelectedEntryID()==static_cast<int>(i);
                ImGui::BeginDisabled(!interactive || !cbutActiveMode->isLocallyEnabled());
                const bool clicked=ImGui::InvisibleButton("##mode",size,ImGuiButtonFlags_EnableNav);
                const auto p=position,end=ImVec2(position.x+size.x,position.y+size.y);
                auto* draw=ImGui::GetWindowDrawList();
                const bool hovered=ImGui::IsItemHovered();
                const auto background=selected?IM_COL32(65,105,140,255):
                    (hovered?IM_COL32(82,82,82,255):IM_COL32(54,54,54,255));
                draw->AddRectFilled(p,end,background);
                draw->AddRect(p,end,IM_COL32(108,108,108,255));
                if(auto* icon=entries[i].GetIcon()) WapMapImGui::Sprite(draw,*hge,icon,{p.x+4,p.y+(size.y-icon->GetHeight())/2});
                const auto caption=LegacyText::ToUtf8(entries[i].GetCaption(),cp);
                draw->AddText({p.x+(entries[i].GetIcon()?22.f:4.f),p.y+(size.y-ImGui::GetFontSize())/2},
                    IM_COL32(240,240,240,255),caption.c_str());
                ImGui::EndDisabled();
                if(clicked) queue(cbutActiveMode,static_cast<float>(i));
                x+=entries[i].GetWidth(); ImGui::PopID();
            }
            ImGui::PopID();
            if(iMode==EWW_MODE_TILE) {
                auto* combo=hmbTile->ddActivePlane;
                ImGui::SetCursorScreenPos({viewport->Pos.x+static_cast<float>(combo->getX()),
                                           viewport->Pos.y+static_cast<float>(combo->getY()+3)});
                ImGui::SetNextItemWidth(static_cast<float>(combo->getWidth()));
                const int selected=combo->getSelected();
                const auto planeName=selected>=0 && selected<hParser->GetPlanesCount()?LegacyText::ToUtf8(hParser->GetPlane(selected)->GetName(),cp):std::string();
                const auto label=planeName.empty()?std::string("Layer"):planeName;
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,{4,2});
                ImGui::PushStyleColor(ImGuiCol_FrameBg,IM_COL32(48,48,48,255));
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,IM_COL32(68,68,68,255));
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive,IM_COL32(78,78,78,255));
                ImGui::BeginDisabled(!interactive || !combo->isLocallyEnabled());
                if(ImGui::BeginCombo("##active-plane",label.c_str())) {
                    for(int i=0;i<hParser->GetPlanesCount();++i) {
                        ImGui::PushID(i); const auto name=LegacyText::ToUtf8(hParser->GetPlane(i)->GetName(),cp);
                        if(ImGui::Selectable(name.c_str(),i==selected)) queue(combo,static_cast<float>(i));
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
                Tooltip(combo); ImGui::EndDisabled(); ImGui::PopStyleColor(3); ImGui::PopStyleVar();
            }
        }
        if(hmbTile->IsVisible()) {
            for(auto* button:{hmbTile->butIconSelect,hmbTile->butIconPencil,hmbTile->butIconBrush,hmbTile->butIconFill,hmbTile->butIconWriteID,hmbTile->butZoom})
                if(DrawToolButton(button,LAY_MODEBAR_Y,interactive && !n.action)) queue(button);
        }
        if(hmbObject->IsVisible()) for(auto* button:hmbObject->vButtons)
            if(DrawToolButton(button,LAY_MODEBAR_Y,interactive && !n.action)) queue(button);
        if(hmbActive) for(const int x:hmbActive->vSeparators) {
            const auto origin=ImGui::GetWindowPos(); ImGui::GetWindowDrawList()->AddLine({origin.x+x,origin.y+2},{origin.x+x,origin.y+LAY_MODEBAR_H-3},IM_COL32(65,65,65,255));
        }
    }
    if(toolbarShown) EndStrip(interactive);
    const int statusY=height-LAY_STATUS_H;
    if(BeginStrip("###WapMapStatus",statusY,LAY_STATUS_H)) {
        for(auto* button:{butMicroTileCB,butMicroObjectCB}) if(DrawToolButton(button,statusY,interactive && !n.action)) queue(button);
        if(hParser) {
            int wx=-1,wy=-1,tx=-1,ty=-1; float mx,my; hge->Input_GetMousePos(&mx,&my);
            if(interactive && hge->Input_IsMouseOver() && !ImGui::GetIO().WantCaptureMouse && conMain->getWidgetAt(mx,my)==vPort->GetWidget()) {
                auto* plane=GetActivePlane();
                wx=Scr2WrdX(plane,mx); wy=Scr2WrdY(plane,my);
                if(plane->GetTileWidth()>0 && plane->GetPlaneWidth()>0) tx=(wx/plane->GetTileWidth())%plane->GetPlaneWidth();
                if(plane->GetTileHeight()>0 && plane->GetPlaneHeight()>0) ty=(wy/plane->GetTileHeight())%plane->GetPlaneHeight();
            }
            ImGui::SetCursorPos({72,5});
            std::string statusText;
            if(ImGui::BeginChild("values",{std::max(1.f,width-310.f),22},ImGuiChildFlags_None,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoNavInputs)) {
                const std::pair<const char*,std::string> fields[]{
                    {GETL2S("Various","TilesOnScreen"),std::to_string(iTilesOnScreen)},
                    {GETL2S("Various","ObjectsOnScreen"),std::to_string(iObjectsOnScreen)},
                    {GETL2S("Various","WorldMousePosition"),std::to_string(wx)+"x"+std::to_string(wy)},
                    {GETL2S("Various","TileMousePosition"),std::to_string(tx)+"x"+std::to_string(ty)},
                    {"FPS:",std::to_string(hge->Timer_GetFPS())}};
                for(size_t i=0;i<std::size(fields);++i) {
                    if(i) ImGui::SameLine(0,14);
                    const auto text=LegacyText::ToUtf8(fields[i].first,cp)+" "+fields[i].second;
                    if(!statusText.empty()) statusText+='\n';
                    statusText+=text;
                    ImGui::TextUnformatted(text.c_str());
                }
            }
            ImGui::EndChild();
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) ImGui::SetTooltip("%s",statusText.c_str());
            ImGui::SetCursorPos({width-230.f,5}); ImGui::Text("%.1fx",fZoom);
        }
        if(sliZoom->isLocallyVisible()) {
            float value=static_cast<float>(sliZoom->getValue());
            ImGui::SetCursorPos({static_cast<float>(sliZoom->getX()),static_cast<float>(sliZoom->getY()-statusY)});
            ImGui::SetNextItemWidth(static_cast<float>(sliZoom->getWidth()));
            ImGui::BeginDisabled(!interactive || !sliZoom->isLocallyEnabled());
            if(ImGui::SliderFloat("##zoom",&value,-5,5,"",ImGuiSliderFlags_AlwaysClamp)) queue(sliZoom,value);
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                const auto label=LegacyText::ToUtf8(GETL2S("Various","Zoom"),cp);
                ImGui::SetTooltip("%s %.2fx",label.c_str(),fZoom);
            }
            ImGui::EndDisabled();
        }
    }
    EndStrip(interactive);
    if(hParser && MDI->GetDocsCount() > 0) {
        const float dockY=static_cast<float>(LAY_MDI_Y);
        const float dockHeight=std::max(1.f,static_cast<float>(height)-dockY-LAY_STATUS_H);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowPos({viewport->Pos.x,viewport->Pos.y+dockY},ImGuiCond_Always);
        ImGui::SetNextWindowSize({viewport->Size.x,dockHeight},ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,0);
        const auto dockFlags=ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|
            ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|
            ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoBringToFrontOnFocus|
            ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|
            ImGuiWindowFlags_NoBackground;
        ImGui::Begin("###WapMapMapDockspaceHost",nullptr,dockFlags);
        const ImGuiID dockspaceId=ImGui::GetID("WapMapMapDockspace");
        ImGui::DockSpace(dockspaceId,{0,0},ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();
        ImGui::PopStyleVar(2);
        MDI->DrawImGuiViews(dockspaceId);
        const bool showWelcome = !MDI->HasDockedMainViewportDoc();
        winWelcome->setVisible(showWelcome);
        if (showWelcome) {
            const int welcomeX = (width - winWelcome->getWidth()) / 2;
            const int canvasTop = LAY_MDI_Y;
            const int canvasHeight = height - canvasTop - LAY_STATUS_H;
            const int welcomeY = canvasTop + (canvasHeight - winWelcome->getHeight()) / 2;
            winWelcome->setPosition(welcomeX, welcomeY);
        }
    }
    // Legacy context widgets are covered by the native map image. Use their
    // models and action listeners, but let ImGui own drawing and mouse capture.
    for(auto* context : {objContext,tilContext}) {
        if(_isOnTop() && context->isVisible()) {
            n.context=context;
            n.contextDocument=hParser;
            context->hide();
            ImGui::OpenPopup("###WapMapContext");
            ImGui::SetNextWindowPos(ImGui::GetMousePos(),ImGuiCond_Appearing);
        }
    }
    if(ImGui::BeginPopup("###WapMapContext")) {
        if(!hParser || n.contextDocument!=hParser || !_isOnTop()) {
            ImGui::CloseCurrentPopup();
        } else {
            WapMapImGui::ContextMenu(n.context,*hge,[&](SHR::Context* context,int id) {
                n.contextAction=context;
                n.contextActionId=id;
            });
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}
#endif
