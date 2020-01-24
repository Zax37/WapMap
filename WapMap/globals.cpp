#include "globals.h"
#include "../shared/cStateMgr.h"
#include "../shared/cSFS.h"
#include "../shared/commonFunc.h"
#include "funcConsole.h"
#include "conf.h"
#include "states/error.h"
#include "langID.h"
#include "cInterfaceSheet.h"
#include "cBrush.h"

#include <sys/types.h>
#include <direct.h>
#include "globlua.h"
#include <windows.h>
#include <sstream>
#include <process.h>
#include <string>

cGlobals * GV;
extern HGE * hge;
cInterfaceSheet * _ghGfxInterface;

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

cGlobals::cGlobals()
{
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
	Console->Printf("It's %02d:%02d of %02d.%02d.%04d. \\`*-.", time.wHour, time.wMinute, time.wDay, time.wMonth, time.wYear);

	Console->Print("                           )  _`-.");
	Console->Print("                          .  : `. .");
	Console->Print("                          : _   '  \\");
	Console->Print("Wap~g~Map~w~                   ; *` _.   `*-._");
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

	bool bError = 0;
	const char * lang = ini->GetValue("WapMap", "Language", "English");
	try {
		Lang = new SHR::cLanguage(lang, WA_LANGVER);
	}
	catch (int& i) {
		char tmp[128], tmp2[64];
		if (i == -1)
			sprintf(tmp2, "language not found");
		else if (i == -2)
			sprintf(tmp2, "invalid version (%d, but requires %d)", atoi(lang), WA_LANGVER);
		else
			sprintf(tmp2, "unknown error");
		sprintf(tmp, "Unable to load language '%s' (%s).", lang, tmp2);
		Console->Printf("~r~%s~w~", tmp);
		MessageBox(0, tmp, "WapMap error", MB_OK | MB_ICONERROR);
		exit(123);
	}

	Console->Printf("~g~Loaded language '~y~%s~g~' by ~y~%s~w~.", lang, Lang->GetAuthor());

	if (!strcmp(Lang->GetCode(), "RU")) {
		Console->Printf("~y~Detected russian, switching to CP1251.");
		setlocale(LC_ALL, "Russian_Russia.1251");
	}

	{
		std::vector<SHR::DisplayMode> disp = SHR::GetDisplayModes();
		int w = std::stoi(ini->GetValue("WapMap", "DisplayWidth", "1024")),
			h = std::stoi(ini->GetValue("WapMap", "DisplayHeight", "768"));
		int smallw = 10000, smallh = 10000;
		bool ok = 0, defok = 0;
		for (int i = 0; i < disp.size(); i++) {
			if (disp[i].iWidth < 1024 || disp[i].iHeight < 768) continue;
			if (disp[i].iWidth*disp[i].iHeight < smallw*smallh) {
				smallw = disp[i].iWidth;
				smallh = disp[i].iHeight;
			}
			if (disp[i].iWidth == w && disp[i].iHeight == h && disp[i].iDepth == 32)
				ok = 1;
			if (disp[i].iWidth == 1024 && disp[i].iHeight == 768 && disp[i].iDepth == 32)
				defok = 1;
			if (defok && ok)
				break;
		}
		if (!ok && !defok) {
			Console->Printf("~r~Settings and default display mode are unsupported, trying smallest %dx%d...~w~", smallw, smallh);
			w = smallw;
			h = smallh;
		}
		if (!ok) {
			Console->Printf("~r~Settings display mode is unsupported, running in default 1024x768 mode.~w~");
			w = 1024;
			h = 768;
		}
		else {
			Console->Printf("~g~Settings display mode valid; running in %dx%d mode.", w, h);
		}
		iScreenW = w;
		iScreenH = h;

		bFirstRun = atoi(ini->GetValue("WapMap", "FirstRun", "1"));
		if (bFirstRun) {
			ini->SetValue("WapMap", "Language", lang);
			ini->SetValue("WapMap", "FirstRun", "0");
			ini->SetValue("WapMap", "DisplayWidth", std::to_string(iScreenW).c_str());
			ini->SetValue("WapMap", "DisplayHeight", std::to_string(iScreenH).c_str());
			ini->SaveFile("settings.cfg");
		}
	}
}

