#include "cAppMenu.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "states/editing_ww.h"
#include "cInterfaceSheet.h"
#include "imguiTheme.h"
#include "legacyText.h"
#include "langID.h"
#include <cmath>

extern HGE* hge;

void cAppMenu::QueueNativeAction(SHR::Context* context,SHR::ContextEl* entry,int plane) {
    nativeActionContext=context;
    nativeActionID=entry->GetID();
    nativeActionCaption=entry->GetCaption();
    nativeActionValue=context==conOpenMRU?GV->editState->hMruList->GetRecentlyUsedFile(entry->GetID()):"";
    nativeActionPlane=plane;
    nativeDocument=GV->editState->hParser;
    nativeReleaseFocus=true;
}
bool cAppMenu::ThinkNative() {
    if(nativeBusy) return true;
    if(!nativeActionContext) return false;
    auto* context=nativeActionContext;
    nativeActionContext=nullptr;
    if(!GV->editState->_isOnTop() || nativeDocument!=GV->editState->hParser || !bEnabled || !mEnabled) return false;
    bool reachable=false;
    const auto find=[&](auto&& self,SHR::Context* root,int depth)->bool {
        if(depth>8) return false;
        if(root==context) return true;
        for(int i=0;i<root->GetElementsCount();++i) {
            auto* entry=root->GetElementAt(i);
            if(entry->IsEnabled() && entry->GetCascade() && self(self,entry->GetCascade(),depth+1)) return true;
        }
        return false;
    };
    for(auto* entry:hEntries) if(entry->IsEnabled() && find(find,entry->GetContext(),0)) reachable=true;
    if(!reachable) return false;
    auto* selected=context->GetElementByID(nativeActionID);
    if(!selected || !selected->IsEnabled() || nativeActionCaption!=selected->GetCaption()) return false;
    if(context==conOpenMRU && (nativeActionID<0 || nativeActionID>=GV->editState->hMruList->GetFilesCount() ||
        nativeActionValue!=GV->editState->hMruList->GetRecentlyUsedFile(nativeActionID))) return false;
    if(nativeActionPlane>=0) {
        auto* parser=GV->editState->hParser;
        if(!parser || nativeActionPlane>=parser->GetPlanesCount()) return false;
        for(int i=0;i<conPlanesVisibilityList->GetElementsCount();++i)
            if(conPlanesVisibilityList->GetElementAt(i)->GetID()==nativeActionPlane) conPlanesVisibilityList->SetSelectedIt(i);
        SyncPlaneSelectedVisibility();
        if(!selected->IsEnabled()) return false;
    }
    nativeBusy=true;
    context->EmulateClickID(nativeActionID);
    // Plane actions may request a legacy submenu. Native popups own visibility.
    for(auto* entry:hEntries) entry->GetContext()->setVisible(false);
    conOpenMRU->setVisible(false);
    conPlanesVisibilityList->setVisible(false);
    conPlaneVisibility->setVisible(false);
    iSelected=iHovered=iOpened=-1;
    nativeBusy=false;
    return true;
}
void cAppMenu::DrawNativeContext(SHR::Context* context,int plane,int depth) {
    if(depth>8) return;
    ImGui::PushStyleVar(ImGuiStyleVar_MenuItemRounding,4.f);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,IM_COL32(0x17,0x8c,0xe1,0xff));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,IM_COL32(0x17,0x8c,0xe1,0xff));
    const auto cp=LegacyText::CodePage(GV->Lang->GetCode());
    for(int i=0;i<context->GetElementsCount();++i) {
        auto* entry=context->GetElementAt(i);
        if(context->HasSeparatorBefore(i)) ImGui::Separator();
        ImGui::PushID(entry->GetID());
        const auto caption=LegacyText::ToUtf8(entry->GetCaption(),cp);
        const auto label="        "+caption;
        const bool enabled=entry->IsEnabled() && !nativeActionContext;
        auto* parentDraw=ImGui::GetWindowDrawList();
        const auto parentPos=ImGui::GetWindowPos();
        const auto parentSize=ImGui::GetWindowSize();
        const auto beginSubmenu=[&] {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{8,8});
            const bool opened=ImGui::BeginMenu(label.c_str(),enabled);
            ImGui::PopStyleVar();
            if(opened) {
                const auto popupPos=ImGui::GetWindowPos();
                if(popupPos.x>=parentPos.x+parentSize.x-1.f)
                    ImGui::SetWindowPos({popupPos.x+4.f,popupPos.y},ImGuiCond_Always);
            }
            return opened;
        };
        const auto drawIcon=[&] {
            if(auto* icon=entry->GetIcon()) {
                const auto p=ImGui::GetItemRectMin(),end=ImGui::GetItemRectMax();
                const float iconY=std::round(p.y+(end.y-p.y-icon->GetHeight())/2);
                WapMapImGui::Sprite(parentDraw,*hge,icon,{p.x+4,iconY});
            }
        };
        if(context==conPlanesVisibilityList) {
            const bool opened=beginSubmenu();
            drawIcon();
            if(opened) {
                const int id=entry->GetID();
                auto* parser=GV->editState->hParser;
                if(parser && id>=0 && id<parser->GetPlanesCount()) {
                    conPlanesVisibilityList->SetSelectedIt(i);
                    SyncPlaneSelectedVisibility();
                    const bool visible=GV->editState->hPlaneData[id]->bDraw;
                    const auto visibleLabel=LegacyText::ToUtf8(GETL2S("AppMenuNative","PlaneVisible"),cp);
                    if(ImGui::MenuItem(visibleLabel.c_str(),nullptr,visible)) QueueNativeAction(context,entry,id);
                    ImGui::Separator();
                    ImGui::BeginDisabled(!visible);
                    DrawNativeContext(conPlaneVisibility,id,depth+1);
                    ImGui::EndDisabled();
                }
                ImGui::EndMenu();
            }
        } else if(entry->GetCascade()) {
            const bool opened=beginSubmenu();
            drawIcon();
            if(opened) {
                DrawNativeContext(entry->GetCascade(),plane,depth+1);
                ImGui::EndMenu();
            }
        } else {
            if(ImGui::MenuItem(label.c_str(),nullptr,entry->GetIcon(true)!=nullptr,enabled)) QueueNativeAction(context,entry,plane);
            drawIcon();
        }
        ImGui::PopID();
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}
int cAppMenu::GetNativeMenuWidth() const {
    if(!ImGui::GetCurrentContext()) return GetMenuWidth()-LAY_APPMENU_X;
    const auto cp=LegacyText::CodePage(GV->Lang->GetCode());
    const auto& style=ImGui::GetStyle();
    float width=0;
    for(int i=0;i<AppMenu_EntryCount;++i) {
        const auto caption=LegacyText::ToUtf8(hEntries[i]->GetLabel(),cp);
        width+=ImGui::CalcTextSize(caption.c_str()).x+style.FramePadding.x*2;
        if(i+1<AppMenu_EntryCount) width+=style.ItemSpacing.x;
    }
    return static_cast<int>(std::ceil(width));
}
void cAppMenu::DrawImGui() {
    const bool interactive=GV->editState->_isOnTop() && !nativeBusy;
    if(!interactive) nativeFocus=false;
    const auto* viewport=ImGui::GetMainViewport();
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowPos({viewport->Pos.x+LAY_APPMENU_X,viewport->Pos.y+LAY_APPMENU_Y},ImGuiCond_Always);
    const float menuHeight=static_cast<float>(LAY_APPMENU_H-1);
    const float menuFramePaddingY=(menuHeight-ImGui::GetFontSize())*0.5f;
    ImGui::SetNextWindowSize({static_cast<float>(GetNativeMenuWidth()),menuHeight},ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize,{1,1});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,0);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,4.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,4.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,{6,menuFramePaddingY});
    ImGui::PushStyleVar(ImGuiStyleVar_MenuItemRounding,4.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,0);
    ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha,GV->editState->_isOnTop()?ImGui::GetStyle().DisabledAlpha:1.f);
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg,IM_COL32(0x2f,0x2f,0x2f,0xff));
    ImGui::PushStyleColor(ImGuiCol_PopupBg,IM_COL32(0x0f,0x0f,0x0f,0xff));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,IM_COL32(0x45,0x45,0x45,0xff));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,IM_COL32(0x45,0x45,0x45,0xff));
    const bool shown=ImGui::Begin("###WapMapApplicationMenu",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoBringToFrontOnFocus);
    if(shown && ImGui::BeginMenuBar()) {
        ImGui::BeginDisabled(!interactive);
        if(interactive && nativeReleaseFocus) { ImGui::SetWindowFocus(nullptr); nativeReleaseFocus=false; }
        const bool focus=interactive && (nativeFocus || (ImGui::IsKeyPressed(ImGuiKey_F10) && !ImGui::GetIO().WantTextInput));
        bool focused=false;
        bool anyMenuOpen=false;
        for(int i=0;i<AppMenu_EntryCount;++i) {
            const bool enabled=bEnabled && mEnabled && hEntries[i]->IsEnabled();
            if(focus && enabled && !focused) { ImGui::SetWindowFocus(); ImGui::SetKeyboardFocusHere(); focused=true; }
            ImGui::PushID(i);
            const auto caption=LegacyText::ToUtf8(hEntries[i]->GetLabel(),LegacyText::CodePage(GV->Lang->GetCode()));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{8,8});
            if(ImGui::BeginMenu(caption.c_str(),enabled)) { anyMenuOpen=true; DrawNativeContext(hEntries[i]->GetContext()); ImGui::EndMenu(); }
            ImGui::PopStyleVar();
            ImGui::PopID();
        }
        nativeFocus=false;
        ImGui::EndDisabled();
        ImGui::EndMenuBar();
        if(interactive && (nativeReleaseFocus || (!anyMenuOpen && ImGui::IsWindowFocused() &&
            (ImGui::IsKeyPressed(ImGuiKey_Escape) || (!focus && (ImGui::IsKeyReleased(ImGuiKey_LeftAlt) || ImGui::IsKeyReleased(ImGuiKey_RightAlt))) ||
             (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered()))))) {
            ImGui::SetWindowFocus(nullptr);
            nativeReleaseFocus=false;
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(9);
    const int buttonsLeft = hge->System_GetState(HGE_SCREENWIDTH) -
        LAY_APP_BUTTONS_COUNT * LAY_APP_BUTTON_W - GV->editState->bMaximized * 2;
    const float menuRight = viewport->Pos.x + LAY_APPMENU_X + GetNativeMenuWidth();
    if (menuRight < viewport->Pos.x + buttonsLeft) {
        ImGui::GetForegroundDrawList()->AddRectFilled(
            {menuRight, viewport->Pos.y + LAY_APPMENU_Y},
            {viewport->Pos.x + buttonsLeft, viewport->Pos.y + LAY_APPMENU_Y + menuHeight},
            IM_COL32(0x2f,0x2f,0x2f,0xff));
    }
    if(!GV->editState->bMaximized) {
        auto* draw=ImGui::GetForegroundDrawList();
        const auto min=viewport->Pos;
        const auto max=ImVec2(viewport->Pos.x+viewport->Size.x,viewport->Pos.y+viewport->Size.y);
        const auto color=ImGui::GetColorU32(ImGuiCol_Border);
        draw->AddLine({min.x+0.5f,min.y+0.5f},{max.x-0.5f,min.y+0.5f},color);
        draw->AddLine({min.x+0.5f,min.y+0.5f},{min.x+0.5f,max.y-0.5f},color);
        draw->AddLine({min.x+0.5f,max.y-0.5f},{max.x-0.5f,max.y-0.5f},color);
        draw->AddLine({max.x-0.5f,min.y+0.5f},{max.x-0.5f,max.y-0.5f},color);
    }
}
#endif
