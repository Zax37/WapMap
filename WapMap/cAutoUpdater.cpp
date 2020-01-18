#include "cAutoUpdater.h"
#include "../shared/curl/curl.h"
#include "../shared/curl/types.h"
#include "../shared/curl/easy.h"
#include "conf.h"
#include "globals.h"
#include "langID.h"
#include "../shared/commonFunc.h"
#include "states/editing_ww.h"
#include "../shared/HashLib/hashlibpp.h"

#include <direct.h>

extern HGE * hge;

cAutoUpdater * _AU_GLOBAL_PTR;

void cAUAL::action(const gcn::ActionEvent &actionEvent)
{
    if( actionEvent.getSource() == m_hOwn->butYes ){
     /*if( m_hOwn->bPatchReady ){
      GV->Console->Print("Terminating process and executing patch...");
      ShellExecute(NULL, "open", "Patch.exe", "", "", SW_SHOWNORMAL);
      exit(666);
     }else*/
      m_hOwn->TransformToDownload();
    }else if( actionEvent.getSource() == m_hOwn->butNo ){
     m_hOwn->bKill = 1;
    }
}

cAUAL::cAUAL(cAutoUpdater * owner)
{
    m_hOwn = owner;
}

void cAUVP::Draw(int iCode)
{
    if( m_hOwn->iTotal != 0 ){
     m_hOwn->pbProgress->setEnd(m_hOwn->iTotal);
     m_hOwn->pbProgress->setProgress(m_hOwn->iDownloaded);
    }
    if( m_hOwn->iState == AU_DOWNLOADINGFILES && m_hOwn->vszFilesToUpdate.size() != 0 ){
     char tmp[256];
     char * f1 = SHR::FormatSize(m_hOwn->iDownloaded), * f2 = SHR::FormatSize(m_hOwn->iTotal);
     sprintf(tmp, "%s... %.2f%% [%s/%s] (%d/%d)", m_hOwn->vszFilesToUpdate[0], m_hOwn->pbProgress->getPercentageProgress(), f1, f2, m_hOwn->iDownloadFilesCount-m_hOwn->vszFilesToUpdate.size()+1, m_hOwn->iDownloadFilesCount);
     delete [] f1;
     delete [] f2;
     m_hOwn->labActualize->setCaption(tmp);
     m_hOwn->labActualize->adjustSize();
    }
}

static size_t _AU_ListCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    int len = size*nmemb, offset = 0;
    if( _AU_GLOBAL_PTR->szUpdateList != NULL ){
     offset = strlen(_AU_GLOBAL_PTR->szUpdateList);
     len += offset;
    }
    char * tmp = new char[len+1];
    tmp[len] = '\0';
    for(int i=0;i<offset;i++)
     tmp[i] = _AU_GLOBAL_PTR->szUpdateList[i];
    for(int i=offset;i<len;i++)
     tmp[i] = ((char*)ptr)[i-offset];
    if( _AU_GLOBAL_PTR->szUpdateList != NULL )
     delete _AU_GLOBAL_PTR->szUpdateList;
    _AU_GLOBAL_PTR->szUpdateList = tmp;
    return (size_t)(size * nmemb);
}

static int _AU_PatchProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    _AU_GLOBAL_PTR->iDownloaded = dlnow;
    _AU_GLOBAL_PTR->iTotal = dltotal;
    return 0;
}

static size_t _AU_PatchCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    //Herculo.Log(LOG_INFO, "got %d bytes", size * nmemb);
    fwrite(ptr, size, nmemb, _AU_GLOBAL_PTR->hPatchFile);
    return (size_t)(size * nmemb);
}