cGlobals::~cGlobals()
{
#ifdef SHOWMEMUSAGE
	DeleteCriticalSection(&csMemUsage);
#endif
	delete StateMgr;
	delete Console;
	delete sprConsoleBG;
	delete sprLogoBig, sprBlank;
	delete gcnParts.gcnfntMyriad10, gcnParts.gcnfntMyriad13, gcnParts.gcnfntSystem;
	delete fntMyriad10, fntMyriad13, fntMyriad20, fntMyriad80, fntSystem17;
	delete gcnImageLoader;
	delete gcnInput;
	delete gcnGraphics;
	delete IF;
	delete[] szLastOpenPath;
	for (int i = 0; i < 2; i++) {
		delete sprCaptionMinimalize[i];
		delete sprCaptionClose[i];
	}
	delete sprLogoMini;
	delete sprIconCaption;

	for (int x = 0; x < 2; x++)
		for (int y = 0; y < 3; y++) {
			delete gcnParts.sprButBar[x][y];
			delete gcnParts.sprButBarH[x][y];
			delete gcnParts.sprButBarD[x][y];
			delete gcnParts.sprButBarP[x][y];
		}

	delete sprCheckboard;

	delete gcnParts.sprWindowBG;
	delete gcnParts.sprGcnWinBarL, gcnParts.sprGcnWinBarM, gcnParts.sprGcnWinBarR, gcnParts.sprCur, gcnParts.sprPBarEL, gcnParts.sprPBarEM,
		gcnParts.sprPBarER, gcnParts.sprPBarFL, gcnParts.sprPBarFM, gcnParts.sprPBarFR, gcnParts.sprIconInfo, gcnParts.sprIconError,
		gcnParts.sprIconWarning, gcnParts.sprButDDL, gcnParts.sprButDDR, gcnParts.sprButDDC;
}

int Console_Panic(lua_State *L)
{
	GV->Console->Printf("~r~Lua: %s~w~", lua_tostring(L, -1));
	longjmp(GV->jmp_env, 1);
	return 0;
}

