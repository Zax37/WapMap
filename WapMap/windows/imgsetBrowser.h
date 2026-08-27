#ifndef H_WIN_IMGSETBROWSER
#define H_WIN_IMGSETBROWSER

#include "../../shared/gcnwidgets/wLabel.h"
#include "../../shared/gcnwidgets/wButton.h"
#include "../../shared/gcnwidgets/wSlider.h"
#include "../../shared/gcnwidgets/wScrollArea.h"

#include "window.h"

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

    void action(const ActionEvent &actionEvent) override;

    void Synchronize();
};

#endif // H_WIN_TILEBROWSER
