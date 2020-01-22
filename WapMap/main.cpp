#include "hge.h"
#include "conf.h"
#include "globals.h"
#include "states/editing_ww.h"
#include "conf.h"
#include "../shared/commonFunc.h"
#include <windows.h>
#include <tlhelp32.h>
#include "cControllerIPC.h"

#include <hgeVector.h>
#include <direct.h>

HGE *hge = 0;

extern bool _UpdaterThink();

bool FrameFunc() {
    //return 0;
    return GV->StateMgr->Think();
}

bool RenderFunc() {
    return GV->StateMgr->Render();
}

bool ExitFunc() {
    return GV->StateMgr->ExitFunc();
}

bool AppWindowNotifyFunc() {
    GV->StateMgr->OS_Notify();
    return 1;
}

bool GfxRestoreFunc() {
    GV->StateMgr->GfxRestore();
    return 1;
}

bool FileDroppedFunc() {
    GV->StateMgr->FileDropped();
    return 1;
}

bool AppFocusOnFunc() {
    GV->StateMgr->AppFocus(1);
    return 0;
}

bool AppFocusOffFunc() {
    GV->StateMgr->AppFocus(0);
    return 0;
}

int main(int argc, char *argv[]) {
    std::string command;
    for (int i = 1; i < argc; i++) {
        command += argv[i];

        if (i != argc - 1) //this check prevents adding a space after last argument.
            command += " ";
    }

    return WinMain(NULL, NULL, const_cast<char *>(command.c_str()), 0);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int) {
    char tmp[768];
    ::GetModuleFileName(NULL, tmp, 768);

#ifndef BUILD_DEBUG
    char *fn = SHR::GetFile(tmp);
    if (strcmp(fn, EXENAME) != 0) {
        MessageBox(NULL, "This installation of WM is broken. Please download and install fresh version.", "WapMap",
                   MB_OK | MB_ICONERROR);
        return 1;
    }
    delete[] fn;
#endif

    if (cmdline[0] == '"') {
        cClientIPC *hIPC = new cClientIPC();
        if (hIPC->IsConnected()) {
            char* tmp = new char[strlen(cmdline) - 1];
            for (int i = 1; i < strlen(cmdline) - 1; i++)
                tmp[i - 1] = cmdline[i];
            if (hIPC->RemoteOpenMap(tmp)) {
                delete hIPC;
                return 0;
            }
			delete[] tmp;
        }
        delete hIPC;
    }

    DWORD procid = 0;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    int refcnt = 0;
    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (strcmpi(entry.szExeFile, EXENAME) == 0) {
                refcnt++;
                procid = entry.th32ProcessID;
                if (refcnt == 2) {
                    MessageBox(NULL, "WapMap is already running.", "WapMap", MB_OK | MB_ICONERROR);
                    return 1;
                }
            }
        }
    }
    CloseHandle(snapshot);

    char *myDIR = SHR::GetDir(tmp);
    _chdir(myDIR);
    delete[] myDIR;

    HMODULE hmod = LoadLibrary("SciLexer.dll");
    if (hmod == NULL) {
        MessageBox(0, "Could not load Scintilla library.", "WapMap", MB_OK | MB_ICONERROR);
        return 0xBAADBAAD;
    }

    GV = new cGlobals();
    GV->dwProcID = procid;
    GV->szCmdLine = std::string(cmdline);
    GV->Console->Printf(GV->szCmdLine.length() > 0 ? "~w~CMD is `~y~%s~w~`." : "~w~CMD is empty.", cmdline);
    GV->Console->Printf("~w~Process ID: ~y~%ld", procid);

    hge = hgeCreate(HGE_VERSION);
#ifdef BUILD_DEBUG
    hge->System_SetState(HGE_LOGFILE, "log.txt");
#endif
    hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
    hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
    hge->System_SetState(HGE_EXITFUNC, ExitFunc);
    hge->System_SetState(HGE_GFXRESTOREFUNC, GfxRestoreFunc);
    hge->System_SetState(HGE_FILEDROPFUNC, FileDroppedFunc);
    hge->System_SetState(HGE_FOCUSLOSTFUNC, AppFocusOffFunc);
    hge->System_SetState(HGE_FOCUSGAINFUNC, AppFocusOnFunc);
    hge->System_SetState(HGE_WMNOTIFYFUNC, AppWindowNotifyFunc);
    if (!strncmp(cmdline, "-updateBM", 9)) {
        hge->System_SetState(HGE_SCREENWIDTH, 320);
        hge->System_SetState(HGE_SCREENHEIGHT, 140);
    } else {
        hge->System_SetState(HGE_SCREENWIDTH, GV->iScreenW);
        hge->System_SetState(HGE_SCREENHEIGHT, GV->iScreenH);
    }
    hge->System_SetState(HGE_ACCEPTFILES, 1);
    hge->System_SetState(HGE_TITLE, WA_TITLEBAR);
    hge->System_SetState(HGE_WINDOWED, 1);
    hge->System_SetState(HGE_USESOUND, 1);
    hge->System_SetState(HGE_SHOWSPLASH, 0);
    hge->System_SetState(HGE_DONTSUSPEND, 1);
    hge->System_SetState(HGE_ICON, MAKEINTRESOURCE(100));
    hge->System_SetState(HGE_TEXTUREFILTER, 0);
    //hge->System_SetState(HGE_FPS, 60);

    //if( !strncmp(cmdline, "-updateBM", 9) )
    hge->System_SetState(HGE_WINDOWCAPTION, 0);

    int left = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXFIXEDFRAME) + 1;
    int top = GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION) + 1;

    if (hge->System_Initiate()) {
        HWND hwnd = hge->System_GetState(HGE_HWND);
        HRGN rgn = CreateRectRgn(left, top, left + GV->iScreenW, top + GV->iScreenH);
        SetWindowRgn(hwnd, rgn, true);

        GV->Init();
        GV->StateMgr->Init();

        GV->StateMgr->Push(new State::EditingWW());
        hge->System_Start();
    } else {
        MessageBox(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
    GV->Console->Printf("~w~Application shutdown.");
    delete GV;
    hge->System_Shutdown();
    hge->Release();
    return 0;
}
