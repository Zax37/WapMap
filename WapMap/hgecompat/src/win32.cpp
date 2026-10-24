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
#include <dwmapi.h>

#ifdef WAPMAP_ENABLE_IMGUI
#include <imgui_internal.h>
#endif

#include <string>
#include <vector>

namespace {

HGE* activeHge{};
WNDPROC previousWindowProc{};
bool oleInitialized{};
POINTL droppedFilesPosition{};
IDropTarget* registeredDropTarget{};

void DisableNativeFrame(HWND window) {
    // WS_THICKFRAME is needed for native resizing, but DWM must not draw its
    // own frame over the editor's custom chrome on hover/activation changes.
    const DWMNCRENDERINGPOLICY policy = DWMNCRP_DISABLED;
    DwmSetWindowAttribute(window, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));
}

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
    UpdateWin32ResizePreview(window, message, lparam);
    switch (message) {
    case WM_MOUSEACTIVATE:
        return MA_ACTIVATE;
    case WM_NCHITTEST: {
        if (!activeHge) {
            break;
        }
#ifdef WAPMAP_ENABLE_IMGUI
        if (ImGui::GetCurrentContext() &&
            ImGui::GetCurrentContext()->MovingWindow) {
            return HTCLIENT;
        }
#endif
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
    case WM_NCACTIVATE:
        return TRUE;
    case WM_DWMCOMPOSITIONCHANGED:
    case WM_THEMECHANGED:
        DisableNativeFrame(window);
        break;
    case WM_GETMINMAXINFO:
        if (const auto info = reinterpret_cast<MINMAXINFO*>(lparam)) {
            info->ptMinTrackSize.x = 1024;
            info->ptMinTrackSize.y = 550;
        }
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lparam) == HTCLIENT) {
            // Legacy widgets only override the cursor for special cases.
            // Restore the default before dispatching each new mouse movement.
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
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

void UpdateWin32ResizePreview(HWND window, UINT message, LPARAM lparam) {
    // The native sizing loop blocks SFML's render loop. Use an owned overlay
    // with a hollow region so previewing the bounds never redraws the editor.
    constexpr wchar_t property[] = L"WapMap.ResizePreview";
    auto preview = static_cast<HWND>(GetPropW(window, property));
    if (message == WM_EXITSIZEMOVE || message == WM_NCDESTROY) {
        if (preview) {
            RemovePropW(window, property);
            DestroyWindow(preview);
        }
        return;
    }
    if (message != WM_SIZING || !lparam) return;

    const auto& bounds = *reinterpret_cast<const RECT*>(lparam);
    const int width = bounds.right - bounds.left;
    const int height = bounds.bottom - bounds.top;
    if (width <= 0 || height <= 0) return;
    if (!preview) {
        preview = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
            L"STATIC", nullptr, WS_POPUP | SS_WHITERECT,
            bounds.left, bounds.top, width, height, window, nullptr,
            GetModuleHandleW(nullptr), nullptr);
        if (!preview) return;
        if (!SetPropW(window, property, preview)) {
            DestroyWindow(preview);
            return;
        }
        SetLayeredWindowAttributes(preview, 0, 230, LWA_ALPHA);
    }
    const int border = GetSystemMetrics(SM_CXSIZEFRAME);
    const int thickness = border > 2 ? border : 3;
    const auto region = CreateRectRgn(0, 0, width, height);
    const auto interior = CreateRectRgn(thickness, thickness,
                                       width - thickness, height - thickness);
    if (region && interior) {
        CombineRgn(region, region, interior, RGN_DIFF);
        if (!SetWindowRgn(preview, region, TRUE)) DeleteObject(region);
    } else if (region) {
        DeleteObject(region);
    }
    if (interior) DeleteObject(interior);
    SetWindowPos(preview, HWND_TOP, bounds.left, bounds.top, width, height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

bool InstallWin32WindowFeatures(HWND window, HGE* hge, const bool acceptFiles) {
    if (!window || !hge || previousWindowProc) {
        return false;
    }

    activeHge = hge;
    previousWindowProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WapProc)));
    if (!previousWindowProc) {
        activeHge = nullptr;
        return false;
    }

    // Install the custom-frame handler before requesting a frame recalculation
    // so Windows never briefly paints the standard frame during setup.
    DisableNativeFrame(window);
    const auto style = GetWindowLongPtr(window, GWL_STYLE);
    SetWindowLongPtr(window, GWL_STYLE,
                     style | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
    SetWindowPos(window, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

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
    UpdateWin32ResizePreview(window, WM_EXITSIZEMOVE, 0);
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
