#ifndef H_WIN_IMGSETBROWSER
#define H_WIN_IMGSETBROWSER

#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wScrollArea.h"

#include "window.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "assetBrowserUi.h"
#endif

class cSprBankAssetIMG;

class winImageSetBrowser : public cWindow {
private:
    SHR::Lab *labImageSets, *labImageSetName, *labImageSetChecksum, *labFrameCount,
            *labImageSetNameV, *labImageSetChecksumV, *labFrameCountV,
            *labImageFileName, *labImagePath, *labImageDim,
            *labImageFileNameV, *labImagePathV, *labImageDimV;
    SHR::ScrollArea *saFrames, *saImageSets;
    SHR::Container *conFrames, *conImageSets;
    WIDG::Viewport *vp;
    SHR::But *butImportImageSet, *butDeleteImageSet, *butRenameImageSet, *butBrowseFolder, *butDeleteFrame, *butChangeFrameID, *butImageDetails,
            *butAddFrames;
    SHR::Lab *labFrameID, *labFrameModDate, *labFrameChecksum, *labFrameOrigin,
            *labFrameIDV, *labFrameModDateV, *labFrameChecksumV, *labFrameOriginV;
    int iSelectedImageSet, iSelectedFrame, iHighlightedIS, iHighlightedF;
#ifdef WAPMAP_ENABLE_IMGUI
    AssetBrowserUi::State native;
    void ThinkNative();
    void DrawImGui() override;
#endif

    bool bSingleGroup;
    std::vector<cSprBankAssetIMG *> *hSingleGroup;
    std::vector<cSprBankAssetIMG *> vtGroups[3];

    int MouseHandleGroup(std::vector<cSprBankAssetIMG *> tiles, int x, int y, int tilesPerRow);

    void RenderFrameGroup(std::vector<cSprBankAssetIMG *> tiles, int x, int y);

public:
    winImageSetBrowser();

    void Draw(int piCode) override;

    void Think() override;

    void OnDocumentChange() override;

    void Open() override;
    void Close() override;
#ifdef WAPMAP_ENABLE_IMGUI
    bool isOpened() override { return native.open; }
#endif

    void action(const ActionEvent &actionEvent) override;

    void Synchronize();
};

#endif // H_WIN_TILEBROWSER
