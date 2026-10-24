#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>
#include <vector>
#include <string>

class winPlaneProperties : public cWindow {
    struct TileSets : gcn::ListModel {
        std::vector<std::string> names;
        int getNumberOfElements() override { return static_cast<int>(names.size()); }
        std::string getElementAt(int index) override { return names.at(index); }
    } tileSets;
    bool open{}, positionOnOpen{};
    WWD::Parser* document{};
    WWD::Plane* plane{};
    int selected{}, tileSet{}, anchor{5}, planeCount{};
    int pending{};
    bool busy{};
    std::array<std::array<char, 257>, 10> fields{};
    std::array<bool, 5> flags{};
    void load(int index);
public:
    winPlaneProperties();
    void Open() override;
    void Close() override;
    void DrawImGui() override;
    void Think() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
