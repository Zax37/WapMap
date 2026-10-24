#include "imageImport.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "editing_ww.h"
#include "../globals.h"
#include "../langID.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../windows/assetBrowserUi.h"
#include "../windows/imageImportData.h"
#include "../windows/imageImportFiles.h"
#include "../databanks/tiles.h"
#include "../databanks/imageSets.h"
#include <SFML/Graphics/Image.hpp>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <cstdio>
#include <commdlg.h>

extern HGE* hge;
namespace State {
namespace {
std::string Upper(std::string s) { for(auto& c:s) { if(c>='a'&&c<='z') c-='a'-'A'; if(c=='\\') c='/'; } return s; }
std::string Utf8(const std::filesystem::path& path) { const auto u=path.u8string(); return {reinterpret_cast<const char*>(u.data()),u.size()}; }
}
struct ImageImport::Native {
    struct Row {
        std::filesystem::path path;
        ImageImportData::Image data;
        std::array<char,32> id{};
        std::string warning;
        bool valid=false;
        HTEXTURE texture=nullptr;
        std::unique_ptr<hgeSprite> sprite;
        ~Row() { sprite.reset(); if(texture) hge->Texture_Free(texture); }
    };
    ImageImportType type;
    WWD::Parser* document=nullptr;
    std::string destination, error, nameError, resultName;
    std::array<char,512> name{};
    std::vector<std::unique_ptr<Row>> rows;
    std::vector<std::string> dropped;
    bool busy=false, close=false, browse=false, save=false, autoID=false, valid=false;
    int remove=-1, focusedRow=-1;
    int MaxID() const { return type==ImageImportTile ? 32767 : INT_MAX; }
    cTileImageSet* TileSet() const { return destination.empty()?nullptr:GV->editState->hTileset->GetSet(destination.c_str(),0); }
    cSprBankAsset* ImageSet() const { return destination.empty()?nullptr:GV->editState->SprBank->GetAssetByID(destination.c_str()); }
    std::optional<std::string> Folder() {
        auto& e=*GV->editState;
        std::string folder;
        if (!destination.empty()) {
            if (type==ImageImportTile) { if (!TileSet()) return std::nullopt; folder=destination; }
            else {
                auto* set=ImageSet();
                if (!set || !destination.starts_with("CUSTOM_")) return std::nullopt;
                if (set->GetSpritesCount()>0) {
                    auto path=Upper(set->GetIMGByIterator(0)->GetFile().strPath);
                    if (!path.starts_with("IMAGES/")) return std::nullopt;
                    const auto slash=path.rfind('/');
                    if (slash<=7) return std::nullopt;
                    folder=path.substr(7,slash-7);
                } else { folder=destination.substr(7); for(auto& c:folder) if(c=='_') c='/'; }
            }
        } else {
            const auto encoded=LegacyText::FromUtf8(name.data(),CP_ACP,128);
            if (!encoded || !ImageImportData::Name(*encoded,type==ImageImportObject)) return std::nullopt;
            folder=Upper(*encoded);
            if (type==ImageImportTile && e.hTileset->GetSet(folder.c_str(),0)) return std::nullopt;
            auto mounted=folder; for(auto& c:mounted) if(c=='/') c='_';
            if (type==ImageImportObject && e.SprBank->GetAssetByID(("CUSTOM_"+mounted).c_str())) return std::nullopt;
        }
        if (!ImageImportData::Name(folder,type==ImageImportObject)) return std::nullopt;
        return folder;
    }
    void Validate() {
        auto& e=*GV->editState;
        valid=!rows.empty();
        const auto folder=Folder();
        nameError.clear(); resultName.clear();
        if (!folder) { valid=false; nameError=GETL2S("ImageImportNative","InvalidName"); }
        else { resultName=*folder; if(type==ImageImportObject) { for(auto& c:resultName) if(c=='/') c='_'; resultName="CUSTOM_"+resultName; } }
        std::unordered_map<int,int> counts;
        for(const auto& row:rows) if(auto id=ImageImportData::ID(row->id.data(),MaxID())) ++counts[*id];
        for(auto& row:rows) {
            const auto id=ImageImportData::ID(row->id.data(),MaxID());
            row->warning.clear(); row->valid=true;
            if(!id) { row->warning=GETL2S("ImageImportNative","InvalidID"); row->valid=false; }
            else if(counts[*id]>1) { row->warning=GETL2S("Win_ImageImport","WarnMultipleID"); row->valid=false; }
            else {
                cAsset* existing=nullptr;
                if(type==ImageImportTile) { if(auto* set=TileSet()) existing=set->GetTile(static_cast<short>(*id)); }
                else if(auto* set=ImageSet()) existing=set->GetIMGByID(*id);
                if(existing && existing->GetFile().hFeed==e.hDataCtrl->GetFeed(DB_FEED_CUSTOM)) {
                    row->warning=GETL2SV("Win_ImageImport",type==ImageImportTile?L"WarnUsedByCustomTile":L"WarnUsedByCustomImage"); row->valid=false;
                } else if(existing) row->warning=GETL2SV("Win_ImageImport",type==ImageImportTile?L"InfoTileReplace":L"InfoImageReplace");
                if(folder && e.hDataCtrl->GetFeed(DB_FEED_CUSTOM)) {
                    const auto path=std::filesystem::path(e.hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation())/(type==ImageImportTile?"TILES":"IMAGES")/ *folder/(std::to_string(*id)+".pid");
                    std::error_code ec;
                    if(std::filesystem::exists(path,ec)) { row->valid=false; row->warning=GETL2S("ImageImportNative","ExistingFile"); }
                    if(ec) { row->valid=false; row->warning=ec.message(); }
                }
            }
            valid=valid&&row->valid;
        }
        if(!e.hParser->GetFilePath() || !*e.hParser->GetFilePath()) {
            valid=false;
            nameError=GETL2S("ImageImportNative","SaveMapFirst");
        }
    }
};
void ImageImport::InitNative(ImageImportType type,void* destination) {
    native=new Native;
    native->type=type; native->document=GV->editState->hParser;
    if(destination) native->destination=type==ImageImportTile?static_cast<cTileImageSet*>(destination)->GetName():static_cast<cSprBankAsset*>(destination)->GetID();
}
void ImageImport::DestroyNative() { delete native; native=nullptr; }

void ImageImport::AddNativeFile(const std::string& path) {
    native->dropped.push_back(path);
}
bool ImageImport::ThinkNative() {
    auto& n=*native;
    if(n.busy) return false;
    auto& e=*GV->editState;
    if(n.document!=e.hParser || !e.hParser || n.close) { _popMe({}); return false; }
    if(!_isOnTop()) return false;
    n.busy=true;
    if(n.remove>=0 && static_cast<size_t>(n.remove)<n.rows.size()) { n.rows.erase(n.rows.begin()+n.remove); n.focusedRow=-1; }
    n.remove=-1;
    if(n.browse) {
        n.browse=false;
        n.error.clear();
        std::vector<wchar_t> selected(65536);
        OPENFILENAMEW dialog{};
        dialog.lStructSize=sizeof(dialog); dialog.hwndOwner=hge->System_GetState(HGE_HWND);
        dialog.lpstrFilter=L"Images (*.pid;*.pcx;*.png;*.bmp;*.jpg;*.jpeg;*.gif)\0*.pid;*.pcx;*.png;*.bmp;*.jpg;*.jpeg;*.gif\0\0";
        dialog.lpstrFile=selected.data(); dialog.nMaxFile=static_cast<DWORD>(selected.size());
        dialog.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
        if(GetOpenFileNameW(&dialog)) {
            const std::filesystem::path first(selected.data());
            auto* p=selected.data()+wcslen(selected.data())+1;
            if(!*p) n.dropped.push_back(Utf8(first));
            else for(;*p;p+=wcslen(p)+1) n.dropped.push_back(Utf8(first/p));
        } else if(const auto code=CommDlgExtendedError()) n.error="File dialog error: "+std::to_string(code);
    }
    ImageImportData::Palette palette{};
    for(int i=0;i<256;++i) palette[i]=e.hDataCtrl->GetPalette()->GetColor(i);
    int transparent=0;
    if(e.hParser->GetGame()!=WWD::Game_Claw) for(int i=0;i<256;++i) if(palette[i]==0xFFFF0084u) { transparent=i; break; }
    if(!n.dropped.empty()) n.error.clear();
    for(const auto& path:n.dropped) {
        try {
            const auto file=std::filesystem::u8path(path).lexically_normal();
            bool duplicate=false;
            for(const auto& row:n.rows) { std::error_code ec; if(std::filesystem::equivalent(file,row->path,ec)) { duplicate=true; break; } }
            if(duplicate) continue;
            const auto size=std::filesystem::file_size(file);
            if(!size || size>256*1024*1024) throw std::runtime_error(GETL2S("ImageImportNative","ReadError"));
            ImageImportData::Bytes bytes(static_cast<size_t>(size));
            std::ifstream stream(file,std::ios::binary);
            if(!stream.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()))) throw std::runtime_error(GETL2S("ImageImportNative","ReadError"));
            auto extension=Upper(file.extension().string());
            std::optional<ImageImportData::Image> image;
            if(extension==".PID") image=ImageImportData::PID(bytes);
            else if(extension==".PCX") image=ImageImportData::PCX(bytes);
            else {
                sf::Image decoded;
                if(!decoded.loadFromMemory(bytes.data(),bytes.size())) throw std::runtime_error(GETL2S("ImageImportNative","ReadError"));
                const auto size=decoded.getSize();
                if(size.x>16384 || size.y>16384 || !ImageImportData::Dimensions(size.x,size.y)) throw std::runtime_error(GETL2S("ImageImportNative","ReadError"));
                image.emplace(); image->width=size.x; image->height=size.y;
                image->pixels.resize(size_t(size.x)*size.y);
                const auto* rgba=decoded.getPixelsPtr();
                std::unordered_map<std::uint32_t,unsigned char> cache;
                for(size_t i=0;i<image->pixels.size();++i) {
                    const auto color=(std::uint32_t(rgba[4*i+3])<<24)|(rgba[4*i]<<16)|(rgba[4*i+1]<<8)|rgba[4*i+2];
                    auto it=cache.find(color);
                    if(it==cache.end()) { if(cache.size()>65536) cache.clear(); it=cache.emplace(color,ImageImportData::Closest(color,palette,transparent)).first; }
                    image->pixels[i]=it->second;
                }
            }
            if(!image) throw std::runtime_error(GETL2S("ImageImportNative","ReadError"));
            if(n.type==ImageImportTile) {
                auto* set=n.TileSet();
                const int w=set?set->GetTileWidth():e.hParser->GetPlane(0)->GetTileWidth();
                const int h=set?set->GetTileHeight():e.hParser->GetPlane(0)->GetTileHeight();
                if(image->width!=w || image->height!=h) throw std::runtime_error(GETL2S("ImageImportNative","TileSize"));
            }
            if(image->palette && !(image->flags & 64)) {
                std::array<unsigned char,256> mapping{};
                for(int i=0;i<256;++i) mapping[i]=i==0?static_cast<unsigned char>(transparent):ImageImportData::Closest((*image->palette)[i],palette,transparent,false);
                for(auto& pixel:image->pixels) pixel=mapping[pixel];
                image->palette.reset();
            }
            auto row=std::make_unique<Native::Row>(); row->path=file; row->data=std::move(*image);
            // Preview has its own texture: deleting/repacking another row cannot invalidate it.
            row->texture=hge->Texture_Create(row->data.width,row->data.height);
            if(!row->texture) throw std::runtime_error(GETL2S("ImageImportNative","ReadError"));
            auto* pixels=hge->Texture_Lock(row->texture);
            if(!pixels) throw std::runtime_error(GETL2S("ImageImportNative","ReadError"));
            const int stride=hge->Texture_GetWidth(row->texture);
            for(int y=0;y<row->data.height;++y) for(int x=0;x<row->data.width;++x) {
                const auto id=row->data.pixels[size_t(y)*row->data.width+x];
                pixels[size_t(y)*stride+x]=(row->data.flags&64)?(std::uint32_t(id)<<24)|0xFFFFFFu:(id==transparent?0x00FFFFFFu:palette[id]);
            }
            hge->Texture_Unlock(row->texture);
            row->sprite=std::make_unique<hgeSprite>(row->texture,0,0,row->data.width,row->data.height);
            if(const auto id=ImageImportData::FilenameID(path,n.MaxID())) std::snprintf(row->id.data(),row->id.size(),"%d",*id);
            n.rows.push_back(std::move(row));
        } catch(const std::exception& ex) { n.error=path+"\n"+LegacyText::ToUtf8(ex.what(),LegacyText::CodePage(GV->Lang->GetCode())); }
    }
    n.dropped.clear();
    if(n.autoID) {
        n.autoID=false;
        std::int64_t next=1;
        if(auto* set=n.TileSet()) next=static_cast<std::int64_t>(set->GetMaxTileID())+1;
        if(auto* set=n.ImageSet()) next=static_cast<std::int64_t>(set->GetMaxIMGID())+1;
        if(next+static_cast<std::int64_t>(n.rows.size())-1>n.MaxID()) n.error=LegacyText::ToUtf8(GETL2S("ImageImportNative","InvalidID"),LegacyText::CodePage(GV->Lang->GetCode()));
        else for(auto& row:n.rows) std::snprintf(row->id.data(),row->id.size(),"%d",static_cast<int>(next++));
    }
    n.Validate();
    if(n.save) {
        n.save=false;
        if(n.valid) {
            n.error.clear();
            try {
                const auto folder=n.Folder();
                if(!folder) throw std::runtime_error(GETL2S("ImageImportNative","InvalidName"));
                std::vector<ImageImportFiles::Output> output;
                for(const auto& row:n.rows) output.push_back({*ImageImportData::ID(row->id.data(),n.MaxID()),ImageImportData::EncodePID(row->data)});
                e.hDataCtrl->FixCustomDir();
                const auto directory=std::filesystem::path(e.hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation())/(n.type==ImageImportTile?"TILES":"IMAGES")/ *folder;
                if(!ImageImportFiles::Save(directory,output,n.error)) throw std::runtime_error(n.error);
                n.close=true;
            } catch(const std::exception& ex) { n.error=LegacyText::ToUtf8(ex.what(),LegacyText::CodePage(GV->Lang->GetCode())); }
        }
    }
    n.busy=false;
    return false;
}

