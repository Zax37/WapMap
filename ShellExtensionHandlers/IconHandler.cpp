/****************************** Module Header ******************************\
Module Name:  IconHandler.cpp
Copyright (c) Microsoft Corporation.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include "IconHandler.h"

#include <Shlwapi.h>

extern HINSTANCE  g_hInstDll;

#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP IconHandler::QueryInterface(REFIID riid, void **ppv)
{
  static const QITAB qit[] =
  {
    QITABENT(IconHandler, IPersistFile),
    QITABENT(IconHandler, IExtractIcon),
    { 0 },
  };
  return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) IconHandler::AddRef()
{
  return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) IconHandler::Release()
{
  ULONG cRef = InterlockedDecrement(&m_cRef);
  if (0 == cRef)
  {
    delete this;
  }

  return cRef;
}

#pragma endregion


#pragma region IExtractIcon

STDMETHODIMP IconHandler::GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax, int* piIndex, UINT* pwFlags)
{
	DWORD     dwFileSizeLo, dwFileSizeHi;
	DWORDLONG qwSize;
	HANDLE    hFile;

	hFile = CreateFile(m_szFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == hFile)
		return S_FALSE;    // tell the shell to use a default icon

	dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	CloseHandle(hFile);

	if ((DWORD)-1 == dwFileSizeLo && GetLastError() != NO_ERROR)
		return S_FALSE;    // tell the shell to use a default icon

	qwSize = DWORDLONG(dwFileSizeHi) << 32 | dwFileSizeLo;

	TCHAR szModulePath[MAX_PATH];

	GetModuleFileName(g_hInstDll, szModulePath, MAX_PATH);

	lstrcpyn(szIconFile, szModulePath, cchMax);

	if (0 == qwSize)
		*piIndex = 0;
	else if (qwSize < 4096)
		*piIndex = 1;
	else if (qwSize < 8192)
		*piIndex = 2;
	else
		*piIndex = 3;

	*pwFlags = 0;
	return S_OK;
}

STDMETHODIMP IconHandler::Extract(LPCTSTR pszFile, UINT nIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT nIconSize)
{
	return S_FALSE;
}

#pragma endregion

