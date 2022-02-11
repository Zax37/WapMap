#include "globals.h"
#include "../shared/cStateMgr.h"
#include "../shared/cSFS.h"
#include "../shared/commonFunc.h"
#include "funcConsole.h"
#include "version.h"
#include "states/dialog.h"
#include "langID.h"
#include "cBrush.h"
#include <direct.h>
#include "globlua.h"
#include "resources.h"
#include <sstream>
#include <process.h>
#include <string>

cGlobals *GV;
hgeQuad q = {};
extern HGE *hge;
cInterfaceSheet *_ghGfxInterface;

#ifdef SHOWMEMUSAGE
void _ThreadingMemUsage(void * globals)
{
    cGlobals * gv = (cGlobals*)globals;
    char buffer[128];
    sprintf(buffer, "tasklist /nh /fi \"PID eq %d\" /fo csv", int(GV->dwProcID));
    FILE * pipe = _popen(buffer, "r");
    if (!pipe) return;
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    _pclose(pipe);
    if (strlen(result.c_str()) <= 3) return;
    char * ret = new char[strlen(result.c_str()) + 1];
    strcpy(ret, result.c_str());
    char * lastoccur = strrchr(ret, ',');
    int lastpos = lastoccur - ret;
    char* ret2 = new char[strlen(ret) - lastpos - 3];
    for (int i = 0; i < strlen(ret) - lastpos - 1 - 2; i++)
        ret2[i] = ret[i + lastpos + 2];
    ret2[strlen(ret) - lastpos - 4] = '\0';
    ret2[strlen(ret2) - 3] = '\0';
    char *p1 = ret2, *p2 = ret2;
    while (*p1 != 0) {
        if (!isdigit(*p1))
            ++p1;
        else
            *p2++ = *p1++;
    }
    *p2 = 0;
    int memused = atoi(ret2);
    delete[] ret2;

    EnterCriticalSection(&gv->csMemUsage);
    sprintf(gv->szMemUsage, "%.2f", float(memused) / 1024.0f);
    LeaveCriticalSection(&gv->csMemUsage);
    delete[] ret;
}

void cGlobals::UpdateMemUsage()
{
    fMemUsageTimer += hge->Timer_GetDelta();
    if (fMemUsageTimer > MEMUSAGEUPDATEDELAY) {
        fMemUsageTimer -= MEMUSAGEUPDATEDELAY;
        _beginthread(_ThreadingMemUsage, 0, (void*)this);
    }
}
#endif

void cGlobals::GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