static size_t _AU_VerCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    char temp[nmemb+1];
    strncpy(temp, (char*)ptr, nmemb);
    temp[nmemb] = '\0';

    char * line = strtok(temp, "\n");
    while( line != NULL ){
     if( !strncmp(line, "update", 6) && GV->bAutoUpdate ){
      if( line[7] == '0' ){
       _AU_GLOBAL_PTR->bUpdates = 0;
       GV->Console->Print("No updates found.");
      }else if( line[7] == '1' ){
       _AU_GLOBAL_PTR->bUpdates = 1;
       if( strlen(line) == 8 ){
        _AU_GLOBAL_PTR->szUpdateInfo = NULL;
       }else{
        char * tmp = new char[strlen(line)-8+1];
        for(int i=8;i<strlen(line);i++)
         tmp[i-8] = line[i];
        tmp[strlen(line)-8] = '\0';
        _AU_GLOBAL_PTR->szUpdateInfo = SHR::Replace(tmp, "~n~", "\n");
        delete tmp;
       }
       GV->Console->Printf("~g~Found update.");
      }else{
       GV->Console->Print("~r~Unknown update response.");
       _AU_GLOBAL_PTR->bErrors = 1;
      }
     }else if( !strncmp(line, "serial", 6) ){
      GV->szSerial = new char[32];
      char * serialptr = line+7;
      strncpy(GV->szSerial, serialptr, 30);
      GV->szSerial[31] = '\0';
      HKEY key;
      if( RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Kijedi Studio\\WapMap", 0, KEY_SET_VALUE, &key) == ERROR_SUCCESS ){
       DWORD size = 32;
       RegSetValueEx(key, "UID", 0, REG_SZ, (BYTE*)GV->szSerial, size);
       RegCloseKey(key);
       GV->Console->Printf("~g~UID obtained: ~y~%s", GV->szSerial);
      }else{
       GV->Console->Printf("~r~UID obtained, unable to save (key open fail).");
      }
     }else if( !strncmp(line, "message", 7) && GV->editState != NULL ){
      int iMsgID, iTitleLen;
      sscanf(line, "message %d %d %*s", &iMsgID, &iTitleLen);

      //GV->Console->Printf("Received message ID ~y~%d~w~ (title: ~y~%s~w~).", iMsgID, GV->editState->szServerMsgTitle);
     }else{
      GV->Console->Printf("~r~Unknown response from update server [~y~%s~r~].", line);
     }
     line = strtok(NULL, "\n");
    }
    _AU_GLOBAL_PTR->bReady = 1;
    return (size_t)(size * nmemb);
}

cAutoUpdater::cAutoUpdater(bool checkonly)
{
    hPatchFile = NULL;
    szUpdateList = 0;
    iState = AU_NONE;
    _AU_GLOBAL_PTR = this;
    szUpdateInfo = NULL;
    bUpdates = bReady = bKill = bErrors = 0;
    winActualize = NULL;
    hAL = NULL;
    hVP = NULL;
    bOnlyCheck = checkonly;
    vpActualize = NULL;
    labActualize = NULL;
    butYes = butNo = NULL;

    RECT DesktopRect;
    GetWindowRect(GetDesktopWindow(), &DesktopRect);

    char client[25+64];
    sprintf(client, "WapMap/%d/%s,%dx%d,%dx%d/%s",
            WA_VER,
            GV->Lang->GetCode(), hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT), DesktopRect.right, DesktopRect.bottom,
            GV->szSerial);
    newver_curl = curl_easy_init();
    update_curlm = curl_multi_init();
    char adress[256];
    #ifdef BUILD_DEBUG
     sprintf(adress, "http://%s/update/checkver.php?debug=1", GV->szUpdateServer);
    #else
     sprintf(adress, "http://%s/update/checkver.php", GV->szUpdateServer);
    #endif
    GV->Console->Printf("Checking for updates on ~y~%s~w~...", GV->szUpdateServer);
    curl_easy_setopt(newver_curl, CURLOPT_URL, adress);
    curl_easy_setopt(newver_curl, CURLOPT_USERAGENT, client);
    curl_easy_setopt(newver_curl, CURLOPT_WRITEFUNCTION, _AU_VerCallback);
    curl_multi_add_handle(update_curlm, newver_curl);

    long tm;
    curl_multi_timeout(update_curlm, &tm);
    fDelay = tm/1000;
    iState = AU_SEARCHINGUPDATES;
}

cAutoUpdater::~cAutoUpdater()
{
    if( winActualize != NULL ){
     delete butYes, butNo;
     delete labActualize;
     delete winActualize;
     delete hAL;
     delete vpActualize;
     delete hVP;
    }
    if( szUpdateInfo != NULL )
     delete szUpdateInfo;
}

