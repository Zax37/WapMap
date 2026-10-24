#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>

class cAsset;
class cAniBankAsset;
class cSndBankAsset;
namespace ANI { class Animation; }

class winMediaBrowser : public cWindow {
    bool sounds{}, open{}, positionOnOpen{}, playing{}, loop{}, mirror{}, border{}, paused{};
    WWD::Parser* document{};
    std::string selectedName, selectedHash;
    const void* selectedInstance{};
    std::array<char, 256> filter{}, imageSet{};
    int frame{}, speed{3}, volume{50};
    double elapsed{};
    HCHANNEL channel{};
    HEFFECT effect{};
    float duration{}, position{};
    std::vector<std::pair<std::string, std::string>> metadata;
    cAsset* Resolve();
    ANI::Animation* Animation();
    void Select(cAsset* asset, bool autoPlay);
    void PlaySound();
    void StopSound();
    void DrawAnimation();
    void DrawSound();
public:
    explicit winMediaBrowser(bool sounds);
    ~winMediaBrowser() override;
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override;
    bool isOpened() override { return open; }
};
#endif
