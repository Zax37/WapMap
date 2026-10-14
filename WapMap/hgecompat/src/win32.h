#ifndef WAPMAP_HGE_COMPAT_WIN32_H
#define WAPMAP_HGE_COMPAT_WIN32_H

#include <Windows.h>

class HGE;

bool InstallWin32WindowFeatures(HWND window, HGE* hge, bool acceptFiles);
void RemoveWin32WindowFeatures(HWND window);
void GetWin32DroppedFilesPosition(int* x, int* y);

#endif
