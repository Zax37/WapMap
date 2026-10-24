#pragma once
#include "globals.h"
#include "imguiTheme.h"
#include "legacyText.h"
#include "../shared/gcnWidgets/wContext.h"
#include <cmath>

namespace WapMapImGui {
template<class QueueAction>
void ContextMenu(SHR::Context* context, HGE& renderer, const QueueAction& queue, int depth=0) {
    if(!context || depth>16) return;
    ImGui::PushID(context);
    const auto cp=LegacyText::CodePage(GV->Lang->GetCode());
    for(int i=0;i<context->GetElementsCount();++i) {
        auto* entry=context->GetElementAt(i);
        if(context->HasSeparatorBefore(i)) ImGui::Separator();
        ImGui::PushID(i);
        const auto label="        "+LegacyText::ToUtf8(entry->GetCaption(),cp);
        auto* draw=ImGui::GetWindowDrawList();
        const auto drawIcons=[&] {
            const auto p=ImGui::GetItemRectMin(),end=ImGui::GetItemRectMax();
            for(int slot=0;slot<2;++slot) {
                if(auto* icon=entry->GetIcon(slot)) {
                    const float x=slot?end.x-40.f:p.x+4.f;
                    const float y=std::round(p.y+(end.y-p.y-icon->GetHeight())/2);
                    const int alpha=static_cast<int>(255.f*(entry->IsEnabled()?1.f:ImGui::GetStyle().DisabledAlpha));
                    Sprite(draw,renderer,icon,{x,y},IM_COL32(255,255,255,alpha));
                }
            }
        };
        if(auto* cascade=entry->GetCascade()) {
            const bool opened=ImGui::BeginMenu(label.c_str(),entry->IsEnabled());
            drawIcons();
            if(opened) {
                ContextMenu(cascade,renderer,queue,depth+1);
                ImGui::EndMenu();
            }
        } else {
            if(ImGui::MenuItem(label.c_str(),nullptr,false,entry->IsEnabled())) queue(context,entry->GetID());
            drawIcons();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
}
}