void cAutoUpdater::PopupQuestion(SHR::Contener * dest)
{
    /*if( iProtocolVersion == 2 ){
     winActualize = new SHR::Win(&GV->gcnParts, "Aktualizacja");
     winActualize->setDimension(gcn::Rectangle(0, 0, 300, 150));
     dest->add(winActualize, hge->System_GetState(HGE_SCREENWIDTH)/2-150, hge->System_GetState(HGE_SCREENHEIGHT)/2-75);
     GV->editState->FakeRender();
    }*/
    hAL = new cAUAL(this);
    if( winActualize == NULL )
     winActualize = new SHR::Win(&GV->gcnParts, GETL(Lang_ActualizeCaption));
    else
     winActualize->setCaption(GETL(Lang_ActualizeCaption));
    char tmp[512];
    sprintf(tmp, "%s\n%s", szUpdateInfo, GETL(Lang_ActualizeQuestion));
    winActualize->setDimension(gcn::Rectangle(0, 0, 400, 60+GV->fntMyriad13->GetHeightb(390, tmp)));
    if( labActualize == NULL )
     labActualize = new SHR::Lab(tmp);
    else
     labActualize->setCaption(tmp);
    labActualize->adjustSize();
    labActualize->setHeight(winActualize->getHeight()-60);
    winActualize->add(labActualize, 5, 10);
    dest->add(winActualize, hge->System_GetState(HGE_SCREENWIDTH)/2-200, hge->System_GetState(HGE_SCREENHEIGHT)/2-winActualize->getHeight()/2);

    butYes = new SHR::But(GV->hGfxInterface, GETL(Lang_Yes));
    butYes->setDimension(gcn::Rectangle(0, 0, 100, 32));
    butYes->addActionListener(hAL);
    winActualize->add(butYes, 75, winActualize->getHeight()-50);

    butNo = new SHR::But(GV->hGfxInterface, GETL(Lang_No));
    butNo->setDimension(gcn::Rectangle(0, 0, 100, 32));
    butNo->addActionListener(hAL);
    winActualize->add(butNo, 225, winActualize->getHeight()-50);
}

void cAutoUpdater::TransformToExit()
{
    pbProgress->setVisible(0);
    butNo->setVisible(0);

    butYes->setCaption(GETL(Lang_OK));
    butYes->setX(150);
    butYes->setVisible(1);

    labActualize->setCaption(GETL(Lang_PatchRestart));
    labActualize->setWidth(380);
    labActualize->setHeight(100);
}

void cAutoUpdater::TransformToDownload()
{
    butYes->setVisible(0);
    labActualize->setCaption(GETL(Lang_DownloadingUpdate));
    labActualize->adjustSize();
    butNo->setX(150);
    butNo->setCaption(GETL(Lang_Cancel));
    pbProgress = new SHR::ProgressBar(&GV->gcnParts);
    pbProgress->setDimension(gcn::Rectangle(0, 0, 380, 31));
    pbProgress->setProgress(0);
    pbProgress->setEnd(10);
    winActualize->add(pbProgress, 10, 50);

    hVP = new cAUVP(this);

    vpActualize = new WIDG::Viewport(hVP, 0);
    winActualize->add(vpActualize, 5, 5);

    newver_curl = curl_easy_init();
    char path[strlen(GV->szUpdateServer)+32];
    sprintf(path, "http://%s/update/updateScript.usc", GV->szUpdateServer);
    GV->Console->Printf("Downloading update list...", path);
    curl_easy_setopt(newver_curl, CURLOPT_URL, path);
    char client[25];
    sprintf(client, "WapMap/%d", WA_VER);
    curl_easy_setopt(newver_curl, CURLOPT_USERAGENT, client);
    curl_easy_setopt(newver_curl, CURLOPT_WRITEFUNCTION, _AU_ListCallback);
    update_curlm = curl_multi_init();
    curl_multi_add_handle(update_curlm, newver_curl);

    long tm;
    curl_multi_timeout(update_curlm, &tm);
    fDelay = tm/1000.0f;
    iState = AU_DOWNLOADINGLIST;
}

