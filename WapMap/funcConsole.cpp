#include "funcConsole.h"
#include "globals.h"

#ifdef BUILD_DEBUG
#include <psapi.h>
#endif

#include <direct.h>

void ConMoo(int argc, char * argv[])
{
	GV->Console->Print("                            ..---.");
	GV->Console->Print("                            // |\\||\\");
	GV->Console->Print("    \\   /                 ///;||\\|;\\               \\   /");
	GV->Console->Print("      o               .__// \\\\____//\\_,.             o");
	GV->Console->Print("    /   \\              Y{_\\_/  \'' = __/           /   \\");
	GV->Console->Print("                       \\___   (o) (o)  }       /");
	GV->Console->Print("                          /         :--'   SACRE MOO!");
	GV->Console->Print("                     .---/ \\_    `__\\-.");
	GV->Console->Print("                    /     `--\\___(o'o) \\");
	GV->Console->Print("             \\   / |     \\      `===='  |\\   /");
	GV->Console->Print("               o    `.    `.    .'    .'   o");
	GV->Console->Print("             /   \\   (`.    `. '    .')  /   \\");
	GV->Console->Print("                     (  `.    `...'   )");
	GV->Console->Print("                     (   .+.    `-.   )");
	GV->Console->Print("                     (.-'  .>-._   `-.)");
	GV->Console->Print("    \\   /           (___.-'      `-.___)            \\   /");
	GV->Console->Print("      o              (                )               o");
	GV->Console->Print("    /   \\            (                )             /   \\");
}

void ConHelp(int argc, char * argv[])
{
	GV->Console->Help(argc, argv);
}

void ConVar(int argc, char * argv[])
{
	GV->Console->ListVars();
}

void ConVarSet(int argc, char * argv[])
{
	GV->Console->SetVar(argc, argv);
}

void ConVarGet(int argc, char * argv[])
{
	GV->Console->GetVar(argc, argv);
}

void ConLua(int argc, char * argv[])
{
	int cmdlen = 0;
	for (int i = 0; i < argc; i++)
		cmdlen += strlen(argv[i]) + 1;
	char* cmd = new char[cmdlen];
	cmd[0] = '\0';
	for (int i = 0; i < argc; i++) {
		strcat(cmd, argv[i]);
		if (i + 1 < argc)
			strcat(cmd, " ");
	}
	if (cmd[strlen(cmd) - 1] == '\\') {
		cmd[strlen(cmd) - 1] = '\0';
		if (GV->conLuaBuf == NULL) {
			GV->conLuaBuf = new char[strlen(cmd) + 1];
			GV->conLuaBuf[0] = '\0';
			strcpy(GV->conLuaBuf, cmd);
		}
		else {
			char * tmp = GV->conLuaBuf;
			GV->conLuaBuf = new char[strlen(tmp) + strlen(cmd) + 2];
			sprintf(GV->conLuaBuf, "%s\n%s", tmp, cmd);
			delete[] tmp;
		}
		GV->Console->Print(cmd);
	}
	else {
		//execute
		char * tmp = new char[strlen(GV->conLuaBuf) + 2 + strlen(cmd)];
		sprintf(tmp, "%s\n%s", GV->conLuaBuf, cmd);
		GV->ExecuteLua(tmp, 0);
		delete[] tmp;
		delete[] GV->conLuaBuf;
		GV->conLuaBuf = NULL;
	}
	delete[] cmd;
}

void ConFlushLua(int argc, char * argv[])
{
	if (GV->conLuaBuf != NULL) {
		delete[] GV->conLuaBuf;
		GV->conLuaBuf = NULL;
		GV->ReloadLua();
	}
	GV->Console->Printf("~g~Lua flushed.~w~");
}

void ConLuaGetBuf(int argc, char * argv[])
{
	if (GV->conLuaBuf == NULL)
		GV->Console->Print("Buffer is empty.");
	else
		GV->Console->Printf("~y~Buffer contains:~w~\n%s", GV->conLuaBuf);
}

void ConCWD(int argc, char * argv[])
{
	char tmp[MAX_PATH];
	_getcwd(tmp, MAX_PATH);
	GV->Console->Printf("CWD: ~y~%s~w~", tmp);
}

void ConShader(int argc, char * argv[])
{
	GV->StateMgr->ReloadShader();
}

#ifdef BUILD_DEBUG
void ConMemInfo(int argc, char * argv[])
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GV->dwProcID);
	PROCESS_MEMORY_COUNTERS pmc;
	if (NULL == hProcess) {
		GV->Console->Printf("~r~Access fail.~w~");
		return;
	}
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
		GV->Console->Printf("PageFaultCount: 0x%08X [%ld]", pmc.PageFaultCount, pmc.PageFaultCount);
		GV->Console->Printf("PeakWorkingSetSize: 0x%08X [%ld]", pmc.PeakWorkingSetSize, pmc.PeakWorkingSetSize);
		GV->Console->Printf("WorkingSetSize: 0x%08X [%ld]", pmc.WorkingSetSize, pmc.WorkingSetSize);
		GV->Console->Printf("QuotaPeakPagedPoolUsage: 0x%08X [%ld]", pmc.QuotaPeakPagedPoolUsage, pmc.QuotaPeakPagedPoolUsage);
		GV->Console->Printf("QuotaPagedPoolUsage: 0x%08X [%ld]", pmc.QuotaPagedPoolUsage, pmc.QuotaPagedPoolUsage);
		GV->Console->Printf("QuotaPeakNonPagedPoolUsage: 0x%08X [%ld]", pmc.QuotaPeakNonPagedPoolUsage, pmc.QuotaPeakNonPagedPoolUsage);
		GV->Console->Printf("QuotaNonPagedPoolUsage: 0x%08X [%ld]", pmc.QuotaNonPagedPoolUsage, pmc.QuotaNonPagedPoolUsage);
		GV->Console->Printf("PagefileUsage: 0x%08X [%ld]", pmc.PagefileUsage, pmc.PagefileUsage);
		GV->Console->Printf("PeakPagefileUsage: 0x%08X [%ld]", pmc.PeakPagefileUsage, pmc.PeakPagefileUsage);
	}
	CloseHandle(hProcess);
}
#endif
