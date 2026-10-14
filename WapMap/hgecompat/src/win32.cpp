/*
** WapMap HGE compatibility layer
** HGE-derived interface portions are covered by ../LICENSE.txt.
*/
#include "hge.h"
#include "win32.h"

#include <Windowsx.h>
#include <Shellapi.h>
#include <Ole2.h>
#include <ShlObj.h>

#include <string>
#include <vector>

namespace {

HGE* activeHge{};
WNDPROC previousWindowProc{};
bool oleInitialized{};
POINTL droppedFilesPosition{};
IDropTarget* registeredDropTarget{};

std::string WideToNarrow(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }
    const auto count = WideCharToMultiByte(CP_ACP, 0, value.data(),
                                           static_cast<int>(value.size()), nullptr, 0,
                                           nullptr, nullptr);
    if (count <= 0) {
        return {};
    }
    std::string result(static_cast<size_t>(count), '\0');
    WideCharToMultiByte(CP_ACP, 0, value.data(), static_cast<int>(value.size()),
                        result.data(), count, nullptr, nullptr);
    return result;
}

class FileDropTarget final : public IDropTarget {
public:
    explicit FileDropTarget(HGE* hge) : hge_(hge) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
        if (!object) {
            return E_POINTER;
        }
        *object = nullptr;
        if (iid == IID_IUnknown || iid == IID_IDropTarget) {
            *object = static_cast<IDropTarget*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return static_cast<ULONG>(InterlockedIncrement(&references_));
    }

    ULONG STDMETHODCALLTYPE Release() override {
        const auto references = InterlockedDecrement(&references_);
        if (references == 0) {
            delete this;
        }
        return static_cast<ULONG>(references);
    }

    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* object, DWORD, POINTL, DWORD* effect) override {
        if (!effect) {
            return E_POINTER;
        }
        *effect = DROPEFFECT_NONE;
        auto& files = hge_->System_GetDraggedFiles();
        files.clear();

        FORMATETC format{CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        if (!object || FAILED(object->QueryGetData(&format))) {
            return S_OK;
        }
        STGMEDIUM medium{};
        if (FAILED(object->GetData(&format, &medium))) {
            return S_OK;
        }

        const auto drop = static_cast<HDROP>(GlobalLock(medium.hGlobal));
        if (drop) {
            const auto count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
            std::vector<wchar_t> path(MAX_PATH);
            for (UINT index = 0; index < count; ++index) {
                const auto length = DragQueryFileW(drop, index, path.data(),
                                                   static_cast<UINT>(path.size()));
                if (length >= path.size()) {
                    path.resize(static_cast<size_t>(length) + 1);
                    DragQueryFileW(drop, index, path.data(), static_cast<UINT>(path.size()));
                }
                files.push_back(WideToNarrow(std::wstring(path.data())));
            }
            GlobalUnlock(medium.hGlobal);
        }
        ReleaseStgMedium(&medium);

        if (files.empty()) {
            return S_OK;
        }
        *effect = DROPEFFECT_COPY;
        if (const auto callback = hge_->System_GetState(HGE_FILEDRAGINFUNC)) {
            callback();
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragOver(DWORD, POINTL, DWORD* effect) override {
        if (effect) {
            *effect = hge_->System_GetDraggedFiles().empty() ? DROPEFFECT_NONE : DROPEFFECT_COPY;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeave() override {
        if (const auto callback = hge_->System_GetState(HGE_FILEDRAGOUTFUNC)) {
            callback();
        }
        hge_->System_GetDraggedFiles().clear();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Drop(IDataObject*, DWORD, POINTL point, DWORD* effect) override {
        if (effect) {
            *effect = hge_->System_GetDraggedFiles().empty() ? DROPEFFECT_NONE : DROPEFFECT_COPY;
        }
        droppedFilesPosition = point;
        if (!hge_->System_GetDraggedFiles().empty()) {
            if (const auto callback = hge_->System_GetState(HGE_FILEDROPFUNC)) {
                callback();
            }
        }
        hge_->System_GetDraggedFiles().clear();
        return S_OK;
    }

private:
    ~FileDropTarget() = default;
    LONG references_{1};
    HGE* hge_;
};

LRESULT CALLBACK WapProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_NCHITTEST: {
        if (!activeHge) {
            break;
        }
        const auto border = activeHge->System_GetState(HGE_BORDER_WIDTH);
        if (border <= 0) {
            return HTCLIENT;
        }

        RECT rect{};
        if (!GetWindowRect(window, &rect)) {
            break;
        }
        const auto x = GET_X_LPARAM(lparam) - rect.left;
        const auto y = GET_Y_LPARAM(lparam) - rect.top;
        const auto width = rect.right - rect.left;
        const auto height = rect.bottom - rect.top;
        const bool left = x < border;
        const bool right = x >= width - border;
        const bool top = y < border;
        const bool bottom = y >= height - border;

        if (top && left) return HTTOPLEFT;
        if (top && right) return HTTOPRIGHT;
        if (bottom && right) return HTBOTTOMRIGHT;
        if (bottom && left) return HTBOTTOMLEFT;
        if (left) return HTLEFT;
        if (right) return HTRIGHT;
        if (top) return HTTOP;
        if (bottom) return HTBOTTOM;
        return HTCLIENT;
    }
    case WM_NCCALCSIZE:
    case WM_NCPAINT:
        return 0;
    case WM_GETMINMAXINFO:
        if (const auto info = reinterpret_cast<MINMAXINFO*>(lparam)) {
            info->ptMinTrackSize.x = 1024;
            info->ptMinTrackSize.y = 550;
        }
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lparam) == HTCLIENT) {
            return TRUE;
        }
        break;
    case WM_SYSCOMMAND:
        if ((wparam & 0xFFF0) == SC_MAXIMIZE && activeHge) {
            if (const auto callback = activeHge->System_GetState(HGE_FULLSCREENTOGGLEFUNC)) {
                callback();
                return 0;
            }
        }
        break;
    case WM_NCDESTROY: {
        const auto previous = previousWindowProc;
        if (registeredDropTarget) {
            RevokeDragDrop(window);
            registeredDropTarget = nullptr;
        }
        if (oleInitialized) {
            OleUninitialize();
            oleInitialized = false;
        }
        const auto result = CallWindowProc(previous, window, message, wparam, lparam);
        SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(previousWindowProc));
        previousWindowProc = nullptr;
        activeHge = nullptr;
        return result;
    }
    }

    return CallWindowProc(previousWindowProc, window, message, wparam, lparam);
}

}