cGlobals::cGlobals() {
    q.blend = BLEND_DEFAULT;
#ifdef SHOWMEMUSAGE
    fMemUsageTimer = 0.0f;
#endif
    StateMgr = new SHR::cStateMgr();
    Console = new SHR::cConsole();
    Console->LogToFile("log.html");
    Console->AddCommand("moo", "SACRE MOO!", ConMoo);
    Console->AddCommand("help", "Shows help; help [cmd] - Shows detailed help.", ConHelp);
    Console->AddCommand("vlist", "Shows list of editable variables.", ConVar);
    Console->AddCommand("vget", "vget [name] - Get variable's value.", ConVarGet);
    Console->AddCommand("vset", "vset [name] [value] Set variable's value.", ConVarSet);
    Console->AddCommand("lua", "lua [cmd] - Execute specified command.", ConLua);
    Console->AddCommand("flushlua", "Reloads the interpreter.", ConFlushLua);
    Console->AddCommand("luabuf", "Prints Lua buffer.", ConLuaGetBuf);
    Console->AddCommand("getcwd", "Prints current working directory.", ConCWD);
#ifdef SHOWMEMUSAGE
    Console->AddCommand("memtest", "Checks memory usage.", ConMemInfo);
#endif
    Console->AddCommand("reloadsh", "Reloads shader.", ConShader);

    _SYSTEMTIME time;
    GetSystemTime(&time);

    Console->Print("KSEngine initialized.     _");
    Console->Printf("It's %02d:%02d of %02d.%02d.%04d. \\`*-.", time.wHour, time.wMinute, time.wDay, time.wMonth,
                    time.wYear);

    Console->Print("                           )  _`-.");
    Console->Print("                          .  : `. .");
    Console->Print("                          : _   '  \\");
    Console->Print("~g~WapMap~w~                   ; *` _.   `*-._");
    Console->Printf("%-26s`-.-'          `-.", WA_VERSTRING);
    Console->Print("                            ;       `       `.");
    Console->Print("Type '~y~help~w~' to view list    :.       .        \\");
    Console->Print("of available commands.      . \\  .   :   .-'   .");
    Console->Print("                            '  `+.;  ;  '      :");
    Console->Print("Type '~y~help x~w~' to see how    :  '  |    ;       ;-.");
    Console->Print("to use particular cmd.      ; '   : :`-:     _.`* ;");
    Console->Print("                         .*' /  .*' ; .*`- +'  `*'");
    Console->Print("                         `*-*   `*-*  `*-*'");

    {
        char tmp[MAX_PATH];
        _getcwd(tmp, MAX_PATH);
        Console->Printf("CWD: ~y~%s~w~", tmp);
    }

    ini = new CSimpleIni;
    ini->LoadFile("settings.cfg");

    const char *lang = ini->GetValue("WapMap", "Language", "English");
    try {
        Lang = new SHR::cLanguage(lang, WA_LANGVER);
    }
    catch (int &i) {
        char tmp[128], tmp2[64];
        if (i == -1)
            sprintf(tmp2, "language not found");
        else if (i == -2)
            sprintf(tmp2, "invalid version (%d, but requires %d)", atoi(lang), WA_LANGVER);
        else
            sprintf(tmp2, "unknown error");
        sprintf(tmp, "Unable to load language '%s' (%s).", lang, tmp2);
        Console->Printf("~r~%s~w~", tmp);
        State::MessageBox(PRODUCT_NAME, tmp, ST_DIALOG_ICON_ERROR);
        exit(123);
    }

    Console->Printf("~g~Loaded language '~y~%s~g~' by ~y~%s~w~.", lang, Lang->GetAuthor());

    if (!strcmp(Lang->GetCode(), "RU")) {
        Console->Printf("~y~Detected russian, switching to CP1251.");
        setlocale(LC_ALL, "Russian_Russia.1251");
    }

    {
        std::vector<SHR::DisplayMode> disp = SHR::GetDisplayModes();
        int w = std::stoi(ini->GetValue("WapMap", "DisplayWidth", STR(DEF_SCREEN_WIDTH))),
            h = std::stoi(ini->GetValue("WapMap", "DisplayHeight", STR(DEF_SCREEN_HEIGHT)));

        int horizontal, vertical;
        GetDesktopResolution(horizontal, vertical);
        bFirstRun = atoi(ini->GetValue("WapMap", "FirstRun", "1"));

        if (MIN_SUPPORTED_SCREEN_WIDTH > horizontal || MIN_SUPPORTED_SCREEN_HEIGHT > vertical) {
            if (w != horizontal || h != vertical) {
                w = horizontal;
                h = vertical;
                MessageBoxA(0, Lang->GetString("Strings", Lang_SmallScreenWarning_Message), Lang->GetString("Strings", Lang_SmallScreenWarning_Title), 0);
            }
        } else {
            int smallw = 10000, smallh = 10000;
            bool ok = 0, defok = 0;
            for (int i = 0; i < disp.size(); i++) {
                if (disp[i].iWidth < MIN_SUPPORTED_SCREEN_WIDTH || disp[i].iHeight < MIN_SUPPORTED_SCREEN_HEIGHT
                    || disp[i].iWidth > horizontal || disp[i].iHeight > vertical) continue;
                if (disp[i].iWidth * disp[i].iHeight < smallw * smallh) {
                    smallw = disp[i].iWidth;
                    smallh = disp[i].iHeight;
                }
                if (disp[i].iWidth == w && disp[i].iHeight == h && disp[i].iDepth == 32)
                    ok = 1;
                if (disp[i].iWidth == DEF_SCREEN_WIDTH && disp[i].iHeight == DEF_SCREEN_HEIGHT && disp[i].iDepth == 32)
                    defok = 1;
                if (defok && ok)
                    break;
            }

            if (ok) {
                Console->Printf("~g~Settings display mode valid; running in %dx%d mode.", w, h);
            } else {
                if (defok) {
                    Console->Printf("~r~Settings display mode is unsupported, running in default " STR(DEF_SCREEN_WIDTH) "x" STR(DEF_SCREEN_HEIGHT) " mode.~w~");
                    w = DEF_SCREEN_WIDTH;
                    h = DEF_SCREEN_HEIGHT;
                } else {
                    Console->Printf("~r~Settings and default display mode are unsupported, trying smallest %dx%d...~w~", smallw, smallh);
                    w = smallw;
                    h = smallh;
                }
            }
        }
        
        iScreenW = w;
        iScreenH = h;

        if (bFirstRun) {
            ini->SetValue("WapMap", "Language", lang);
            ini->SetValue("WapMap", "FirstRun", "0");
            ini->SetValue("WapMap", "DisplayWidth", std::to_string(iScreenW).c_str());
            ini->SetValue("WapMap", "DisplayHeight", std::to_string(iScreenH).c_str());
            ini->SaveFile("settings.cfg");
        }
    }
}

cGlobals::~cGlobals() {
#ifdef SHOWMEMUSAGE
    DeleteCriticalSection(&csMemUsage);
#endif
    delete StateMgr;
    delete Console;
    delete sprLogoBig;
    delete sprBlank;
    delete gcnParts.gcnfntMyriad10;
    delete gcnParts.gcnfntMyriad13;
    delete gcnParts.gcnfntSystem;
    delete fntMyriad10;
    delete fntMyriad16;
    delete fntMyriad20;
    delete fntMyriad80;
    delete fntSystem17;
    delete gcnImageLoader;
    delete gcnInput;
    delete gcnGraphics;
    delete IF;
    delete[] szLastOpenPath;
    for (int i = 0; i < 2; i++) {
        delete sprCaptionMinimze[i];
        delete sprCaptionClose[i];
    }
    delete sprLogoCaption;

    delete sprCheckboard;
    delete gcnParts.sprPBarEL;
    delete gcnParts.sprPBarEM;
    delete gcnParts.sprPBarER;
    delete gcnParts.sprPBarFL;
    delete gcnParts.sprPBarFM;
    delete gcnParts.sprPBarFR;
    delete gcnParts.sprIconQuestion;
    delete gcnParts.sprIconInfo;
    delete gcnParts.sprIconError;
    delete gcnParts.sprIconWarning;
}

