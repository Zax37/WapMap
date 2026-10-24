#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>
#include <memory>

class winTileProperties : public cWindow {
    bool open{}, positionOnOpen{}, dragging{}, apply{};
    WWD::Parser* document{};
    std::unique_ptr<WWD::TileAttrib> draft;
    int tileID{-1}, inside{}, outside{}, dragX{}, dragY{};
    std::array<std::array<char, 32>, 7> fields{};
    void load(int id);
    void resize(int width, int height, int type);
public:
    winTileProperties();
    ~winTileProperties() override;
    void Open() override;
    void Close() override;
    void DrawImGui() override;
    void Think() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