void cAutoUpdater::ParseUpdateList()
{
    hashwrapper * myWrapper = new md5wrapper();
    char line[512];
    char * tmpptr = szUpdateList;
    while( tmpptr = SHR::GetLine(tmpptr, line) ){
     if( line[0] == '/' && line[1] == '/' || strlen(line) < 4 ) continue;
     if( !strncmp(line, "delete", 6) ){
      char path[256];
      for(int i=0;i<256;i++) path[i] = '\0';
      for(int i=0;i<256&&i<strlen(line)-7;i++)
       path[i] = line[i+7];
      char * pathptr = new char[strlen(path)+1];
      strcpy(pathptr, path);
      vszFilesToDelete.push_back(pathptr);
      GV->Console->Printf("Delete request: ~y~%s", path);
     }else if( !strncmp(line, "force", 5) ){
      char path[256];
      for(int i=0;i<256;i++) path[i] = '\0';
      for(int i=0;i<256&&i<strlen(line)-6;i++)
       path[i] = line[i+6];
      char * pathptr = new char[strlen(path)+1];
      strcpy(pathptr, path);
      vszFilesToUpdate.push_back(pathptr);
      GV->Console->Printf("Forced request: ~y~%s", path);
     }else{
      char path[256], hash[100];
      for(int i=0;i<100;i++) hash[i] = '\0';
      for(int i=0;(i<strlen(line)&&i<256);i++){
       if( line[i] == ':' ){
        path[i] = '\0';
        break;
       }else
        path[i] = line[i];
      }
      for(int i=0;i<100&&i+strlen(path)+2<strlen(line);i++){
       hash[i] = line[i+strlen(path)+2];
      }
      const char * myhash;
      bool bError = 0;
      try{
       myhash = myWrapper->getHashFromFile(path).c_str();
      }catch( hlException exc ){
       bError = 1;
      }
      if( !bError ){
       if( strcmp(myhash, hash) != 0 ){
        GV->Console->Printf("~w~File ~y~%s ~w~differs!", path);
        char * pathptr = new char[strlen(path)+1];
        strcpy(pathptr, path);
        vszFilesToUpdate.push_back(pathptr);
       }else{
        GV->Console->Printf("~g~File ~y~%s ~g~up to date.", path);
       }
      }else{
       GV->Console->Printf("~r~File ~y~%s ~r~unable to check.", path);
       char * pathptr = new char[strlen(path)+1];
       strcpy(pathptr, path);
       vszFilesToUpdate.push_back(pathptr);
      }
     }
    }
    delete myWrapper;
}