void cGlobals::Init()
{
#ifdef SHOWMEMUSAGE
	szMemUsage[0] = '\0';
	InitializeCriticalSection(&csMemUsage);
	_beginthread(_ThreadingMemUsage, 0, this);
#endif
	editState = NULL;
	char exe_name[MAX_PATH + 1];
	exe_name[MAX_PATH] = '\0';
	::GetModuleFileName(NULL, exe_name, MAX_PATH);

	char * myDIR = SHR::GetDir(exe_name);

	{
		FILE * f = fopen("Patch.exe", "r");
		if (f != NULL) {
			fclose(f);
			unlink("Patch.exe");
		}
	}

	HKEY serialkey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Kijedi Studio\\WapMap", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &serialkey, 0) == ERROR_SUCCESS) {
		char tmp[256];
		DWORD size = 256;
		if (RegQueryValueEx(serialkey, "UID", 0, 0, (BYTE*)(&tmp), &size) != ERROR_SUCCESS) {
			Console->Printf("~r~Unable to obtain UID from registry (no value).");
			szSerial = "";
		}
		else {
			szSerial = new char[strlen(tmp) + 1];
			strcpy(szSerial, tmp);
			Console->Printf("~w~UID: ~y~%s", szSerial);
		}
		RegCloseKey(serialkey);
	}
	else {
		Console->Printf("~r~Unable to obtain UID from registry (no key).");
		szSerial = "";
	}

	HKEY verkey;
	DWORD dwMajorVersion, dwMinorVersion, dwBuild;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE, &verkey) == ERROR_SUCCESS) {
		int major, minor;
		char tmp[256];
		DWORD size = 256;
		RegQueryValueEx(verkey, "CurrentVersion", 0, 0, (BYTE*)(&tmp), &size);
		sscanf(tmp, "%d.%d", &major, &minor);
		size = 256;
		dwMajorVersion = major;
		dwMinorVersion = minor;
		RegQueryValueEx(verkey, "CurrentBuildNumber", 0, 0, (BYTE*)(&tmp), &size);
		sscanf(tmp, "%d", &dwBuild);
		size = 256;
		RegQueryValueEx(verkey, "ProductName", 0, 0, (BYTE*)(&szNameOS), &size);
		/*char serial[256];
		size = 256;
		RegQueryValueEx(verkey, "ProductId", 0, 0, (BYTE*)(&serial), &size);
		Console->Printf("%s", serial);*/
		RegCloseKey(verkey);
		if (major == 5 && minor == 0)
			iOS = OS_2000;
		else if (major == 5 && minor == 1)
			iOS = OS_XP;
		else if (major == 5 && minor == 2)
			iOS = OS_SERVER2003;
		else if (major == 6 && minor == 0)
			iOS = OS_VISTA;
		else if (major == 6 && minor == 1)
			iOS = OS_7;
		else
			iOS = OS_UNKNOWN;
		Console->Print("~g~System info acquired from registry.");
	}
	else {
		DWORD dwVersion;
		dwVersion = ::GetVersion();
		dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
		dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
		if (dwVersion < 0x80000000)
			dwBuild = (DWORD)(HIWORD(dwVersion));
		else
			dwBuild = 0;
		if (dwMajorVersion < 5) {
			iOS = OS_OLD;
			sprintf(szNameOS, "95/98");
		}
		else if (dwMajorVersion == 5 && dwMinorVersion == 0) {
			iOS = OS_2000;
			sprintf(szNameOS, "2000");
		}
		else if (dwMajorVersion == 5 && dwMinorVersion == 1) {
			iOS = OS_XP;
			sprintf(szNameOS, "XP");
		}
		else if (dwMajorVersion == 5 && dwMinorVersion == 2) {
			iOS = OS_SERVER2003;
			sprintf(szNameOS, "Server2003");
		}
		else if (dwMajorVersion == 6 && dwMinorVersion == 0) {
			iOS = OS_VISTA;
			sprintf(szNameOS, "Vista");
		}
		else if (dwMajorVersion == 6 && dwMinorVersion == 1) {
			iOS = OS_7;
			sprintf(szNameOS, "Win7");
		}
		else {
			iOS = OS_UNKNOWN;
			sprintf(szNameOS, "???");
		}
		Console->Print("~r~Unable to acquire system info from registry. Trying hard way...");
	}

	Console->Printf("OS: ~y~%d~w~.~y~%d ~w~(b~y~%d~w~), recognised as: ~y~%s~w~.", dwMajorVersion, dwMinorVersion, dwBuild, szNameOS);

	SYSTEM_INFO m_si;
	GetSystemInfo(&m_si);
	Console->Printf("Memory span: ~y~0x%x~w~ - ~y~0x%x~w~, page size: ~y~%d~w~b.", m_si.lpMinimumApplicationAddress, m_si.lpMaximumApplicationAddress, m_si.dwPageSize);
	MEMORYSTATUS meminf;
	GlobalMemoryStatus(&meminf);
	DWORD totalmem = meminf.dwAvailPhys / (100 - DWORD(meminf.dwMemoryLoad)) * 100;
	char * mem = SHR::FormatSize(int(meminf.dwAvailPhys));
	char * mem2 = SHR::FormatSize(totalmem);
	char * mem3 = SHR::FormatSize(totalmem - meminf.dwAvailPhys);
	Console->Printf("Physical memory: used ~y~%s~w~ (~y~%d~w~%%), avail. ~y~%s~w~, total ~~y~%s~w~ (est.).", mem3, int(meminf.dwMemoryLoad), mem, mem2);
	Console->Printf("Logical cores: ~y~%d", m_si.dwNumberOfProcessors);
	delete[] mem;
	delete[] mem2;
	delete[] mem3;

	BOOL bIsWow64 = 0;
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");

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
	//mkdir("res");
	//repo.Unpack();
	void * ptr;
	int size;

	ptr = repo.GetFileAsRawData("levels.png", &size);
	texLevels = hge->Texture_Load((const char*)ptr, size);
	delete[] ptr;
	//texLevels = hge->Texture_Load("res/levels.png");
	for (int i = 0; i < 18; i++)
		sprLevels[i] = new hgeSprite(texLevels, (i % 3) * 300, int(i / 3) * 150, 300, 150);

	ptr = repo.GetFileAsRawData("logicsClaw.wls", &size);
	//void * ptr = hge->Resource_Load("res/logicsClaw.wls", &size);
	std::string line;
	std::istringstream istr(std::string((const char*)ptr, size));
	while (getline(istr, line)) {
		vstrClawLogics.push_back(line.substr(0, line.length() - 1));
	}
	//hge->Resource_Free(ptr);
	delete[] ptr;

	Console->Print("   Main sheet...");

	ptr = repo.GetFileAsRawData("main.png", &size);
	texMain = hge->Texture_Load((const char*)ptr, size);
	delete[] ptr;
	//texMain = hge->Texture_Load("res/main.png");

	hGfxInterface = new cInterfaceSheet;
	_ghGfxInterface = hGfxInterface;
	hGfxInterface->sprMainBackground = new hgeSprite(texMain, 896, 896, 128, 128);
	hGfxInterface->sprMainFrameLU = new hgeSprite(texMain, 224, 0, 10, 26);
	hGfxInterface->sprMainFrameU = new hgeSprite(texMain, 224, 26, 96, 26);
	hGfxInterface->sprMainFrameRU = new hgeSprite(texMain, 234, 0, 10, 26);
	hGfxInterface->sprMainFrameR = new hgeSprite(texMain, 244, 0, 3, 26);
	hGfxInterface->sprMainFrameRD = new hgeSprite(texMain, 247, 0, 26, 26);
	hGfxInterface->sprMainFrameD = new hgeSprite(texMain, 273, 0, 26, 3);
	hGfxInterface->sprMainFrameLD = new hgeSprite(texMain, 299, 0, 15, 5);

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
	}

	hGfxInterface->sprMiniLogo = new hgeSprite(texMain, 384, 19, 110, 30);
	hGfxInterface->sprAppBar[0] = new hgeSprite(texMain, 384, 0, 20, 19);
	hGfxInterface->sprAppBar[1] = new hgeSprite(texMain, 404, 0, 20, 19);

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
		for (int x = 0; x < 2; x++)
			hGfxInterface->sprWindow[y * 2 + x] = new hgeSprite(texMain, 918 + 6 * x, 874 + 6 * y, 5, 5);

	for (int s = 0; s < 2; s++)
		for (int i = 0; i < 2; i++)
			for (int p = 0; p < 3; p++) {
				hGfxInterface->sprDropDown[i][s][p] = new hgeSprite(texMain, 933 + 6 * p, 876 + 5 * i + 10 * s, 5, 5);
			}

	for (int y = 0; y < 2; y++)
		for (int x = 0; x < 4; x++)
			hGfxInterface->sprSlider[y][x] = new hgeSprite(texMain, 928 + x * 12, 757 + y * 12, 12, (y ? 15 : 12));

	hGfxInterface->sprSliderBG[0] = new hgeSprite(texMain, 928, 784, 5, 5);
	hGfxInterface->sprSliderBG[1] = new hgeSprite(texMain, 933, 784, 5, 5);
	hGfxInterface->sprSliderBG[2] = new hgeSprite(texMain, 971, 784, 5, 5);

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

	hGfxInterface->sprDropDownBar[0] = new hgeSprite(texMain, 846, 736, 13, 22);
	hGfxInterface->sprDropDownBar[1] = new hgeSprite(texMain, 860, 736, 22, 22);
	hGfxInterface->sprDropDownBar[2] = new hgeSprite(texMain, 883, 736, 21, 22);
	hGfxInterface->sprDropDownButtonFocused = new hgeSprite(texMain, 883, 736 + 22, 21, 22);
	hGfxInterface->sprDropDownBarArrow = new hgeSprite(texMain, 846, 758, 9, 7);

	hGfxInterface->sprMainShadeBar = new hgeSprite(texMain, 896, 887, 37, 9);

	Console->Print("   Various...");

	sprConsoleBG = new hgeSprite(texMain, 512, 0, 128, 128);
	Console->SetBG(sprConsoleBG);

	SYSTEMTIME time;
	GetLocalTime(&time);
	if (time.wMonth == 12 && time.wDay > 15 || time.wMonth == 1 && time.wDay < 3)
		sprLogoBig = new hgeSprite(texMain, 0, 640, 299, 81);
	else
		sprLogoBig = new hgeSprite(texMain, 512, 151, 302, 76);

	sprBlank = new hgeSprite(texMain, 26, 0, 6, 6);

	sprLogoMini = new hgeSprite(texMain, 0, 487, 99, 25);
	sprIconCaption = new hgeSprite(texMain, 160, 231, 19, 22);

	sprCaptionMinimalize[0] = new hgeSprite(texMain, 50, 306, 20, 20);
	sprCaptionClose[0] = new hgeSprite(texMain, 70, 306, 21, 20);
	sprCaptionMinimalize[1] = new hgeSprite(texMain, 50, 346, 20, 20);
	sprCaptionClose[1] = new hgeSprite(texMain, 70, 346, 21, 20);

	sprCheckboard = new hgeSprite(texMain, 640, 0, 120, 120);

	sprThumbtack = new hgeSprite(texMain, 448, 640, 64, 111);

	for (int i = 0; i < 2; i++)
		sprMicroIcons[i] = new hgeSprite(texMain, 80 + 18 * i, 250, 18, 18);

	for (int i = 0; i < 14; i++)
		sprLevelsMicro16[i] = new hgeSprite(texMain, 512 + 16 * i, 512, 16, 16);

	/*int desclen;
	char * desc = (char*)repo.GetFileAsRawData("fonts/myriad/80px.fnt", &desclen);
	ptr = repo.GetFileAsRawData("fonts/myriad/80px.png", &size);
	HTEXTURE tex = hge->Texture_Load((const char*)ptr, size);
	printf("hge %d, tex %d\n", (int)hge, tex);
	printf("desc: %s\n", desc);
	fnt = new hgeFont(desc, desclen, tex, 1, 0, 0);
	test = new hgeSprite(tex, 0, 0, 512, 512);*/
	if (!strcmp(Lang->GetCode(), "RU")) {
		fntMyriad13 = SHR::LoadFontFromSFS(&repo, "16px_r.fnt", "16px_r.png");
	}
	else {
		fntMyriad13 = SHR::LoadFontFromSFS(&repo, "16px.fnt", "16px.png");
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
	gcnParts.gcnfntMyriad13 = new gcn::HGEImageFont(fntMyriad13, 0);
	gcnParts.gcnfntMyriad10 = new gcn::HGEImageFont(fntMyriad10, 0);
	gcnParts.gcnfntSystem = new gcn::HGEImageFont(fntSystem17, 0);

	gcn::Widget::setGlobalFont(gcnParts.gcnfntMyriad13);

	Console->Print("   UI assets...");

	gcnParts.sprWindowBG = new hgeSprite(texMain, 736, 256, 256, 256);

	gcnParts.sprGcnWinBarL = new hgeSprite(texMain, 0, 275, 3, 24);
	gcnParts.sprGcnWinBarM = new hgeSprite(texMain, 3, 275, 3, 24);
	gcnParts.sprGcnWinBarR = new hgeSprite(texMain, 47, 275, 3, 24);

	gcnParts.sprButBar[0][0] = new hgeSprite(texMain, 0, 6, 4, 33);
	gcnParts.sprButBar[0][1] = new hgeSprite(texMain, 4, 6, 4, 33);
	gcnParts.sprButBar[0][2] = new hgeSprite(texMain, 136, 6, 4, 33);
	gcnParts.sprButBarD[0][0] = new hgeSprite(texMain, 34, 39, 4, 33);
	gcnParts.sprButBarD[0][1] = new hgeSprite(texMain, 38, 39, 4, 33);
	gcnParts.sprButBarD[0][2] = new hgeSprite(texMain, 64, 39, 4, 33);
	gcnParts.sprButBarH[0][0] = new hgeSprite(texMain, 102, 39, 4, 33);
	gcnParts.sprButBarH[0][1] = new hgeSprite(texMain, 106, 39, 4, 33);
	gcnParts.sprButBarH[0][2] = new hgeSprite(texMain, 132, 39, 4, 33);
	gcnParts.sprButBarP[0][0] = new hgeSprite(texMain, 68, 39, 4, 33);
	gcnParts.sprButBarP[0][1] = new hgeSprite(texMain, 72, 39, 4, 33);
	gcnParts.sprButBarP[0][2] = new hgeSprite(texMain, 98, 39, 4, 33);

	gcnParts.sprButBar[1][0] = new hgeSprite(texMain, 92, 303, 4, 18);
	gcnParts.sprButBar[1][1] = new hgeSprite(texMain, 96, 303, 4, 18);
	gcnParts.sprButBar[1][2] = new hgeSprite(texMain, 106, 303, 4, 18);
	gcnParts.sprButBarD[1][0] = new hgeSprite(texMain, 110, 303, 4, 18);
	gcnParts.sprButBarD[1][1] = new hgeSprite(texMain, 114, 303, 4, 18);
	gcnParts.sprButBarD[1][2] = new hgeSprite(texMain, 124, 303, 4, 18);
	gcnParts.sprButBarH[1][0] = new hgeSprite(texMain, 146, 303, 4, 18);
	gcnParts.sprButBarH[1][1] = new hgeSprite(texMain, 150, 303, 4, 18);
	gcnParts.sprButBarH[1][2] = new hgeSprite(texMain, 160, 303, 4, 18);
	gcnParts.sprButBarP[1][0] = new hgeSprite(texMain, 128, 303, 4, 18);
	gcnParts.sprButBarP[1][1] = new hgeSprite(texMain, 132, 303, 4, 18);
	gcnParts.sprButBarP[1][2] = new hgeSprite(texMain, 142, 303, 4, 18);

	gcnParts.sprCur = new hgeSprite(texMain, 175, 135, 15, 22);
	gcnParts.sprCurHand = new hgeSprite(texMain, 158, 135, 17, 24);
	gcnParts.sprCurHand->SetHotSpot(6, 1);

	gcnParts.sprPBarFL = new hgeSprite(texMain, 50, 275, 3, 31);
	gcnParts.sprPBarFM = new hgeSprite(texMain, 53, 275, 3, 31);
	gcnParts.sprPBarFR = new hgeSprite(texMain, 59, 275, 3, 31);

	gcnParts.sprPBarEL = new hgeSprite(texMain, 72, 275, 3, 31);
	gcnParts.sprPBarEM = new hgeSprite(texMain, 64, 275, 3, 31);
	gcnParts.sprPBarER = new hgeSprite(texMain, 69, 275, 3, 31);

	gcnParts.sprButDDL = new hgeSprite(texMain, 48, 108, 3, 27);
	gcnParts.sprButDDC = new hgeSprite(texMain, 51, 108, 50, 27);
	gcnParts.sprButDDR = new hgeSprite(texMain, 196, 108, 26, 27);

	gcnParts.sprIconInfo = new hgeSprite(texMain, 320, 0, 64, 64);
	gcnParts.sprIconError = new hgeSprite(texMain, 384, 69, 64, 64);
	gcnParts.sprIconWarning = new hgeSprite(texMain, 320, 69, 64, 64);

	gcnParts.sprArrowWhiteDD = new hgeSprite(texMain, 48, 146, 9, 6);
	gcnParts.sprArrowGrayDD = new hgeSprite(texMain, 57, 146, 9, 6);

	gcnParts.sprIconClose = new hgeSprite(texMain, 153, 39, 20, 20);

	gcnParts.sprCBox = new hgeSprite(texMain, 0, 105, 13, 13);
	gcnParts.sprCBoxOn = new hgeSprite(texMain, 0, 118, 14, 13);

	gcnParts.sprRadio = new hgeSprite(texMain, 0, 131, 13, 14);
	gcnParts.sprRadioChecked = new hgeSprite(texMain, 0, 145, 13, 14);

	sprShadeBar = new hgeSprite(texMain, 24, 0, 2, 6);

	sprIconGoodSmall = new hgeSprite(texMain, 126, 74, 16, 16);
	sprIconBadSmall = new hgeSprite(texMain, 126, 90, 16, 16);

	sprMicroVert = new hgeSprite(texMain, 27, 138, 5, 9);
	sprMicroHor = new hgeSprite(texMain, 32, 138, 9, 5);
	sprMicroZoom = new hgeSprite(texMain, 32, 143, 10, 10);

	sprSmiley = new hgeSprite(texMain, 992, 193, 32, 32);
	sprSmiley->SetHotSpot(16, 16);

	sprDottedLineHorizontal = new hgeSprite(texMain, 765, 0, 102, 5);
	sprDottedLineVertical = new hgeSprite(texMain, 760, 0, 5, 102);

	sprArrowHorizontalL = new hgeSprite(texMain, 765, 5, 35, 49);
	sprArrowHorizontalC = new hgeSprite(texMain, 800, 5, 8, 49);
	sprArrowHorizontalR = new hgeSprite(texMain, 805, 5, 39, 49);

	sprArrowVerticalU = new hgeSprite(texMain, 867, 0, 48, 37);
	sprArrowVerticalM = new hgeSprite(texMain, 867, 37, 48, 128);
	sprArrowVerticalD = new hgeSprite(texMain, 867, 165, 48, 38);

	sprArrowVerticalM->SetHotSpot(22, 128);
	sprArrowVerticalU->SetHotSpot(22, 37);

	for (int i = 0; i < 4; i++)
		sprCloudTip[i] = new hgeSprite(texMain, 50 + 10 * (i == 1 || i == 3), 326 + 10 * (i > 1), 10, 10);

	Console->Print("   Fancy icons...");

	for (int y = 0; y < 2; y++)
		for (int x = 0; x < 6; x++)
			sprIcons[y * 6 + x] = new hgeSprite(texMain, 320 + x * 32, 133 + y * 32, 32, 32);
	for (int y = 0; y < 2; y++)
		for (int x = 0; x < 9; x++)
			sprIcons[y * 9 + x + 12] = new hgeSprite(texMain, 224 + x * 32, 197 + y * 32, 32, 32);
	for (int y = 0; y < 6; y++)
		for (int x = 0; x < 14; x++)
			sprIcons[x + 30 + y * 14] = new hgeSprite(texMain, 224 + x * 32, 261 + y * 32, 32, 32);

	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 16; x++)
			if (114 + x + y * 16 < GfxIcons_Count)
				sprIcons[114 + x + y * 16] = new hgeSprite(texMain, 512 + x * 32, 528 + y * 32, 32, 32);

	for (int y = 0; y < 3; y++) {
		for (int i = 0; i < 27; i++)
			if (y * 27 + i < Gfx16Icons_Count)
				sprIcons16[y * 27 + i] = new hgeSprite(texMain, 240 + i * 16, 453 + 16 * y, 16, 16);
	}
	for (int x = 0; x < 18; x++)
		sprIcons16[81 + x] = new hgeSprite(texMain, 736 + 16 * x, 512, 16, 16);
	for (int x = 0; x < 32; x++)
		sprIcons16[99 + x] = new hgeSprite(texMain, 512 + 16 * x, 720, 16, 16);
	for (int x = 0; x < 2; x++)
		sprIcons16[131 + x] = new hgeSprite(texMain, 512 + 16 * x, 736, 16, 16);

	for (int i = 0; i < 4; i++) {
		sprGamesBig[i] = new hgeSprite(texMain, 190, 135 + 32 * i, 32, 32);
		sprGamesSmall[i] = new hgeSprite(texMain, 126 + 16 * i, 215, 16, 16);
	}

	sprTile = new hgeSprite(texMain, 384, 640, 64, 64);
	sprTile->SetColor(0x99FFFFFF);

	for (int i = 0; i < 4; i++)
		sprIcons128[i] = new hgeSprite(texMain, i * 128, 512, 128, 128);

	sprUsedLibs = new hgeSprite(texMain, 673, 269, 351, 243);

	sprContextCascadeArrow = new hgeSprite(texMain, 153, 79, 14, 16);

	sprDropShadowCorner = new hgeSprite(texMain, 26, 455, 11, 12);
	sprDropShadowLeft = new hgeSprite(texMain, 0, 455, 11, 7);
	sprDropShadowUp = new hgeSprite(texMain, 45, 455, 9, 10);
	sprDropShadowVert = new hgeSprite(texMain, 38, 455, 6, 6);
	sprDropShadowHor = new hgeSprite(texMain, 12, 455, 13, 7);

	IF = new SHR::Interface(&gcnParts);

	szClawPath = NULL;
	const char * pVal = NULL;
	pVal = ini->GetValue("Paths", "Claw", "");

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
			szClawPath = new char[1];
			szClawPath[0] = '\0';
		}
		else {
			DWORD keysnum = 0;
			RegQueryInfoKey(key, 0, 0, 0, &keysnum, 0, 0, 0, 0, 0, 0, 0);
			char * clawkey = NULL;
			for (int i = 0; i < keysnum; i++) {
				DWORD namelen = 255;
				TCHAR* keyname = new TCHAR[namelen];
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
					szClawPath = new char[MAX_PATH];
					DWORD size = MAX_PATH;
					RegQueryValueEx(clawkeyh, "InstallPath", 0, 0, (BYTE*)szClawPath, &size);
					RegCloseKey(clawkeyh);
					if (szClawPath[strlen(szClawPath) - 1] == '/' || szClawPath[strlen(szClawPath) - 1] == '\\')
						szClawPath[strlen(szClawPath) - 1] = '\0';
					Console->Printf("~g~Found install path (by RealNetworks): %s~w~", szClawPath);
					ini->SetValue("Paths", "Claw", szClawPath);
					ini->SaveFile("settings.cfg");
				}
				delete[] clawkey;
			}
			RegCloseKey(key);
		}
	}
	else {
		szClawPath = new char[strlen(pVal) + 1];
		strcpy(szClawPath, pVal);
	}

	if (szClawPath == NULL) {
		char tmp[768];
		bool fail = 0;

		Console->Printf("~w~Checking WM directory for Claw installation...");

		sprintf(tmp, "%s/CLAW.EXE", myDIR);
		FILE * f = fopen(tmp, "r");
		if (f) fclose(f);
		else    fail = 1;
		if (!fail) {
			szClawPath = new char[strlen(myDIR) + 1];
			strcpy(szClawPath, myDIR);
		}
		else {
			char * npath = SHR::GetDir(myDIR);
			Console->Printf("~w~Checking in parent directory...");
			sprintf(tmp, "%s/CLAW.EXE", npath);
			f = fopen(tmp, "r");
			if (f) fclose(f);
			else    fail = 1;
			if (!fail) {
				szClawPath = new char[strlen(npath) + 1];
				strcpy(szClawPath, npath);
			}
			delete[] npath;
		}

		if (szClawPath != NULL) {
			Console->Printf("~g~Found in ~y~%s~g~.", myDIR);
			ini->SetValue("Paths", "Claw", szClawPath);
			ini->SaveFile("settings.cfg");
			delete[] szClawPath;
		}
		else {
			Console->Printf("~r~Unable to find Claw installation.");
		}
	}

	pVal = ini->GetValue("RSS", "TCR");
	if (pVal == NULL) {
		szUrlTCR = new char[1];
		szUrlTCR[0] = '\0';
	}
	else {
		szUrlTCR = new char[strlen(pVal) + 1];
		strcpy(szUrlTCR, pVal);
	}

	pVal = ini->GetValue("WapMap", "LastOpenDir");
	if (pVal == NULL) {
		szLastOpenPath = new char[strlen(myDIR) + 1];
		strcpy(szLastOpenPath, myDIR);
	}
	else {
		szLastOpenPath = new char[strlen(pVal) + 1];
		strcpy(szLastOpenPath, pVal);
	}

	pVal = ini->GetValue("WapMap", "LastSaveDir");
	if (pVal == NULL) {
		szLastSavePath = new char[strlen(myDIR) + 1];
		strcpy(szLastSavePath, myDIR);
	}
	else {
		szLastSavePath = new char[strlen(pVal) + 1];
		strcpy(szLastSavePath, pVal);
	}

	bSmoothZoom = atoi(ini->GetValue("WapMap", "SmoothZoom", "1"));

	bRealSim = atoi(ini->GetValue("WapMap", "RealSim", "0"));

	bAutoUpdate = atoi(ini->GetValue("WapMap", "AutoUpdate", "1"));
	bAlphaHigherPlanes = atoi(ini->GetValue("WapMap", "AlphaOverlapping ", "0"));

	bool enableconsole = atoi(ini->GetValue("WapMap", "Console", "0"));
	Console->Enable(enableconsole);

	sprKijan = new hgeSprite(texMain, 939, 0, 80, 105);
	sprKijan->SetHotSpot(40, 52);
	sprNapo = new hgeSprite(texMain, 961, 106, 63, 86);
	sprNapo->SetHotSpot(31, 43);

	bKijan = bNapo = 0;
    Console->AddModifiableBool("kijan", &bKijan);
    Console->AddModifiableBool("napoleon", &bNapo);
    Console->AddModifiableBool("realsim", &bRealSim);

	//SprBank = NULL;

	conL = NULL;
	ReloadLua();

	conLuaBuf = NULL;

	delete[] myDIR;

	//ini.SetValue("section", "key", "newvalue");

	//font = new gcn::ImageFont("data/fixedfont.png", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	//font = new gcn::HGEImageFont(fntSystem17);
	//font->getFont()->SetColor(0xFF000000);
	//gcn::Widget::setGlobalFont(font);

	colBase = 0xff4d4d4d;//0xff989898;
	colBaseDark = colBase - 0x000F0F0F;

	colLineDark = 0xFF0E0E0E;
	colLineBright = 0xFF2B2B2B;
	colOutline = colBase - 0x004d4d4d;

	colBaseGCN = 0x4d4d4d;//0x989898;
	colBaseDarkGCN = colBaseGCN - 0x0f0f0f;
	colLineDarkGCN = colBaseGCN - 0x2e2e2e;
	colLineBrightGCN = colBaseGCN + 0x0f0f0f;
	colOutlineGCN = colBaseGCN - 0x4d4d4d;
}

