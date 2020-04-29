/****************************** Module Header ******************************\
Module Name:  IconHandler.h
Copyright (c) Microsoft Corporation.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include <comdef.h>
#include <shlobj.h>
#include <atlconv.h>


class IconHandler :
	public IPersistFile,
	public IExtractIcon
{
public:
  // IUnknown
  IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
  IFACEMETHODIMP_(ULONG) AddRef();
  IFACEMETHODIMP_(ULONG) Release();

  // IPersistFile
  STDMETHOD(GetClassID)(CLSID*) { return E_NOTIMPL; }
  STDMETHOD(IsDirty)() { return E_NOTIMPL; }
  STDMETHOD(Save)(LPCOLESTR, BOOL) { return E_NOTIMPL; }
  STDMETHOD(SaveCompleted)(LPCOLESTR) { return E_NOTIMPL; }
  STDMETHOD(GetCurFile)(LPOLESTR*) { return E_NOTIMPL; }
  STDMETHOD(Load)(LPCOLESTR wszFile, DWORD /*dwMode*/)
  {
	  USES_CONVERSION;
	  lstrcpyn(m_szFilename, OLE2CT(wszFile), MAX_PATH);
	  return S_OK;
  }

  // IExtractIcon
  STDMETHODIMP GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax,
	  int* piIndex, UINT* pwFlags);
  STDMETHODIMP Extract(LPCTSTR pszFile, UINT nIconIndex, HICON* phiconLarge,
	  HICON* phiconSmall, UINT nIconSize);

protected:
	// Reference count of component.
	long m_cRef;

	// Provided during initialization.
	TCHAR     m_szFilename[MAX_PATH];   // Full path to the file in question.
	DWORDLONG m_qwFileSize;             // File size; used by extraction method 2.
};