void cAutoUpdater::Update()
{
    //delay for curl logic
    if( fDelay > 0 ){
     fDelay -= hge->Timer_GetDelta();
     return;
    }

    if( update_curlm == NULL && iState == AU_DOWNLOADINGFILES ){
     //if hPatchFile isnt null it means we already downloaded previous file
     //so we pop that file from downloading queue and close handle
     if( hPatchFile != NULL ){
      fclose(hPatchFile);
      hPatchFile = NULL;
      delete vszFilesToUpdate[0];
      vszFilesToUpdate.erase(vszFilesToUpdate.begin());
     }

     //done downloading files
     if( vszFilesToUpdate.size() == 0 ){
      int iDeletedFiles = 0;
      //delete files from deletion queue
      for(int i=0;i<vszFilesToDelete.size();i++){
       FILE * f = fopen(vszFilesToDelete[i], "wb+");
       if( f != NULL ){
        fclose(f);
        unlink(vszFilesToDelete[i]);
        iDeletedFiles++;
       }else{
        GV->Console->Printf("~r~Unable to delete file ~y~%s~r~.", vszFilesToDelete[i]);
       }
       delete vszFilesToDelete[i];
      }
      vszFilesToDelete.clear();
      GV->Console->Printf("~y~%d ~g~files deleted in update process.", iDeletedFiles);
      GV->Console->Printf("~g~All files downloaded, terminating process and executing updater...");
      //run updater executable and quit
      char tmp[MAX_PATH];
      _getcwd(tmp, MAX_PATH);
      printf("CWD: %s\n", tmp);
      FILE * f = fopen("updatetmp/Updater.exe", "r");
      if( f ){
       printf("file ok\n");
       fclose(f);
      }
      char abspath[strlen(tmp)+32];
      sprintf(abspath, "%s\\updatetmp\\Updater.exe", tmp);
      HINSTANCE err = ShellExecute(NULL, "open", abspath, "-updateWM", NULL, SW_SHOWNORMAL);
      if( err <= (HINSTANCE)32 ){
       printf("exec fail %d for %s\n", (int)err, abspath);
       printf("%d\n", (int)ERROR_FILE_NOT_FOUND);
       printf("%d\n", (int)ERROR_PATH_NOT_FOUND);
       printf("%d\n", (int)ERROR_BAD_FORMAT);
       printf("%d\n", (int)SE_ERR_ACCESSDENIED);
       printf("%d\n", (int)SE_ERR_ASSOCINCOMPLETE);
       printf("%d\n", (int)SE_ERR_DDEBUSY);
      }
      _sleep(500);
      exit(666);
     }

     char * newf = vszFilesToUpdate[0];

     //create directory tree for actual file
     chdir("updatetmp");
     int ilvl = 0, dirp = 0;
     char dir[strlen(newf)+1];
     for(int i=0;i<strlen(newf);i++){
      if( newf[i] == '/' || newf[i] == '\\' ){
       dir[dirp] = '\0';
       dirp = 0;
       mkdir(dir);
       chdir(dir);
       ilvl++;
       continue;
      }
      dir[dirp] = newf[i];
      dirp++;
     }
     for(int i=0;i<ilvl;i++)
      chdir("..");

     char mynewf[256];
     char * ext = SHR::GetExtension(newf);
     if( !strcmp(ext, "rar") ){
      char * file = SHR::GetFileWithoutExt(newf);
      sprintf(mynewf, "%s.exe", file);
      delete [] file;
     }else{
      strcpy(mynewf, newf);
     }
     delete [] ext;

     hPatchFile = fopen(mynewf, "wb");
     chdir("..");

     newver_curl = curl_easy_init();
     char path[strlen(GV->szUpdateServer)+21];
     sprintf(path, "http://%s/update/repo/%s", GV->szUpdateServer, newf);
     GV->Console->Printf("Downloading file ~y~%s~w~...", mynewf);
     curl_easy_setopt(newver_curl, CURLOPT_URL, path);
     char client[25];
     sprintf(client, "WapMap/%d", WA_VER);
     curl_easy_setopt(newver_curl, CURLOPT_USERAGENT, client);
     curl_easy_setopt(newver_curl, CURLOPT_WRITEFUNCTION, _AU_PatchCallback);
     curl_easy_setopt(newver_curl, CURLOPT_PROGRESSFUNCTION, _AU_PatchProgressCallback);
     curl_easy_setopt(newver_curl, CURLOPT_NOPROGRESS, 0);
     update_curlm = curl_multi_init();
     curl_multi_add_handle(update_curlm, newver_curl);

     long tm;
     curl_multi_timeout(update_curlm, &tm);
     fDelay = tm/1000.0f;
     return;
    }else if( update_curlm != NULL ){
     int alive;
     curl_multi_perform(update_curlm, &alive);

     if( !alive ){
      curl_multi_remove_handle(update_curlm, newver_curl);
      curl_easy_cleanup(newver_curl);
      curl_multi_cleanup(update_curlm);
      update_curlm = NULL;
      if( iState == AU_DOWNLOADINGLIST ){
       if( szUpdateList == NULL || strlen(szUpdateList) == 0 ){
        GV->Console->Print("~r~Failed downloading update list.");
        iState = AU_NONE;
       }else{
        GV->Console->Printf("~g~List downloaded [~y~%d ~g~bytes].", strlen(szUpdateList));
        ParseUpdateList();
        if( vszFilesToUpdate.size() == 0 ){
         GV->Console->Print("No files to update.");
        }else{
         GV->Console->Printf("~y~%d ~w~files to update. Proceeding to download...", vszFilesToUpdate.size());
         iDownloadFilesCount = vszFilesToUpdate.size()+1;
         iState = AU_DOWNLOADINGFILES;
         mkdir("updatetmp");
         char * t = new char[12];
         strcpy(t, "Updater.rar");
         vszFilesToUpdate.push_back(t);
        }
       }
      }
      /*if( bDownloading ){
       fclose(hPatchFile);
       bPatchReady = 1;
       if( rename("patch.cache", "Patch.exe") != 0 ){
        unlink("Patch.exe");
        rename("patch.cache", "Patch.exe");
       }
       char * str = SHR::FormatSize(iTotal);
       GV->Console->Printf("~g~Patch downloaded (%s).", str);
       delete [] str;
      }else*/ if( !bReady )
       bErrors = 1;
     }
    }

    if( bReady ){
     if( bUpdates ){
      if( winActualize == NULL && !bOnlyCheck )
       PopupQuestion(GV->editState->conMain);
     }else{
      bKill = 1;
     }
     /*if( bDownloading && bPatchReady ){
      TransformToExit();
     }*/
    }
}
