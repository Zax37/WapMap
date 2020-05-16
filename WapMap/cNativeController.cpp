#include "cNativeController.h"

#include <cstdio>
#include <windows.h>
#include <hge.h>
#include "globals.h"

extern HGE *hge;

cNativeController::cNativeController() {
    bValid = 0;
    iVersion[0] = iVersion[1] = iVersion[2] = -1;
    iDisplayW = iDisplayH = -1;
    bGod = bArmor = bDebug = 0;
    strVersion = std::string("");
}

cNativeController::cNativeController(std::string strPath) {
    SetPath(strPath);
}

cNativeController::~cNativeController() {

}

bool cNativeController::IsValid() {
    return bValid;
}

bool cNativeController::SetPath(std::string strPath) {
    std::string exepath = strPath + "/claw.exe";
    FILE *f = fopen(exepath.c_str(), "rb");
    if (!f) {
        return 0;
    }
    fclose(f);

    bool obtained = 0;

    DWORD verHandle = NULL;
    UINT size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD verSize = GetFileVersionInfoSize(exepath.c_str(), &verHandle);
    if (verSize != NULL) {
        LPSTR verData = new char[verSize];
        if (GetFileVersionInfo(exepath.c_str(), verHandle, verSize, verData)) {
            if (VerQueryValue(verData, "\\", (VOID FAR *FAR *) &lpBuffer, &size)) {
                if (size) {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *) lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd) {
                        iVersion[0] = HIWORD(verInfo->dwFileVersionMS);
                        iVersion[1] = LOWORD(verInfo->dwFileVersionMS);
                        iVersion[2] = HIWORD(verInfo->dwFileVersionLS);
                        obtained = 1;
                    }
                }
            }
        }
        delete[] verData;
    }
    if (!obtained) return 0;

    strNativePath = strPath;
    char tmp[256];
    sprintf(tmp, "%d.%d.%d", iVersion[0], iVersion[1], iVersion[2]);
    strVersion = tmp;
    bValid = 1;

    //WIN32_FILE_ATTRIBUTE_DATA fileAttrData = {0};
    //if( GetFileAttributesEx(exepath.c_str(), GetFileExInfoStandard, &fileAttrData) == 0 ){
    ftExeModification.dwHighDateTime = ftExeModification.dwLowDateTime = 0;
    //}else
    // ftExeModification = fileAttrData.ftLastWriteTime;
    return 1;
}

int cNativeController::GetVersion(int iPart) {
    if (!bValid || iPart < 0 || iPart > 2) return -1;
    return iVersion[iPart];
}

bool cNativeController::IsCrazyHookAvailable() {
    return (bValid && iVersion[0] >= 1 && iVersion[1] >= 4);
}

bool cNativeController::ExecutableChanged() {
    if (!bValid) return 0;
    fExeCheckTimer += hge->Timer_GetDelta();
    if (fExeCheckTimer < 2.5f)
        return 0;
    fExeCheckTimer -= 2.5f;

    FILETIME nTime;

    std::string exepath = strNativePath + "/claw.exe";

    WIN32_FILE_ATTRIBUTE_DATA fileAttrData = {0};
    if (GetFileAttributesEx(exepath.c_str(), GetFileExInfoStandard, &fileAttrData) == 0)
        return 0;
    nTime = fileAttrData.ftLastWriteTime;
    bool change;
    if (ftExeModification.dwHighDateTime == 0 && ftExeModification.dwLowDateTime == 0)
        change = 1;
    else
        change = (CompareFileTime(&ftExeModification, &nTime) == -1);
    ftExeModification = nTime;
    return change;
}

void cNativeController::SetDisplayResolution(int w, int h) {
    iDisplayW = w;
    iDisplayH = h;
}

void cNativeController::SetGodMode(bool b) {
    bGod = b;
}

void cNativeController::SetArmorMode(bool b) {
    bArmor = b;
}

void cNativeController::SetDebugInfo(bool b) {
    bDebug = b;
}

void cNativeController::RunGame(std::string strMap, int iPosX, int iPosY) {
    if (game == WWD::Game_Unknown) return;

    std::string exePath = GV->gamePaths[game] + "\\";
    switch (game) {
        case WWD::Game_Claw:
            exePath += "CLAW.EXE";
            break;
        case WWD::Game_GetMedieval:
            exePath += "MEDIEVAL.EXE";
            break;
        case WWD::Game_Gruntz:
            exePath += "GRUNTZ.EXE";
            break;
        case WWD::Game_Claw2:
            exePath += "CLAW2.EXE";
            break;
        default:
            return;
    }

    std::string strParams;

    if (game == WWD::Game_Claw) {
        if (!bValid) return;
        GV->Console->Printf("Running map '~y~%s~w~'.", strMap.c_str());
        if (IsCrazyHookAvailable()) {
            strParams += "CL:\"" + strMap + "\"";
            char tmp[64];
            if (iDisplayW != -1 && iDisplayH != -1) {
                sprintf(tmp, "%dx%d", iDisplayW, iDisplayH);
                strParams += " RES:\"" + std::string(tmp) + "\"";
            }
            if (iPosX != -1 && iPosY != -1) {
                sprintf(tmp, "%dx%d", iPosX, iPosY);
                strParams += " POS:\"" + std::string(tmp) + "\"";
                strParams += " -NOALIGN";
            }
            if (bDebug) strParams += " -DEBUGINFOS";
            if (bGod) strParams += " -GOD";
            if (bArmor) strParams += " -ARMOR";
        }
        GV->Console->Printf("Command line is: '~y~%s~w~'.", strParams.c_str());
    }

    ShellExecute(0, "open", exePath.c_str(), (strParams.length() > 0 ? strParams.c_str() : 0), GV->gamePaths[game].c_str(), SW_SHOWNORMAL);
}

void cNativeController::GetDisplayResolution(int *w, int *h) {
    *w = iDisplayW;
    *h = iDisplayH;
}

void cNativeController::SyncDocumentSwitched(WWD::Parser* hParser) {
    if (hParser) {
        game = hParser->GetGame();
    } else {
        game = WWD::Game_Unknown;
    }
}