int Console_Panic(lua_State *L) {
    GV->Console->Printf("~r~Lua: %s~w~", lua_tostring(L, -1));
    longjmp(GV->jmp_env, 1);
    return 0;
}

void cGlobals::Init() {
#ifdef SHOWMEMUSAGE
    szMemUsage[0] = '\0';
    InitializeCriticalSection(&csMemUsage);
    _beginthread(_ThreadingMemUsage, 0, this);
#endif

    HINSTANCE instance = GetModuleHandle(NULL);

    cursors[DEFAULT] = LoadCursor(NULL, IDC_ARROW);
    cursors[TEXT] = LoadCursor(NULL, IDC_IBEAM);
    cursors[DRAG] = LoadCursor(NULL, IDC_SIZEALL);
    cursors[HAND] = LoadCursor(instance, MAKEINTRESOURCE(HAND_CURSOR));
    cursors[GRAB] = LoadCursor(instance, MAKEINTRESOURCE(GRAB_CURSOR));
    cursors[ZOOM_IN] = LoadCursor(instance, MAKEINTRESOURCE(ZOOM_IN_CURSOR));
    cursors[ZOOM_OUT] = LoadCursor(instance, MAKEINTRESOURCE(ZOOM_OUT_CURSOR));

    editState = NULL;
    char exe_name[MAX_PATH + 1];
    exe_name[MAX_PATH] = '\0';
    ::GetModuleFileName(NULL, exe_name, MAX_PATH);

    char *myDIR = SHR::GetDir(exe_name);

    {
        FILE *f = fopen("Patch.exe", "r");
        if (f != NULL) {
            fclose(f);
            unlink("Patch.exe");
        }
    }

    HKEY serialkey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Kijedi Studio\\WapMap", 0, 0, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS, 0, &serialkey, 0) == ERROR_SUCCESS) {
        char tmp[256];
        DWORD size = 256;
        if (RegQueryValueEx(serialkey, "UID", 0, 0, (BYTE *) (&tmp), &size) != ERROR_SUCCESS) {
            Console->Printf("~r~Unable to obtain UID from registry (no value).");
            szSerial = new char[0];
        } else {
            szSerial = new char[strlen(tmp) + 1];
            strcpy(szSerial, tmp);
            Console->Printf("~w~UID: ~y~%s", szSerial);
        }
        RegCloseKey(serialkey);
    } else {
        Console->Printf("~r~Unable to obtain UID from registry (no key).");
        szSerial = new char[0];
    }

    DWORD dwMajorVersion, dwMinorVersion, dwBuild, dwVersion;
    dwVersion = ::GetVersion();
    dwMajorVersion = (DWORD) (LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD) (HIBYTE(LOWORD(dwVersion)));
    if (dwVersion < 0x80000000)
        dwBuild = (DWORD) (HIWORD(dwVersion));
    else
        dwBuild = 0;
    if (dwMajorVersion < 5) {
        iOS = OS_OLD;
        sprintf(szNameOS, "95/98");
    } else if (dwMajorVersion == 5 && dwMinorVersion == 0) {
        iOS = OS_2000;
        sprintf(szNameOS, "2000");
    } else if (dwMajorVersion == 5 && dwMinorVersion == 1) {
        iOS = OS_XP;
        sprintf(szNameOS, "XP");
    } else if (dwMajorVersion == 5 && dwMinorVersion == 2) {
        iOS = OS_SERVER2003;
        sprintf(szNameOS, "Server2003");
    } else if (dwMajorVersion == 6 && dwMinorVersion == 0) {
        iOS = OS_VISTA;
        sprintf(szNameOS, "Vista");
    } else if (dwMajorVersion == 6 && dwMinorVersion == 1) {
        iOS = OS_7;
        sprintf(szNameOS, "Win7");
    } else if (dwMajorVersion == 6 && dwMinorVersion >= 2) {
        iOS = OS_8;
        sprintf(szNameOS, "Win8");
    } else if (dwMajorVersion == 10) {
        iOS = OS_10;
        sprintf(szNameOS, "Win10");
    } else {
        iOS = OS_UNKNOWN;
        sprintf(szNameOS, "???");
    }

    Console->Printf("OS: ~y~%d~w~.~y~%d ~w~(b~y~%d~w~), recognised as: ~y~%s~w~.", dwMajorVersion, dwMinorVersion,
                    dwBuild, szNameOS);

    SYSTEM_INFO m_si;
    GetSystemInfo(&m_si);
    Console->Printf("Memory span: ~y~0x%x~w~ - ~y~0x%x~w~, page size: ~y~%d~w~b.", m_si.lpMinimumApplicationAddress,
                    m_si.lpMaximumApplicationAddress, m_si.dwPageSize);
    MEMORYSTATUS meminf;
    GlobalMemoryStatus(&meminf);
    DWORD totalmem = meminf.dwAvailPhys / (100 - DWORD(meminf.dwMemoryLoad)) * 100;
    char *mem = SHR::FormatSize(int(meminf.dwAvailPhys));
    char *mem2 = SHR::FormatSize(totalmem);
    char *mem3 = SHR::FormatSize(totalmem - meminf.dwAvailPhys);
    Console->Printf("Physical memory: used ~y~%s~w~ (~y~%d~w~%%), avail. ~y~%s~w~, total ~~y~%s~w~ (est.).", mem3,
                    int(meminf.dwMemoryLoad), mem, mem2);
    Console->Printf("Logical cores: ~y~%d", m_si.dwNumberOfProcessors);
    delete[] mem;
    delete[] mem2;
    delete[] mem3;

    BOOL bIsWow64 = 0;
    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");

    if (fnIsWow64Process != NULL)
        fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
    if (bIsWow64)
        Console->Printf("Process emulated on 64-bit environment.");

    Console->Print("Loading required editor data...");

