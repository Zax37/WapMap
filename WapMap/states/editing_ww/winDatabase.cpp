#include "../editing_ww.h"
#include "../../globals.h"
#include "../../conf.h"
#include "../error.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../cInterfaceSheet.h"
#include "../../databanks/imageSets.h"
extern HGE * hge;

void State::EditingWW::DrawLogicBrowser()
{

}

void State::EditingWW::SyncDB_ImageSets()
{
    sadbFramesImg->setVisible(dbAssetsImg->getSelected() != -1);
    if( dbAssetsImg->getSelected() != -1 ){
     if( dbFramesImg->getSelected() == -1 )
      dbFramesImg->setSelected(0);
     cSprBankAsset * as = SprBank->GetAssetByIterator(dbAssetsImg->getSelected());
     //cSprBankAssetIMG * img = as->GetIMGByIterator(dbFramesImg->getSelected());*/

     char tmp[256];
     sprintf(tmp, "~l~%s ~y~%d", GETL2S("WinDatabase", "FileCount"), as->GetSpritesCount());
     labdbisSetFileCount->setCaption(tmp);
     labdbisSetFileCount->adjustSize();

     char * fs = SHR::FormatSize(as->GetSize());
     sprintf(tmp, "~l~%s ~y~%s", GETL2S("WinDatabase", "SetSize"), fs);
     delete [] fs;
     labdbisSetFileSize->setCaption(tmp);
     labdbisSetFileSize->adjustSize();

     idbisMoveX = 0;
     idbisMoveY = 0;
     fdbisZoom = 1;
    }
}