void cGlobals::ResetWD()
{
	char exe_name[MAX_PATH + 1];
	exe_name[MAX_PATH] = '\0';
	::GetModuleFileName(NULL, exe_name, MAX_PATH);
	char * myDIR = SHR::GetDir(exe_name);
	_chdir(myDIR);
	delete[] myDIR;
}

void cGlobals::SetLastOpenPath(const char * npath)
{
	delete[] szLastOpenPath;
	szLastOpenPath = new char[strlen(npath) + 1];
	strcpy(szLastOpenPath, npath);
	ini->SetValue("WapMap", "LastOpenDir", szLastOpenPath);
	ResetWD();
	ini->SaveFile("settings.cfg");
}

void cGlobals::SetLastSavePath(const char * npath)
{
	delete[] szLastSavePath;
	szLastSavePath = new char[strlen(npath) + 1];
	strcpy(szLastSavePath, npath);
	ini->SetValue("WapMap", "LastSaveDir", szLastSavePath);
	ResetWD();
	ini->SaveFile("settings.cfg");
}

void cGlobals::ExecuteLua(const char * script, bool popuponerror)
{
	GV->jmp_val = setjmp(GV->jmp_env);
	if (!GV->jmp_val) {
		if (luaL_dostring(conL, script)) {
			Console->Printf("~r~Lua parser error: %s~w~", lua_tostring(conL, -1));
			if (popuponerror) {
				StateMgr->Push(new State::Error(Lang->GetString("Strings", Lang_LuaError), lua_tostring(conL, -1), ST_ER_ICON_FATAL, ST_ER_BUT_OK, 0));
			}
		}
	}
}

void cGlobals::ReloadLua()
{
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

void cGlobals::RenderDropShadow(int x, int y, int w, int h, unsigned char alpha)
{
	sprDropShadowLeft->SetColor(SETA(0xFFFFFFFF, alpha));
	sprDropShadowHor->SetColor(SETA(0xFFFFFFFF, alpha));
	sprDropShadowCorner->SetColor(SETA(0xFFFFFFFF, alpha));
	sprDropShadowUp->SetColor(SETA(0xFFFFFFFF, alpha));
	sprDropShadowVert->SetColor(SETA(0xFFFFFFFF, alpha));

	sprDropShadowLeft->Render(x, y + h);
	sprDropShadowHor->RenderStretch(x + 11, y + h, x + w - 5, y + h + 6);
	sprDropShadowCorner->Render(x + w - 5, y + h - 5);
	sprDropShadowUp->Render(x + w, y);
	sprDropShadowVert->RenderStretch(x + w, y + 10, x + w + 6, y + h - 5);
}
