#ifndef H_WIN_TILEBROWSER
#define H_WIN_TILEBROWSER

#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wScrollArea.h"

#include "window.h"

class cTile;

class winTileBrowser : public cWindow {
private:
    SHR::Lab *labTileSets, *labTileSetName, *labTileSetChecksum, *labTileCount,
            *labTileSetNameV, *labTileSetChecksumV, *labTileCountV;
    SHR::ScrollArea *saTiles, *saTileSets;
    SHR::Container *conTiles, *conTileSets;
    WIDG::Viewport *vpTileBrowser;
    SHR::But *butImportTileSet, *butDeleteTileSet, *butRenameTileSet, *butBrowseFolder, *butDeleteTile, *butChangeTileID, *butImageDetails,
            *butAddTiles;
    SHR::Lab *labTileID, *labTileModDate, *labTileChecksum, *labTileOrigin,
            *labTileIDV, *labTileModDateV, *labTileChecksumV, *labTileOriginV;
    int iSelectedTileSet, iSelectedTile, iHighlightedTS, iHighlightedT;

    bool bSingleGroup;
    std::vector<cTile *> *hSingleGroup;
    std::vector<cTile *> vtGroups[3];

    int MouseHandleGroup(std::vector<cTile *> tiles, int x, int y, int tilesPerRow);

    void RenderTileGroup(std::vector<cTile *> tiles, int x, int y);

public:
    winTileBrowser();

    ~winTileBrowser();

    virtual void Draw(int piCode);

    virtual void Think();

    virtual void OnDocumentChange();

    virtual void Open();

    virtual void Close();

    virtual void action(const ActionEvent &actionEvent);

    void Synchronize();
};

#endif // H_WIN_TILEBROWSER