bool InstallWin32WindowFeatures(HWND window, HGE* hge, const bool acceptFiles) {
    if (!window || !hge || previousWindowProc) {
        return false;
    }

    activeHge = hge;
    const auto style = GetWindowLongPtr(window, GWL_STYLE);
    SetWindowLongPtr(window, GWL_STYLE,
                     style | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
    SetWindowPos(window, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    previousWindowProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WapProc)));
    if (!previousWindowProc) {
        activeHge = nullptr;
        return false;
    }

    if (acceptFiles) {
        const auto oleResult = OleInitialize(nullptr);
        if (FAILED(oleResult)) {
            RemoveWin32WindowFeatures(window);
            return false;
        }
        oleInitialized = true;
        auto* target = new FileDropTarget(hge);
        const auto registerResult = RegisterDragDrop(window, target);
        target->Release();
        if (FAILED(registerResult)) {
            RemoveWin32WindowFeatures(window);
            return false;
        }
        registeredDropTarget = target;
    }
    return true;
}

void RemoveWin32WindowFeatures(HWND window) {
    if (window && registeredDropTarget) {
        RevokeDragDrop(window);
        registeredDropTarget = nullptr;
    }
    if (oleInitialized) {
        OleUninitialize();
        oleInitialized = false;
    }
    if (window && previousWindowProc) {
        SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(previousWindowProc));
        previousWindowProc = nullptr;
    }
    activeHge = nullptr;
}

void GetWin32DroppedFilesPosition(int* x, int* y) {
    if (x) *x = droppedFilesPosition.x;
    if (y) *y = droppedFilesPosition.y;
}