#if SFS_COMPILER
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile("res/*", &fdata);
    if (hFind != INVALID_HANDLE_VALUE) {
        cSFS_Compiler compiler;
        do {
            if (fdata.cFileName[0] == '.') continue;
            compiler.AddFile((std::string("res/") + fdata.cFileName).c_str(), fdata.cFileName);
        } while (FindNextFile(hFind, &fdata) != 0);
        compiler.CompileTo("data.sfs");
    }
#endif // SFS_COMPILER

    cSFS_Repository repo("data.sfs");
#if BUILD_DEBUG
//mkdir("res");
//repo.Unpack();
#endif
    char *ptr;
    int size;

    /*ptr = repo.GetFileAsRawData("levels.png", &size);
    texLevels = hge->Texture_Load((const char *) ptr, size);
    delete[] ptr;
    //texLevels = hge->Texture_Load("res/levels.png");
    for (int i = 0; i < 18; i++)
        sprLevels[i] = new hgeSprite(texLevels, (i % 3) * 300, int(i / 3) * 150, 300, 150);*/

    std::string line;
    ptr = repo.GetFileAsRawData("logicsClaw.wls", &size);
    //void * ptr = hge->Resource_Load("res/logicsClaw.wls", &size);
    {
        std::istringstream istr(std::string(ptr, size));
        while (getline(istr, line)) {
            vstrClawLogics.push_back(line.substr(0, line.length() - 1));
        }
    }
    //hge->Resource_Free(ptr);
    delete[] ptr;

    ptr = repo.GetFileAsRawData("standardImgsClaw.wis", &size);
    {
        std::istringstream istr(std::string((const char*)ptr, size));
        while (getline(istr, line)) {
            line.pop_back();
            char* lcstr = (char*)line.c_str();
            char* pch = strchr(lcstr, ' ');
            if (pch) {
                *pch = 0;
                if (!strcmp(pch + 1, "NANI")) {
                    vstrNANIImagesets.insert(lcstr);
                }
                else {
                    vstrStandardImagesetAnimations[lcstr] = pch + 1;
                }
            }
            vstrStandardImagesets.insert(lcstr);
        }
    }
    //hge->Resource_Free(ptr);
    delete[] ptr;

    Console->Print("   Main sheet...");

    ptr = repo.GetFileAsRawData("main.png", &size);
    texMain = hge->Texture_Load((const char *) ptr, size);
    delete[] ptr;
    //texMain = hge->Texture_Load("res/main.png");

    hGfxInterface = new cInterfaceSheet;
    _ghGfxInterface = hGfxInterface;

    for (int state = 0; state < 4; state++) {
        hGfxInterface->sprBreadcrumb[state][0] = new hgeSprite(texMain, 950, 792 + 26 * state, 5, 26);
        hGfxInterface->sprBreadcrumb[state][1] = new hgeSprite(texMain, 955, 792 + 26 * state, 5, 26);
        hGfxInterface->sprBreadcrumb[state][2] = new hgeSprite(texMain, 983, 792 + 26 * state, 10, 26);
        hGfxInterface->sprBreadcrumb[state][3] = new hgeSprite(texMain, 1000, 792 + 26 * state, 14, 26);
        hGfxInterface->sprBreadcrumb[state][4] = new hgeSprite(texMain, 1016, 792 + 26 * state, 8, 26);
    }

    for (int x = 0; x < 2; x++) {
        int px = (x == 0 ? 918 : 934),
            oy = (x == 0 ? 17 : 16);
        for (int i = 0; i < 5; i++) {
            hGfxInterface->sprControl[x][i] = new hgeSprite(texMain, px, 789 + oy * i, 16, oy);
        }
        hGfxInterface->sprControl[x][5] = new hgeSprite(texMain, 898, x == 0 ? 851 : 869, 18, 18);
    }

    for (int i = 0; i < LAY_APP_BUTTONS_COUNT + 1; ++i) {
        hGfxInterface->sprAppBar[i] = new hgeSprite(texMain, 928 + i * 20, 0, 20, 20);
    }

    for (int y = 0; y < 4; y++) {
        int py = 792 + y * 26;
        hGfxInterface->sprButton[y][0] = new hgeSprite(texMain, 950, py, 4, 4);
        hGfxInterface->sprButton[y][1] = new hgeSprite(texMain, 954, py, 25, 4);
        hGfxInterface->sprButton[y][2] = new hgeSprite(texMain, 1020, py, 4, 4);

        hGfxInterface->sprButton[y][3] = new hgeSprite(texMain, 950, py + 4, 4, 18);
        hGfxInterface->sprButton[y][4] = new hgeSprite(texMain, 954, py + 4, 25, 18);
        hGfxInterface->sprButton[y][5] = new hgeSprite(texMain, 1020, py + 4, 4, 18);

        hGfxInterface->sprButton[y][6] = new hgeSprite(texMain, 950, py + 22, 4, 4);
        hGfxInterface->sprButton[y][7] = new hgeSprite(texMain, 954, py + 22, 25, 4);
        hGfxInterface->sprButton[y][8] = new hgeSprite(texMain, 1020, py + 22, 4, 4);
    }

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 4; x++)
            hGfxInterface->sprSlider[y][x] = new hgeSprite(texMain, 928 + x * 12, 757 + y * 12, 12, (y ? 15 : 12));

    for (int i = 0; i < 3; ++i) {
        hGfxInterface->sprSliderBG[0][i] = new hgeSprite(texMain, 896, 736 + i * 5, 5, 5);
        hGfxInterface->sprSliderBG[1][i] = new hgeSprite(texMain, 928 + i * 5, 784, 5, 5);
    }

    for (int i = 0; i < 3; i++) {
        hGfxInterface->sprScrollbar[0][i][0] = new hgeSprite(texMain, 904 + i * 24, 736, 9, 16);
        hGfxInterface->sprScrollbar[0][i][1] = new hgeSprite(texMain, 914 + i * 24, 736, 4, 16);
        hGfxInterface->sprScrollbar[0][i][2] = new hgeSprite(texMain, 919 + i * 24, 736, 9, 16);

        hGfxInterface->sprScrollbar[1][i][0] = new hgeSprite(texMain, 976 + i * 16, 736, 16, 9);
        hGfxInterface->sprScrollbar[1][i][1] = new hgeSprite(texMain, 976 + i * 16, 746, 16, 4);
        hGfxInterface->sprScrollbar[1][i][2] = new hgeSprite(texMain, 976 + i * 16, 751, 16, 9);
    }

    for (int z = 0; z < 2; z++) {
        hGfxInterface->sprTextField[z][0] = new hgeSprite(texMain, 978 + z * 23, 760, 7, 7);
        hGfxInterface->sprTextField[z][1] = new hgeSprite(texMain, 987 + z * 23, 760, 5, 7);
        hGfxInterface->sprTextField[z][2] = new hgeSprite(texMain, 994 + z * 23, 760, 7, 7);

        hGfxInterface->sprTextField[z][3] = new hgeSprite(texMain, 978 + z * 23, 767, 4, 15);
        hGfxInterface->sprTextField[z][4] = new hgeSprite(texMain, 997 + z * 23, 767, 4, 15);

        hGfxInterface->sprTextField[z][5] = new hgeSprite(texMain, 978 + z * 23, 782, 7, 7);
        hGfxInterface->sprTextField[z][6] = new hgeSprite(texMain, 987 + z * 23, 782, 5, 7);
        hGfxInterface->sprTextField[z][7] = new hgeSprite(texMain, 994 + z * 23, 782, 7, 7);
    }

    hGfxInterface->sprDropDownBarArrow = new hgeSprite(texMain, 917, 768, 9, 7);

    Console->Print("   Various...");

    sprConsoleBG = new hgeSprite(texMain, 672, 192, 128, 128);
    Console->SetBG(sprConsoleBG);

    sprLogoBig = new hgeSprite(texMain, 0, 640, 300, 80);
    sprLogoCaption = new hgeSprite(texMain, 780, 172, 20, 20);

    sprBlank = new hgeSprite(texMain, 640, 120, 6, 6);

    sprCaptionMinimze[0] = new hgeSprite(texMain, 50, 306, 20, 20);
    sprCaptionClose[0] = new hgeSprite(texMain, 70, 306, 21, 20);
    sprCaptionMinimze[1] = new hgeSprite(texMain, 50, 346, 20, 20);
    sprCaptionClose[1] = new hgeSprite(texMain, 70, 346, 21, 20);

    sprCheckboard = new hgeSprite(texMain, 640, 0, 120, 120);

    sprLevelsMicro16[WWD::Game_Claw - WWD::Games_First] = new hgeSprite*[14];
    sprLevelsMicro16[WWD::Game_Gruntz - WWD::Games_First] = new hgeSprite*[8];
    for (int i = 0; i < 14; i++)
        sprLevelsMicro16[WWD::Game_Claw - WWD::Games_First][i] = new hgeSprite(texMain, 416 + 16 * i, 0, 16, 16);
    for (int i = 0; i < 8; i++)
        sprLevelsMicro16[WWD::Game_Gruntz - WWD::Games_First][i] = new hgeSprite(texMain, 768, 16 * i, 16, 16);

    if (!strcmp(Lang->GetCode(), "RU")) {
        fntMyriad16 = SHR::LoadFontFromSFS(&repo, "16px_r.fnt", "16px_r.png");
    } else {
        fntMyriad16 = SHR::LoadFontFromSFS(&repo, "16px.fnt", "16px.png");
    }
    fntMyriad10 = SHR::LoadFontFromSFS(&repo, "10px.fnt", "10px.png");
    fntMyriad20 = SHR::LoadFontFromSFS(&repo, "20px.fnt", "20px.png");
    fntMyriad80 = SHR::LoadFontFromSFS(&repo, "80px.fnt", "80px.png");
    fntSystem17 = SHR::LoadFontFromSFS(&repo, "17px.fnt", "17px.png");
    fntMinimal = SHR::LoadFontFromSFS(&repo, "minimal.fnt", "minimal.png");

    Console->SetFont(fntSystem17);

    Console->Print("   UI initialization...");

    gcnImageLoader = new gcn::HGEImageLoader();
    Console->Print("   Reg...");
    gcn::Image::setImageLoader(gcnImageLoader);
    Console->Print("   Gfx...");
    gcnGraphics = new gcn::HGEGraphics();
    Console->Print("   Input...");
    gcnInput = new gcn::HGEInput();

    Console->Print("   Fonts...");
    gcnParts.gcnfntMyriad13 = new gcn::HGEImageFont(fntMyriad16, 0);
    gcnParts.gcnfntMyriad10 = new gcn::HGEImageFont(fntMyriad10, 0);
    gcnParts.gcnfntSystem = new gcn::HGEImageFont(fntSystem17, 0);

    gcn::Widget::setGlobalFont(gcnParts.gcnfntMyriad13);

    Console->Print("   UI assets...");

    int offX = 992, offY = 704;
    gcnParts.sprPBarFL = new hgeSprite(texMain, offX, offY, 3, 31);
    gcnParts.sprPBarFM = new hgeSprite(texMain, offX + 3, offY, 3, 31);
    gcnParts.sprPBarFR = new hgeSprite(texMain, offX + 9, offY, 3, 31);

    gcnParts.sprPBarEL = new hgeSprite(texMain, offX + 22, offY, 3, 31);
    gcnParts.sprPBarEM = new hgeSprite(texMain, offX + 14, offY, 3, 31);
    gcnParts.sprPBarER = new hgeSprite(texMain, offX + 19, offY, 3, 31);

    offX = 768, offY = 960;
    gcnParts.sprIconQuestion = new hgeSprite(texMain, offX, offY, 64, 64);
    offX += 64;
    gcnParts.sprIconInfo = new hgeSprite(texMain, offX, offY, 64, 64);
    offX += 64;
    gcnParts.sprIconWarning = new hgeSprite(texMain, offX, offY, 64, 64);
    offX += 64;
    gcnParts.sprIconError = new hgeSprite(texMain, offX, offY, 64, 64);

    sprSmiley = new hgeSprite(texMain, 736, 128, 32, 32);
    sprSmiley->SetHotSpot(16, 16);

    sprDottedLineCorner = new hgeSprite(texMain, 768, 160, 12, 12);
    sprDottedLineHorizontal = new hgeSprite(texMain, 784, 160, 16, 4);
    sprDottedLineVertical = new hgeSprite(texMain, 768, 176, 4, 16);

    sprArrowVerticalU = new hgeSprite(texMain, 867, 0, 48, 37);
    sprArrowVerticalM = new hgeSprite(texMain, 867, 37, 48, 128);
    sprArrowVerticalD = new hgeSprite(texMain, 867, 165, 48, 38);

    sprArrowVerticalU->SetHotSpot(22, 37);
    sprArrowVerticalM->SetHotSpot(22, 128);
    sprArrowVerticalD->SetHotSpot(22, 0);

    Console->Print("   Fancy icons...");

    offY = 16;

    for (int i = 0, y = 0; y < 3; ++y) {
        for (int x = 0; x < 40 && i < Gfx16Icons_Count; ++x, ++i) {
            sprIcons16[i] = new hgeSprite(texMain, x * 16, offY, 16, 16);
        }
        offY += 16;
    }

    for (int i = 0, y = 0; y < 7; ++y) {
        for (int x = 0; x < 20 && i < GfxIcons_Count; ++x, ++i) {
            sprIcons[i] = new hgeSprite(texMain, x * 32, offY, 32, 32);
        }
        offY += 32;
    }

    for (int i = 0; i < 5; i++) {
        sprGamesBig[i] = new hgeSprite(texMain, 640, 128 + 32 * i, 32, 32);
        sprGamesSmall[i] = new hgeSprite(texMain, 16 * i, 0, 16, 16);
    }

    sprTile = new hgeSprite(texMain, 672, 128, 64, 64);
    sprTile->SetColor(0x99FFFFFF);

    sprContextCascadeArrow = new hgeSprite(texMain, 905, 756, 10, 12);
    sprTabCloseButton = new hgeSprite(texMain, 917, 756, 9, 9);
    sprTabCloseButtonFocused = new hgeSprite(texMain, 906, 779, 13, 13);
    sprTabAddButton = new hgeSprite(texMain, 896, 800, 16, 16);
    sprTabAddButtonFocused = new hgeSprite(texMain, 896, 816, 16, 16);
    sprHomepageBackButton = new hgeSprite(texMain, 960, 704, 32, 32);

    IF = new SHR::Interface(&gcnParts);

    for (WWD::GAME i = WWD::Games_First; i <= WWD::Games_Last; ++i) {
        gamePaths[i] = ini->GetValue("Paths", WWD::GAME_NAMES[i], "");
    }

    const char *pVal = gamePaths[WWD::Game_Claw].c_str();

