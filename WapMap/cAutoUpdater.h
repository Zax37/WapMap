#ifndef H_C_AUTOUPDATER
#define H_C_AUTOUPDATER

#include <windows.h>
#include <process.h>

#include "../shared/gcnWidgets/wContainer.h"
#include "../shared/gcnWidgets/wWin.h"
#include "../shared/gcnWidgets/wButton.h"
#include "../shared/gcnWidgets/wLink.h"
#include "../shared/gcnWidgets/wLabel.h"
#include "../shared/gcnWidgets/wContext.h"
#include "../shared/gcnWidgets/wProgressBar.h"
#include "wViewport.h"
#include <curl/curl.h>
#include <curl/easy.h>

#define AU_NONE 0
#define AU_SEARCHINGUPDATES 1
#define AU_DOWNLOADINGLIST 2
#define AU_DOWNLOADINGFILES 3

/*
cAutoUpdater checks whether there are any updates,
and if so, downloads them. Integrates interface
handling and all things done dirty way.
*/

class cAutoUpdater;

//internal action listener for gui
 class cAUAL: public gcn::ActionListener {
  private:
   cAutoUpdater * m_hOwn;
  public:
   void action(const gcn::ActionEvent &actionEvent);
   cAUAL(cAutoUpdater * owner);
 };

//internal viewport callback for gui
 class cAUVP: public WIDG::VpCallback {
  private:
   cAutoUpdater * m_hOwn;
  public:
   virtual void Draw(int iCode);
   cAUVP(cAutoUpdater * owner){ m_hOwn = owner; };
 };

class cAutoUpdater {
 friend class cAUVP;
 private:
  //search for updates only (embedded) or perform with gui (normal)? boolean flag
  bool bOnlyCheck;
  //for stats - overall files to download and already downloaded count
  int iDownloadFilesCount, iDownloadedFiles;
 public:
  std::vector<char*> vszFilesToDelete, vszFilesToUpdate;
  char * szUpdateList;
  float fTimeout;
  char * szUpdateInfo;
  int iState;
  bool bUpdates, bReady, bErrors, bKill;
  cAutoUpdater(bool checkonly = 0);
  ~cAutoUpdater();

  FILE * hPatchFile;

  float fDelay;

  CURL * newver_curl;
  CURLM * update_curlm;

  int iDownloaded, iTotal;

  cAUAL * hAL;
  cAUVP * hVP;
  SHR::Win * winActualize;
  SHR::Lab * labActualize;
  SHR::But * butYes, * butNo;
  SHR::ProgressBar * pbProgress;
  WIDG::Viewport * vpActualize;

  void PopupQuestion(SHR::Contener * dest);
  void TransformToDownload();
  void TransformToExit();

  void Update();
  void ParseUpdateList();
};

#endif