bool ImageImport::RenderNative() {
    auto& n=*native;
    if(n.busy || n.document!=GV->editState->hParser) return false;
    n.Validate();
    const auto cp=LegacyText::CodePage(GV->Lang->GetCode());
    const char* caption=GETL2SV("Win_ImageImport",n.type==ImageImportTile?L"WinCaption":L"WinCaption2");
    const auto title=LegacyText::ToUtf8(caption,cp)+"###WapMapImageImport";
    if(!ImGui::IsPopupOpen(title.c_str())) ImGui::OpenPopup(title.c_str());
    const auto* viewport=ImGui::GetMainViewport();
    const ImVec2 size{std::min(720.f,viewport->WorkSize.x-16),std::min(660.f,viewport->WorkSize.y-16)};
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,{.5f,.5f});
    ImGui::SetNextWindowSize(size,ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg,IM_COL32(0,0,0,145));
    bool open=true;
    const bool shown=ImGui::BeginPopupModal(title.c_str(),&open,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor(); ImGui::PopStyleVar();
    if(!open) n.close=true;
    if(!shown) return false;
    if(n.close) { ImGui::CloseCurrentPopup(); ImGui::EndPopup(); return false; }
    WapMapImGui::WindowChrome(*hge,*GV->fntMyriad16,GV->sprTabCloseButton,caption);
    const auto text=[&](const char* value) { WapMapImGui::BitmapText(ImGui::GetWindowDrawList(),*hge,*GV->fntMyriad16,ImGui::GetCursorScreenPos(),value); ImGui::Dummy({0,16}); };
    float y=32;
    if(n.destination.empty()) {
        ImGui::SetCursorPos({10,y}); text(GETL2SV("Win_ImageImport",n.type==ImageImportTile?L"TilesetName":L"ImagesetName"));
        ImGui::SetCursorPos({160,y}); ImGui::SetNextItemWidth(size.x-170);
        ImGui::InputText("##name",n.name.data(),n.name.size());
        y+=28;
    }
    ImGui::SetCursorPos({10,y}); text(GETL2S("Win_ImageImport","ResultName"));
    ImGui::SetCursorPos({160,y}); text(n.resultName.empty()?"-":n.resultName.c_str()); y+=24;
    if(!n.nameError.empty()) { ImGui::SetCursorPos({10,y}); text(n.nameError.c_str()); y+=22; }
    ImGui::SetCursorPos({10,y}); text(GETL2S("Win_ImageImport","FilesToImport")); y+=20;
    ImGui::SetCursorPos({8,y});
    ImGui::PushStyleColor(ImGuiCol_ChildBg,IM_COL32(18,18,18,255));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,{4,0});
    if(ImGui::BeginChild("files",{size.x-16,std::max(80.f,size.y-y-158)},ImGuiChildFlags_Borders|ImGuiChildFlags_NavFlattened)) {
        if(n.rows.empty()) { text(GETL2S("Win_ImageImport","NoFiles")); text(GETL2S("Win_ImageImport","DragDropHint")); }
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(n.rows.size()),81);
        if(n.focusedRow>=0 && static_cast<size_t>(n.focusedRow)<n.rows.size()) clipper.IncludeItemByIndex(n.focusedRow);
        while(clipper.Step()) for(int index=clipper.DisplayStart;index<clipper.DisplayEnd;++index) {
            const size_t i=static_cast<size_t>(index);
            auto& row=*n.rows[i]; ImGui::PushID(&row);
            const auto top=ImGui::GetCursorPos();
            const auto p=ImGui::GetCursorScreenPos();
            AssetBrowserUi::Preview(*hge,row.sprite.get(),p,64,false,true);
            ImGui::SetCursorPos({top.x+76,top.y});
            const auto filename=Utf8(row.path.filename()); ImGui::TextUnformatted(filename.c_str());
            if(ImGui::IsItemHovered()) { const auto full=Utf8(row.path); ImGui::SetTooltip("%s",full.c_str()); }
            ImGui::SetCursorPos({top.x+76,top.y+23}); ImGui::SetNextItemWidth(90);
            ImGui::InputText("##id",row.id.data(),row.id.size(),ImGuiInputTextFlags_CharsDecimal);
            if(ImGui::IsItemActive()) n.focusedRow=index;
            if(!row.valid) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(),ImGui::GetItemRectMax(),IM_COL32(220,75,75,255));
            ImGui::SameLine(); ImGui::Text("%d x %d",row.data.width,row.data.height);
            ImGui::SameLine(); if(ImGui::SmallButton("X")) n.remove=static_cast<int>(i);
            if(!row.warning.empty()) { ImGui::SetCursorPos({top.x+76,top.y+48}); text(row.warning.c_str()); }
            ImGui::SetCursorPos({top.x,top.y+80}); ImGui::Dummy({1,1});
            ImGui::PopID();
        }
    }
    ImGui::EndChild(); ImGui::PopStyleVar(); ImGui::PopStyleColor();
    ImGui::SetCursorPos({10,size.y-146});
    if(WapMapImGui::BitmapButton(*hge,*GV->fntMyriad16,*GV->hGfxInterface,GETL2S("Win_ImageImport","SelectFiles"),{160,25})) n.browse=true;
    ImGui::SameLine(); ImGui::BeginDisabled(n.rows.empty());
    if(WapMapImGui::BitmapButton(*hge,*GV->fntMyriad16,*GV->hGfxInterface,GETL2S("Win_ImageImport","AutoID"),{160,25})) n.autoID=true;
    ImGui::EndDisabled();
    ImGui::SetCursorPos({10,size.y-115}); text(GETL2S("Win_ImageImport","SupportedTypes"));
    ImGui::SetCursorPos({10,size.y-97}); text(GETL2S("Win_ImageImport","TruecolorNote"));
    if(!n.error.empty()) { ImGui::SetCursorPos({10,size.y-76}); if(ImGui::BeginChild("error",{size.x-20,30})) ImGui::TextWrapped("%s",n.error.c_str()); ImGui::EndChild(); }
    ImGui::SetCursorPos({size.x/2-155,size.y-39}); ImGui::BeginDisabled(!n.valid);
    if(WapMapImGui::BitmapButton(*hge,*GV->fntMyriad16,*GV->hGfxInterface,GETL2S("Win_ImageImport","Save"),{150,29})) n.save=true;
    ImGui::EndDisabled(); ImGui::SameLine();
    if(WapMapImGui::BitmapButton(*hge,*GV->fntMyriad16,*GV->hGfxInterface,GETL2S("Win_ImageImport","Cancel"),{150,29})) n.close=true;
    if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)&&ImGui::IsKeyPressed(ImGuiKey_Escape)) n.close=true;
    if(n.close) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return false;
}
}
#endif
