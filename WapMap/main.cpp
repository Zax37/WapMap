#include "hge.h"
#include "version.h"
#include "globals.h"
#include "states/editing_ww.h"
#include "cDataController.h"
#include "../shared/commonFunc.h"
#include <tlhelp32.h>
#include <direct.h>

HGE *hge = 0;

bool FrameFunc() {
    if (GV->StateMgr->Think())
        return true;

    if (GV->editState->hAU) {
        if (GV->editState->hAU->Think()) {
            delete GV->editState->hAU;
            GV->editState->hAU = 0;
        }
    }

    if (GV->anyMapLoaded && GV->editState->hDataCtrl) {
        GV->editState->hDataCtrl->Think();
    }

    return false;
}

bool RenderFunc() {
    return GV->StateMgr->Render();
}

bool ExitFunc() {
    return GV->StateMgr->ExitFunc();
}

bool GfxRestoreFunc() {
    GV->StateMgr->GfxRestore();
    return 1;
}

bool FileDraggedInFunc() {
    GV->StateMgr->FileDraggedIn();
    return 1;
}

bool FileDraggedOutFunc() {
    GV->StateMgr->FileDraggedOut();
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

bool AppResizeFunc() {
    GV->editState->OnResize();
    return 0;
}

bool AppToggleFullscreenFunc() {
    GV->editState->ToggleFullscreen();
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

    remove(".wm_update.exe");

	// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.wwd ?

	HKEY hkey;
	std::string extension = ".wwd";
	const char* desc = "WAP32 Map Document";
	std::string app = std::string(tmp) + " %1";
	const char* action = "Edit with WapMap";

	std::string path = extension +
		"\\shell\\" +
		action +
		"\\"
		"command\\";

	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, extension.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkey, "", 0, REG_SZ, (BYTE*)desc, strlen(desc));
		RegCloseKey(hkey);
	}

	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, path.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, 0) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkey, "", 0, REG_SZ, (BYTE*)app.c_str(), app.length());
		RegCloseKey(hkey);
	}

#ifndef BUILD_DEBUG
    char *fn = SHR::GetFile(tmp);
    if (strcmp(fn, EXENAME) != 0) {
        MessageBox(NULL, "This installation of WapMap is broken. Please download and install fresh version.",
                   PRODUCT_NAME, MB_OK | MB_ICONERROR);
        return 1;
    }
    delete[] fn;
#endif
    DWORD procid = GetCurrentProcessId();
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    int refcnt = 0;
    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (strcmpi(entry.szExeFile, EXENAME) == 0) {
                if (procid != entry.th32ProcessID) {
                    for (int retries = 0; retries <= 2; retries++) {
                        if (cmdline[0] && cmdline[0] != '-') {
                            cClientIPC *hIPC = new cClientIPC();
                            if (hIPC->IsConnected()) {
                                if (hIPC->RemoteOpenMap(cmdline)) {
                                    delete hIPC;
                                    return 0;
                                }
                            }
                            delete hIPC;
                        }

                        _sleep(2000);
                    }

                    MessageBox(NULL, "WapMap is already running.", PRODUCT_NAME, MB_OK | MB_ICONERROR);
                    return 1;
                } else break;
            }
        }
    }
    CloseHandle(snapshot);

    char *myDIR = SHR::GetDir(tmp);
    _chdir(myDIR);
    delete[] myDIR;

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
    hge->System_SetState(HGE_FILEDRAGINFUNC, FileDraggedInFunc);
    hge->System_SetState(HGE_FILEDRAGOUTFUNC, FileDraggedOutFunc);
    hge->System_SetState(HGE_FILEDROPFUNC, FileDroppedFunc);
    hge->System_SetState(HGE_FOCUSLOSTFUNC, AppFocusOffFunc);
    hge->System_SetState(HGE_FOCUSGAINFUNC, AppFocusOnFunc);
    hge->System_SetState(HGE_RESIZEFUNC, AppResizeFunc);
    hge->System_SetState(HGE_FULLSCREENTOGGLEFUNC, AppToggleFullscreenFunc);
    hge->System_SetState(HGE_SCREENWIDTH, GV->iScreenW);
    hge->System_SetState(HGE_SCREENHEIGHT, GV->iScreenH);
    hge->System_SetState(HGE_ACCEPTFILES, true);
    hge->System_SetState(HGE_TITLE, WA_TITLEBAR);
    hge->System_SetState(HGE_WINDOWED, true);
    hge->System_SetState(HGE_USESOUND, true);
    hge->System_SetState(HGE_SHOWSPLASH, false);
    hge->System_SetState(HGE_DONTSUSPEND, true);
    hge->System_SetState(HGE_ICON, MAKEINTRESOURCE(100));
    hge->System_SetState(HGE_TEXTUREFILTER, false);
    //hge->System_SetState(HGE_FPS, 60);
    hge->System_SetState(HGE_WINDOWCAPTION, false);
    hge->System_SetState(HGE_HIDEMOUSE, false);

    if (hge->System_Initiate()) {
        GV->Init();
        GV->StateMgr->Init();
        GV->StateMgr->Push(new State::EditingWW());
        hge->System_Start();
    } else {
        MessageBox(NULL, hge->System_GetErrorMessage(), PRODUCT_NAME, MB_OK | MB_ICONERROR | MB_APPLMODAL);
    }
    GV->Console->Printf("~w~Application shutdown.");
    delete GV;
    hge->System_Shutdown();
    hge->Release();
    return 0;
}
