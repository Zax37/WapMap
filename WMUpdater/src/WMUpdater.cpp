#include <windows.h>
#include <cstdio>
#include <string>

#define UPDATE_DIRECTORY ".wm_update"

const char* files_all = "*";
const char* files_update = UPDATE_DIRECTORY "/*";
const char* save_settings = ".settings.cfg";

int main()
{
	Sleep(1000);
	rename(&save_settings[1], save_settings);

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA((LPCSTR)files_all, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (FindFileData.cFileName[0] == '.') continue;
			remove(FindFileData.cFileName);
		} while (FindNextFileA(hFind, &FindFileData) != 0);
	}

	hFind = FindFirstFileA((LPCSTR)files_update, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (!strcmp(FindFileData.cFileName, ".") || !strcmp(FindFileData.cFileName, "..")) continue;
			std::string filepath(files_update);
			filepath.pop_back();
			filepath += FindFileData.cFileName;
			rename(filepath.c_str(), FindFileData.cFileName);
		} while (FindNextFileA(hFind, &FindFileData) != 0);
	}

	rename(save_settings, &save_settings[1]);
	
	RemoveDirectoryA(UPDATE_DIRECTORY);

	ShellExecuteA(
		NULL,
		"open",
		"WapMap.exe",
		"",
		"",
		SW_SHOWNORMAL
	);

}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int) {
	return main();
}
 