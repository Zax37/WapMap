#ifndef H_WIN_IMGSETBROWSER
#define H_WIN_IMGSETBROWSER
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wScrollArea.h"

#include "window.h"
class cSprBankAssetIMG;

class winImageSetBrowser : public cWindow
{
 private:
  SHR::Lab * labImageSets, * labImageSetName, * labImageSetChecksum, * labFrameCount,
           * labImageSetNameV, * labImageSetChecksumV, * labFrameCountV,
           * labImageFileName, * labImagePath, * labImageDim,
           * labImageFileNameV, * labImagePathV, * labImageDimV;
  SHR::ScrollArea * saFrames, * saImageSets;
  SHR::Contener * conFrames, * conImageSets;
  WIDG::Viewport * vp;
  SHR::But * butImportImageSet, * butDeleteImageSet, * butRenameImageSet, * butBrowseFolder, * butDeleteFrame, * butChangeFrameID, * butImageDetails,
           * butAddFrames;
  SHR::Lab * labFrameID, * labFrameModDate, * labFrameChecksum, * labFrameOrigin,
           * labFrameIDV, * labFrameModDateV, * labFrameChecksumV, * labFrameOriginV;
  int iSelectedImageSet, iSelectedFrame, iHighlightedIS, iHighlightedF;

  bool bSingleGroup;
  std::vector<cSprBankAssetIMG*> * hSingleGroup;
  std::vector<cSprBankAssetIMG*> vtGroups[3];

  int MouseHandleGroup(std::vector<cSprBankAssetIMG*> tiles, int x, int y, int tilesPerRow);
  void RenderFrameGroup(std::vector<cSprBankAssetIMG*> tiles, int x, int y);
 public:
  winImageSetBrowser();
  ~winImageSetBrowser();

  virtual void Draw(int piCode);
  virtual void Think();
  virtual void OnDocumentChange();
  virtual void Open();
  virtual void Close();
  virtual void action(const ActionEvent& actionEvent);

  void Synchronize();
};

#endif // H_WIN_TILEBROWSER