#ifndef SFS_COMPILER
    WIN32_FIND_DATA fdata;
    HANDLE
#endif
            hFind = strlen(pVal) > 0 ? FindFirstFile(pVal, &fdata) : INVALID_HANDLE_VALUE;

    if (hFind == INVALID_HANDLE_VALUE) {
        if (pVal[0] != '\0')
            Console->Printf("~r~Setting's Claw path invalid: '~y~%s~r~'!", pVal);
        else
            Console->Printf("~r~Setting's Claw path empty.~w~");
        HKEY key;
        if (RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Software\\RealNetworks\\Games", &key) != ERROR_SUCCESS) {
            Console->Printf("~r~Unable to find default Claw installation path.~w~\n");
        } else {
            DWORD keysnum = 0;
            RegQueryInfoKey(key, 0, 0, 0, &keysnum, 0, 0, 0, 0, 0, 0, 0);
            char *clawkey = NULL;
            for (int i = 0; i < keysnum; i++) {
                DWORD namelen = 255;
                TCHAR *keyname = new TCHAR[namelen];
                DWORD retCode = RegEnumKeyEx(key, i, keyname, &namelen, NULL, NULL, NULL, 0);
                if (retCode == ERROR_SUCCESS) {
                    if (!strncmp(keyname, "Claw", 4)) {
                        clawkey = new char[strlen(keyname) + 5];
                        sprintf(clawkey, "%s\\1.0", keyname);
                        break;
                    }
                }
                delete[] keyname;
            }
            if (clawkey != NULL) {
                HKEY clawkeyh;
                if (RegOpenKey(key, clawkey, &clawkeyh) == ERROR_SUCCESS) {
                    char tmp[MAX_PATH];
                    DWORD size = MAX_PATH;
                    RegQueryValueEx(clawkeyh, "InstallPath", 0, 0, (BYTE *) tmp, &size);
                    RegCloseKey(clawkeyh);
                    if (tmp[strlen(tmp) - 1] == '/' || tmp[strlen(tmp) - 1] == '\\')
                        tmp[strlen(tmp) - 1] = '\0';
                    Console->Printf("~g~Found install path (by RealNetworks): %s~w~", tmp);
                    ini->SetValue("Paths", "Claw", tmp);
                    ini->SaveFile("settings.cfg");
                    gamePaths[WWD::Game_Claw] = tmp;
                }
                delete[] clawkey;
            }
            RegCloseKey(key);
        }
    }

    if (gamePaths[WWD::Game_Claw].empty()) {
        char tmp[768];
        bool fail = 0;

        Console->Printf("~w~Checking WM directory for Claw installation...");

        sprintf(tmp, "%s/CLAW.EXE", myDIR);
        FILE *f = fopen(tmp, "r");
        if (f) fclose(f);
        else fail = 1;
        if (!fail) {
            gamePaths[WWD::Game_Claw] = myDIR;
        } else {
            char *npath = SHR::GetDir(myDIR);
            Console->Printf("~w~Checking in parent directory...");
            sprintf(tmp, "%s/CLAW.EXE", npath);
            f = fopen(tmp, "r");
            if (f) fclose(f);
            else fail = 1;
            if (!fail) {
                gamePaths[WWD::Game_Claw] = npath;
            }
            delete[] npath;
        }

        if (!gamePaths[WWD::Game_Claw].empty()) {
            Console->Printf("~g~Found in ~y~%s~g~.", myDIR);
            ini->SetValue("Paths", "Claw", myDIR);
            ini->SaveFile("settings.cfg");
        } else {
            Console->Printf("~r~Unable to find Claw installation.");
        }
    }

    pVal = ini->GetValue("RSS", "TCR");
    if (pVal == NULL) {
        szUrlTCR = new char[1];
        szUrlTCR[0] = '\0';
    } else {
        szUrlTCR = new char[strlen(pVal) + 1];
        strcpy(szUrlTCR, pVal);
    }

    pVal = ini->GetValue("WapMap", "LastOpenDir");
    if (pVal == NULL) {
        szLastOpenPath = new char[strlen(myDIR) + 1];
        strcpy(szLastOpenPath, myDIR);
    } else {
        szLastOpenPath = new char[strlen(pVal) + 1];
        strcpy(szLastOpenPath, pVal);
    }

    pVal = ini->GetValue("WapMap", "LastSaveDir");
    if (pVal == NULL) {
        szLastSavePath = new char[strlen(myDIR) + 1];
        strcpy(szLastSavePath, myDIR);
    } else {
        szLastSavePath = new char[strlen(pVal) + 1];
        strcpy(szLastSavePath, pVal);
    }

    bSmoothZoom = atoi(ini->GetValue("WapMap", "SmoothZoom", "1"));

    bRealSim = atoi(ini->GetValue("WapMap", "RealSim", "0"));

    bAutoUpdate = atoi(ini->GetValue("WapMap", "AutoUpdate", "1"));
    bAlphaHigherPlanes = atoi(ini->GetValue("WapMap", "AlphaOverlapping ", "0"));

    bool enableConsole = atoi(ini->GetValue("WapMap", "Console", "0"));
    Console->Enable(enableConsole);

    sprKijan = new hgeSprite(texMain, 736, 160, 32, 32);
    sprKijan->SetHotSpot(16, 16);
    sprZax = new hgeSprite(texMain, 768, 128, 32, 32);
    sprZax->SetHotSpot(16, 16);
    sprSnowflake = new hgeSprite(texMain, 800, 128, 32, 32);
    sprSnowflake->SetHotSpot(16, 16);

    bKijan = bZax = bWinter = false;
    Console->AddModifiableBool("kijan", &bKijan);
    Console->AddModifiableBool("zax", &bZax);
    Console->AddModifiableBool("realsim", &bRealSim);
    Console->AddModifiableBool("winter", &bWinter);

    SYSTEMTIME time;
    GetLocalTime(&time);
    if ((time.wMonth == 12 && time.wDay > 15) || (time.wMonth == 1 && time.wDay < 3))
        bWinter = true;

    conL = NULL;
    ReloadLua();

    conLuaBuf = NULL;

    delete[] myDIR;

    colBase = 0xff4d4d4d;//0xff989898;
    colBaseDark = colBase - 0x000F0F0F;

    colLineDark = 0x7F070707;
    colLineBright = 0x7F565656;
    colOutline = 0xFF585858;

    colFontWhite = 0xFFe1e1e1;

    colActive = 0xFF1585e2;
}

