#ifndef H_STATE_LOADMAP
#define H_STATE_LOADMAP

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wProgressBar.h"
#include "../../shared/cWWD.h"
#include "../../shared/cProgressInfo.h"
#include "../cLogicsList.h"
#include "guichan.hpp"
#include "../cDataController.h"
#include "../cParallelLoop.h"

namespace State {

 class LoadMap : public SHR::cState, public cParallelCallback {
  private:
   void * alt_ptr;
   int alt_size;
   bool alt_planes;
   int alt_width, alt_height;
   char alt_name[64], alt_author[64];
   void UpdateScene(){ Think(); _ForceRender(); };
  public:
   LoadMap(const char * pszFilename);
   LoadMap(void * ptr, int size, bool addplanes, int mw, int mh, char name[64], char author[64]);
   ~LoadMap();
   virtual bool Opaque();
   virtual void Init();
   virtual void Destroy();
   virtual bool Think();
   virtual bool Render();
   virtual void GainFocus(int iReturnCode, bool bFlipped);

   gcn::Gui * gui;
   SHR::Win * winLoad;
   SHR::ProgressBar * barWhole, * barAction;
   SHR::Lab * labWhole, * labAction, * labDesc;

   char * szDir, * szFilename, * szFilepath;

   virtual void ParallelTrigger();

   //int iGlobalProgress, iDetailedProgress, iDetailed

   //sMapData * md;
  //friend class LoadMapActionListener;
 };
};

#endif
