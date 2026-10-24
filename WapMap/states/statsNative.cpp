#include "stats.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../globals.h"
#include "../langID.h"
#include "../cInterfaceSheet.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../windows/mapStatistics.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
#include <unordered_map>

extern HGE* hge;
struct State::MapStats::Native {
    struct Plane {
        std::string name;
        int width, height, tileWidth, tileHeight, objects;
        MapStatistics::Counts attributes{};
    };
    std::string name, author, date;
    std::vector<Plane> planes;
    std::array<std::int64_t,10> treasures{};
    std::int64_t objects=0, treasureCount=0, score=0;
    bool hasClawScore=false;
};
namespace {
constexpr ImU32 TreasureColors[]{IM_COL32(255,200,40,255),IM_COL32(210,150,35,255),IM_COL32(100,180,255,255),IM_COL32(205,120,255,255),IM_COL32(245,100,140,255),IM_COL32(235,90,60,255),IM_COL32(100,220,170,255),IM_COL32(130,195,60,255),IM_COL32(245,145,35,255),IM_COL32(170,180,210,255)};
constexpr ImU32 TileColors[]{IM_COL32(210,210,210,255),IM_COL32(60,120,240,255),IM_COL32(70,200,110,255),IM_COL32(240,220,70,255),IM_COL32(230,65,65,255),IM_COL32(190,80,200,255)};
void Chart(std::span<const std::int64_t> counts,std::span<const ImU32> colors) {
    auto* draw=ImGui::GetWindowDrawList();
    const auto p=ImGui::GetCursorScreenPos();
    constexpr float radius=88, pi=3.14159265358979323846f;
    const ImVec2 center{p.x+radius+4,p.y+radius+4};
    const auto total=std::accumulate(counts.begin(),counts.end(),std::int64_t(0));
    draw->AddCircleFilled(center,radius,IM_COL32(28,28,28,255),96);
    const auto savedFlags=draw->Flags;
    draw->Flags &= ~ImDrawListFlags_AntiAliasedFill;
    double angle=-pi/2;
    if(total) for(size_t i=0;i<counts.size();++i) {
        if(!counts[i]) continue;
        const double end=angle+2*pi*static_cast<double>(counts[i])/total;
        // Individual triangles also handle a full 360-degree slice safely.
        const int segments=std::max(1,static_cast<int>(std::ceil((end-angle)*32)));
        for(int segment=0;segment<segments;++segment) {
            const double a=angle+(end-angle)*segment/segments,b=angle+(end-angle)*(segment+1)/segments;
            draw->AddTriangleFilled(center,{center.x+radius*static_cast<float>(std::cos(a)),center.y+radius*static_cast<float>(std::sin(a))},
                {center.x+radius*static_cast<float>(std::cos(b)),center.y+radius*static_cast<float>(std::sin(b))},colors[i]);
        }
        angle=end;
    }
    draw->Flags=savedFlags;
    draw->AddCircle(center,radius,IM_COL32(110,110,110,255),96);
    ImGui::Dummy({2*radius+8,2*radius+8});
}
void Property(const char* key,const std::string& value) {
    ImGui::TableNextRow(); ImGui::TableNextColumn();
    ImGui::TextUnformatted(LegacyText::ToUtf8(key,LegacyText::CodePage(GV->Lang->GetCode())).c_str());
    ImGui::TableNextColumn(); ImGui::TextUnformatted(value.c_str());
}
void Distribution(std::span<const std::int64_t> counts,std::span<const ImU32> colors,const std::vector<std::string>& labels) {
    const auto start=ImGui::GetCursorPos();
    Chart(counts,colors);
    const float legendX=ImGui::GetContentRegionAvail().x>=460?start.x+205:start.x;
    ImGui::SetCursorPos({legendX,legendX==start.x?start.y+198:start.y});
    if(ImGui::BeginTable("distribution",3,ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("##color",ImGuiTableColumnFlags_WidthFixed,14);
        ImGui::TableSetupColumn("##name"); ImGui::TableSetupColumn("##count");
        const auto total=std::accumulate(counts.begin(),counts.end(),std::int64_t(0));
        for(size_t i=0;i<counts.size();++i) {
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            const auto p=ImGui::GetCursorScreenPos(); ImGui::GetWindowDrawList()->AddRectFilled(p,{p.x+11,p.y+11},colors[i]);
            ImGui::Dummy({11,11}); ImGui::TableNextColumn(); ImGui::TextUnformatted(labels[i].c_str());
            ImGui::TableNextColumn(); ImGui::Text("%lld (%.1f%%)",static_cast<long long>(counts[i]),total?100.*counts[i]/total:0.);
        }
        ImGui::EndTable();
    }
    ImGui::SetCursorPos({start.x,std::max(ImGui::GetCursorPosY(),start.y+198)});
}
}
void State::MapStats::InitNative() {
    if(!hMap) { bKill=true; return; }
    native=new Native;
    auto& data=*native;
    data.name=hMap->GetName(); data.author=hMap->GetAuthor(); data.date=hMap->GetDate();
    data.hasClawScore=hMap->GetGame()==WWD::Game_Claw || hMap->GetGame()==WWD::Game_Claw2;
    static_assert(WWD::Attrib_Clear==0 && WWD::Attrib_Solid==1 && WWD::Attrib_Unknown==5);
    std::unordered_map<int,std::vector<unsigned char>> attributeSummaries;
    for(int i=0;i<hMap->GetPlanesCount();++i) {
        auto* plane=hMap->GetPlane(i);
        Native::Plane snapshot{plane->GetName(),plane->GetPlaneWidth(),plane->GetPlaneHeight(),plane->GetTileWidth(),plane->GetTileHeight(),plane->GetObjectsCount()};
        for(int y=0;y<snapshot.height;++y) for(int x=0;x<snapshot.width;++x) {
            auto* tile=plane->GetTile(x,y);
            std::span<const unsigned char> attributes;
            if(tile && !tile->IsInvisible() && !tile->IsFilled()) {
                auto [entry,inserted]=attributeSummaries.try_emplace(tile->GetID());
                if(inserted) if(auto* attribute=hMap->GetTileAttribs(tile->GetID()))
                    for(auto value:attribute->getAttribSummary()) entry->second.push_back(static_cast<unsigned char>(value));
                attributes=entry->second;
            }
            MapStatistics::AddTile(snapshot.attributes,!tile || tile->IsInvisible(),tile && tile->IsFilled(),attributes);
        }
        data.objects+=snapshot.objects;
        for(int o=0;o<snapshot.objects;++o)
            if(const auto treasure=MapStatistics::Treasure(plane->GetObjectByIterator(o)->GetImageSet())) ++data.treasures[*treasure];
        data.planes.push_back(std::move(snapshot));
    }
    data.treasureCount=std::accumulate(data.treasures.begin(),data.treasures.end(),std::int64_t(0));
    data.score=MapStatistics::Score(data.treasures);
}
void State::MapStats::DestroyNative() { delete native; native=nullptr; }
bool State::MapStats::RenderNative() {
    if(bKill || !native) return false;
    const auto cp=LegacyText::CodePage(GV->Lang->GetCode());
    const auto utf8=[&](const char* s){return LegacyText::ToUtf8(s,cp);};
    const char* caption=GETL2S("Stats","WindowCaption");
    const auto title=utf8(caption)+"###WapMapStats";
    if(!ImGui::IsPopupOpen(title.c_str())) ImGui::OpenPopup(title.c_str());
    const auto* viewport=ImGui::GetMainViewport();
    const ImVec2 size{std::min(680.f,viewport->WorkSize.x-16),std::min(510.f,viewport->WorkSize.y-16)};
    ImGui::SetNextWindowViewport(viewport->ID); ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,{.5f,.5f});
    ImGui::SetNextWindowSize(size,ImGuiCond_Always); ImGui::SetNextWindowScroll({0,0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0}); ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg,IM_COL32(0,0,0,145));
    bool open=true;
    const bool shown=ImGui::BeginPopupModal(title.c_str(),&open,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor(); ImGui::PopStyleVar();
    if(!open) bKill=true;
    if(!shown) return false;
    WapMapImGui::WindowChrome(*hge,*GV->fntMyriad16,GV->sprTabCloseButton,caption);
    ImGui::SetCursorPos({8,30});
    if(ImGui::BeginChild("body",{size.x-16,size.y-78},ImGuiChildFlags_NavFlattened)) {
        if(ImGui::BeginTabBar("tabs")) {
            if(ImGui::BeginTabItem(utf8(GETL2S("Stats","Tab_General")).c_str())) {
                if(ImGui::BeginTable("general",2,ImGuiTableFlags_SizingStretchProp)) {
                    Property(GETL2S("StatsNative","Name"),utf8(native->name.c_str()));
                    Property(GETL2S("StatsNative","Author"),utf8(native->author.c_str()));
                    Property(GETL2S("StatsNative","Date"),utf8(native->date.c_str()));
                    Property(GETL2S("StatsNative","Planes"),std::to_string(native->planes.size()));
                    Property(GETL2S("StatsNative","Objects"),std::to_string(native->objects));
                    Property(GETL2S("Stats","Lab_Treasures"),std::to_string(native->treasureCount));
                    if(native->hasClawScore) Property(GETL2S("StatsNative","Score"),std::to_string(native->score));
                    ImGui::EndTable();
                }
                ImGui::Spacing(); ImGui::TextWrapped("%s",utf8(GETL2S("StatsNative","TreasureNote")).c_str());
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem(utf8(GETL2S("Stats","Tab_Objects")).c_str())) {
                std::vector<std::string> labels;
                for(int i=0;i<10;++i) { const auto key=L"Lab_Treasure"+std::to_wstring(i+1); labels.push_back(utf8(GETL2SV("Stats",key.c_str()))); }
                Distribution(native->treasures,TreasureColors,labels);
                ImGui::TextWrapped("%s",utf8(GETL2S("StatsNative","TreasureNote")).c_str());
                ImGui::EndTabItem();
            }
            for(size_t i=0;i<native->planes.size();++i) {
                const auto& plane=native->planes[i];
                const auto tab=utf8(plane.name.c_str())+"###plane"+std::to_string(i);
                if(ImGui::BeginTabItem(tab.c_str())) {
                    ImGui::Text("%d x %d | %d x %d px | %s: %d",plane.width,plane.height,plane.tileWidth,plane.tileHeight,utf8(GETL2S("StatsNative","Objects")).c_str(),plane.objects);
                    const wchar_t* keys[]{L"Clear",L"Solid",L"Ground",L"Climb",L"Death",L"Unknown"};
                    std::vector<std::string> labels;
                    for(const auto* key:keys) labels.push_back(utf8(GETL2SV("StatsNative",key)));
                    Distribution(plane.attributes,TileColors,labels);
                    ImGui::TextWrapped("%s",utf8(GETL2S("StatsNative","AttributeNote")).c_str());
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
    ImGui::SetCursorPos({(size.x-150)/2,size.y-39});
    if(WapMapImGui::BitmapButton(*hge,*GV->fntMyriad16,*GV->hGfxInterface,GETL2S("StatsNative","Close"),{150,29})) bKill=true;
    if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)&&ImGui::IsKeyPressed(ImGuiKey_Escape)) bKill=true;
    if(bKill) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return false;
}
#endif