void cGlobals::ResetWD() {
    char exe_name[MAX_PATH + 1];
    exe_name[MAX_PATH] = '\0';
    ::GetModuleFileName(NULL, exe_name, MAX_PATH);
    char *myDIR = SHR::GetDir(exe_name);
    _chdir(myDIR);
    delete[] myDIR;
}

void cGlobals::SetLastOpenPath(const char *npath) {
    delete[] szLastOpenPath;
    szLastOpenPath = new char[strlen(npath) + 1];
    strcpy(szLastOpenPath, npath);
    ini->SetValue("WapMap", "LastOpenDir", szLastOpenPath);
    ResetWD();
    ini->SaveFile("settings.cfg");
}

void cGlobals::SetLastSavePath(const char *npath) {
    delete[] szLastSavePath;
    szLastSavePath = new char[strlen(npath) + 1];
    strcpy(szLastSavePath, npath);
    ini->SetValue("WapMap", "LastSaveDir", szLastSavePath);
    ResetWD();
    ini->SaveFile("settings.cfg");
}

void cGlobals::ExecuteLua(const char *script, bool popuponerror) {
    GV->jmp_val = setjmp(GV->jmp_env);
    if (!GV->jmp_val) {
        if (luaL_dostring(conL, script)) {
            Console->Printf("~r~Lua parser error: %s~w~", lua_tostring(conL, -1));
            if (popuponerror) {
                StateMgr->Push(new State::Dialog(Lang->GetString("Strings", Lang_LuaError), lua_tostring(conL, -1),
                                                 ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
            }
        }
    }
}

void cGlobals::ReloadLua() {
    if (conL != NULL)
        lua_close(conL);
    conL = luaL_newstate();
    luaL_openlibs(conL);
    lua_atpanic(conL, Console_Panic);
    lua_register(conL, "wmLog", gv_wmLog);

    lua_register(conL, "wmRandInt", gv_wmRandInt);
    lua_register(conL, "wmRandBool", gv_wmRandBool);

    lua_register(conL, "wmGetFileName", gv_wmGetFileName);
    lua_register(conL, "wmGetMapName", gv_wmGetMapName);
    lua_register(conL, "wmGetMapAuthor", gv_wmGetMapAuthor);
    lua_register(conL, "wmGetMapDate", gv_wmGetMapDate);

    lua_register(conL, "wmGetActivePlane", gv_wmGetActivePlane);
    lua_register(conL, "wmGetPlanesCount", gv_wmGetPlanesCount);
    lua_register(conL, "wmGetPlaneByIt", gv_wmGetPlaneByIt);
    lua_register(conL, "wmGetPlaneByName", gv_wmGetPlaneByName);

    lua_register(conL, "wmGetPlaneName", gv_wmGetPlaneName);
    lua_register(conL, "wmGetPlaneWidth", gv_wmGetPlaneWidth);
    lua_register(conL, "wmGetPlaneHeight", gv_wmGetPlaneHeight);
    lua_register(conL, "wmGetObjectsCount", gv_wmGetObjectsCount);

    lua_register(conL, "wmPlaceTile", wmPlaceTile);
    lua_register(conL, "wmGetTile", wmGetTile);
}

void cGlobals::RenderLogoWithVersion(int x, int y, int alpha) {
    sprLogoBig->SetColor(SETA(0xFFFFFF, alpha));
    sprLogoBig->Render(x - 151, y - 32);
    fntMyriad16->SetColor(SETA(0xFFFFFF, alpha));
    fntMyriad16->Render(x - 55, y + 20, HGETEXT_LEFT, WA_VERSTRING, 0);
}

void cGlobals::SetCursor(APP_CURSOR cursor) {
    ::SetCursor(cursors[cursor]);
}

void cGlobals::SliderDrawBar(int dx, int dy, bool orient, int size, int type, DWORD col) {
    for (int i = 0; i < 3; i++)
        hGfxInterface->sprScrollbar[orient][type][i]->SetColor(col);

    hGfxInterface->sprScrollbar[orient][type][0]->Render(dx, dy);
    if (orient) {
        hGfxInterface->sprScrollbar[orient][type][1]->RenderStretch(dx, dy + 9, dx + 16, dy + size - 9);
        hGfxInterface->sprScrollbar[orient][type][2]->Render(dx, dy + size - 9);
    } else {
        hGfxInterface->sprScrollbar[orient][type][1]->RenderStretch(dx + 9, dy, dx + size - 9, dy + 16);
        hGfxInterface->sprScrollbar[orient][type][2]->Render(dx + size - 9, dy);
    }
}
