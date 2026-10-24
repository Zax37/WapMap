/*
** Haaf's Game Engine 1.8 compatibility interface
** Copyright (C) 2003-2007, Relish Games
**
** This trimmed interface is adapted for WapMap's SFML compatibility layer.
** See ../LICENSE.txt for the HGE license.
*/
#ifndef WAPMAP_HGE_COMPAT_H
#define WAPMAP_HGE_COMPAT_H

#include <Windows.h>
#include <memory>
#include <string>
#include <vector>

#define HGE_VERSION 0x182
#define HGE_CALL __stdcall

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#define M_PI_2 1.57079632679489661923f
#define M_PI_4 0.785398163397448309616f
#define M_1_PI 0.318309886183790671538f
#define M_2_PI 0.636619772367581343076f
#endif

#define ARGB(a, r, g, b) ((DWORD(a) << 24) + (DWORD(r) << 16) + (DWORD(g) << 8) + DWORD(b))
#define GETA(col) ((col) >> 24)
#define GETR(col) (((col) >> 16) & 0xFF)
#define GETG(col) (((col) >> 8) & 0xFF)
#define GETB(col) ((col) & 0xFF)
#define SETA(col, a) (((col) & 0x00FFFFFF) + (DWORD(a) << 24))
#define SETR(col, r) (((col) & 0xFF00FFFF) + (DWORD(r) << 16))
#define SETG(col, g) (((col) & 0xFFFF00FF) + (DWORD(g) << 8))
#define SETB(col, b) (((col) & 0xFFFFFF00) + DWORD(b))

#define BLEND_COLORADD 1
#define BLEND_COLORMUL 0
#define BLEND_ALPHABLEND 2
#define BLEND_ALPHAADD 0
#define BLEND_ZWRITE 4
#define BLEND_NOZWRITE 0
#define BLEND_DEFAULT (BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE)
#define BLEND_DEFAULT_Z (BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_ZWRITE)

#define HGEFPS_UNLIMITED 0
#define HGEFPS_VSYNC (-1)

struct TextureEntry;
struct TargetEntry;
struct EffectEntry;
struct ChannelEntry;
struct SurfaceEntry;

using HTEXTURE = TextureEntry*;
using HTARGET = TargetEntry*;
using HEFFECT = EffectEntry*;
using HCHANNEL = ChannelEntry*;
using HSURFACE = SurfaceEntry*;

enum hgeBoolState {
    HGE_WINDOWED = 1,
    HGE_ZBUFFER = 2,
    HGE_TEXTUREFILTER = 3,
    HGE_USESOUND = 4,
    HGE_DONTSUSPEND = 5,
    HGE_HIDEMOUSE = 6,
    HGE_SHOWSPLASH = 7,
    HGE_WINDOWCAPTION = 30,
    HGE_ACCEPTFILES = 31
};

enum hgeFuncState {
    HGE_FRAMEFUNC = 8,
    HGE_RENDERFUNC = 9,
    HGE_FOCUSLOSTFUNC = 10,
    HGE_FOCUSGAINFUNC = 11,
    HGE_GFXRESTOREFUNC = 12,
    HGE_EXITFUNC = 13,
    HGE_FILEDRAGINFUNC = 32,
    HGE_FILEDRAGOUTFUNC = 33,
    HGE_FILEDROPFUNC = 34,
    HGE_RESIZEFUNC = 35,
    HGE_FULLSCREENTOGGLEFUNC = 37
};

enum hgeHwndState {
    HGE_HWND = 15,
    HGE_HWNDPARENT = 16
};

enum hgeIntState {
    HGE_SCREENWIDTH = 17,
    HGE_SCREENHEIGHT = 18,
    HGE_SCREENBPP = 19,
    HGE_SAMPLERATE = 20,
    HGE_FXVOLUME = 21,
    HGE_MUSVOLUME = 22,
    HGE_STREAMVOLUME = 23,
    HGE_FPS = 24,
    HGE_POWERSTATUS = 25,
    HGE_BORDER_WIDTH = 36
};

enum hgeStringState {
    HGE_ICON = 26,
    HGE_TITLE = 27,
    HGE_INIFILE = 28,
    HGE_LOGFILE = 29
};

using hgeCallback = bool (*)();

enum hgeKeyCode_t {
    HGEK_NO_KEY = 0x00,
    HGEK_LBUTTON = 0x01, HGEK_RBUTTON = 0x02, HGEK_MBUTTON = 0x04,
    HGEK_BACKSPACE = 0x08, HGEK_TAB = 0x09, HGEK_ENTER = 0x0D,
    HGEK_SHIFT = 0x10, HGEK_CTRL = 0x11, HGEK_ALT = 0x12,
    HGEK_PAUSE = 0x13, HGEK_CAPSLOCK = 0x14, HGEK_ESCAPE = 0x1B,
    HGEK_SPACE = 0x20, HGEK_PGUP = 0x21, HGEK_PGDN = 0x22,
    HGEK_END = 0x23, HGEK_HOME = 0x24, HGEK_LEFT = 0x25,
    HGEK_UP = 0x26, HGEK_RIGHT = 0x27, HGEK_DOWN = 0x28,
    HGEK_PRTSCR = 0x2C, HGEK_INSERT = 0x2D, HGEK_DELETE = 0x2E,
    HGEK_0 = 0x30, HGEK_1 = 0x31, HGEK_2 = 0x32, HGEK_3 = 0x33,
    HGEK_4 = 0x34, HGEK_5 = 0x35, HGEK_6 = 0x36, HGEK_7 = 0x37,
    HGEK_8 = 0x38, HGEK_9 = 0x39,
    HGEK_A = 0x41, HGEK_B = 0x42, HGEK_C = 0x43, HGEK_D = 0x44,
    HGEK_E = 0x45, HGEK_F = 0x46, HGEK_G = 0x47, HGEK_H = 0x48,
    HGEK_I = 0x49, HGEK_J = 0x4A, HGEK_K = 0x4B, HGEK_L = 0x4C,
    HGEK_M = 0x4D, HGEK_N = 0x4E, HGEK_O = 0x4F, HGEK_P = 0x50,
    HGEK_Q = 0x51, HGEK_R = 0x52, HGEK_S = 0x53, HGEK_T = 0x54,
    HGEK_U = 0x55, HGEK_V = 0x56, HGEK_W = 0x57, HGEK_X = 0x58,
    HGEK_Y = 0x59, HGEK_Z = 0x5A, HGEK_LWIN = 0x5B, HGEK_RWIN = 0x5C,
    HGEK_APPS = 0x5D,
    HGEK_NUMPAD0 = 0x60, HGEK_NUMPAD1 = 0x61, HGEK_NUMPAD2 = 0x62,
    HGEK_NUMPAD3 = 0x63, HGEK_NUMPAD4 = 0x64, HGEK_NUMPAD5 = 0x65,
    HGEK_NUMPAD6 = 0x66, HGEK_NUMPAD7 = 0x67, HGEK_NUMPAD8 = 0x68,
    HGEK_NUMPAD9 = 0x69, HGEK_MULTIPLY = 0x6A, HGEK_ADD = 0x6B,
    HGEK_SUBTRACT = 0x6D, HGEK_DECIMAL = 0x6E, HGEK_DIVIDE = 0x6F,
    HGEK_F1 = 0x70, HGEK_F2 = 0x71, HGEK_F3 = 0x72, HGEK_F4 = 0x73,
    HGEK_F5 = 0x74, HGEK_F6 = 0x75, HGEK_F7 = 0x76, HGEK_F8 = 0x77,
    HGEK_F9 = 0x78, HGEK_F10 = 0x79, HGEK_F11 = 0x7A, HGEK_F12 = 0x7B,
    HGEK_NUMLOCK = 0x90, HGEK_SCROLLLOCK = 0x91,
    HGEK_SEMICOLON = 0xBA, HGEK_EQUALS = 0xBB, HGEK_COMMA = 0xBC,
    HGEK_MINUS = 0xBD, HGEK_PERIOD = 0xBE, HGEK_SLASH = 0xBF,
    HGEK_GRAVE = 0xC0, HGEK_LBRACKET = 0xDB, HGEK_BACKSLASH = 0xDC,
    HGEK_RBRACKET = 0xDD, HGEK_APOSTROPHE = 0xDE
};

#define INPUT_KEYDOWN 1
#define INPUT_KEYUP 2
#define INPUT_MBUTTONDOWN 3
#define INPUT_MBUTTONUP 4
#define INPUT_MOUSEMOVE 5
#define INPUT_MOUSEWHEEL 6
#define HGEINP_SHIFT 1
#define HGEINP_CTRL 2
#define HGEINP_ALT 4
#define HGEINP_CAPSLOCK 8
#define HGEINP_SCROLLLOCK 16
#define HGEINP_NUMLOCK 32
#define HGEINP_REPEAT 64

struct hgeVertex {
    float x, y, z;
    DWORD col;
    float tx, ty;
};

struct hgeTriple {
    hgeVertex v[3];
    HTEXTURE tex;
    int blend;
};

struct hgeQuad {
    hgeVertex v[4];
    HTEXTURE tex;
    int blend;
};

struct hgeInputEvent {
    int type;
    int key;
    int flags;
    int chr;
    int wheel;
    float x;
    float y;
};

class HGE {
public:
    HGE();
    virtual ~HGE();
    void HGE_CALL Release();

    bool HGE_CALL System_Initiate();
    void HGE_CALL System_Shutdown();
    bool HGE_CALL System_Start();
    char* HGE_CALL System_GetErrorMessage();
    void HGE_CALL System_Log(const char* format, ...);
    std::vector<std::string>& HGE_CALL System_GetDraggedFiles();
    void HGE_CALL System_GetDroppedFilesPosition(int* x, int* y);
    void HGE_CALL System_DoManualMainLoop();

    void HGE_CALL System_SetState(hgeBoolState state, bool value);
    void HGE_CALL System_SetState(hgeFuncState state, hgeCallback value);
    void HGE_CALL System_SetState(hgeHwndState state, HWND value);
    void HGE_CALL System_SetState(hgeIntState state, int value);
    void HGE_CALL System_SetState(hgeStringState state, const char* value);
    bool HGE_CALL System_GetState(hgeBoolState state);
    hgeCallback HGE_CALL System_GetState(hgeFuncState state);
    HWND HGE_CALL System_GetState(hgeHwndState state);
    int HGE_CALL System_GetState(hgeIntState state);
    const char* HGE_CALL System_GetState(hgeStringState state);

    void* HGE_CALL Resource_Load(const char* filename, DWORD* size = nullptr);
    void HGE_CALL Resource_Free(void* resource);
    int HGE_CALL Random_Int(int min, int max);
    float HGE_CALL Random_Float(float min, float max);
    float HGE_CALL Timer_GetTime();
    float HGE_CALL Timer_GetDelta();
    float HGE_CALL Timer_GetDeltaRealtime();
    int HGE_CALL Timer_GetFPS();

    HEFFECT HGE_CALL Effect_Load(const char* filename, DWORD size = 0);
    void HGE_CALL Effect_Free(HEFFECT effect);
    HCHANNEL HGE_CALL Effect_Play(HEFFECT effect);
    HCHANNEL HGE_CALL Effect_PlayEx(HEFFECT effect, int volume = 100, int pan = 0,
                                    float pitch = 1.0f, bool loop = false);
    void HGE_CALL Channel_SetVolume(HCHANNEL channel, int volume);
    void HGE_CALL Channel_Pause(HCHANNEL channel);
    void HGE_CALL Channel_Resume(HCHANNEL channel);
    void HGE_CALL Channel_Stop(HCHANNEL channel);
    bool HGE_CALL Channel_IsPlaying(HCHANNEL channel);
    float HGE_CALL Channel_GetLength(HCHANNEL channel);
    float HGE_CALL Channel_GetPos(HCHANNEL channel);
    void HGE_CALL Channel_SetPos(HCHANNEL channel, float seconds);

    void HGE_CALL Input_GetMousePos(float* x, float* y);
    void HGE_CALL Input_SetMousePos(float x, float y);
#ifdef WAPMAP_ENABLE_IMGUI
    void HGE_CALL Input_SetMouseStateOverride(float x, float y, bool left, bool right,
                                              bool middle, int wheel);
    void HGE_CALL Input_GetPhysicalMousePos(float* x, float* y);
    void HGE_CALL Input_GetPhysicalMouseScreenPos(float* x, float* y);
    void HGE_CALL Input_GetPhysicalMouseButtons(bool* left, bool* right, bool* middle);
    void HGE_CALL Input_SetKeyboardCaptureOverride(bool enabled);
#endif
    int HGE_CALL Input_GetMouseWheel();
    bool HGE_CALL Input_IsMouseOver();
    bool HGE_CALL Input_KeyDown(int key);
    bool HGE_CALL Input_KeyUp(int key);
    bool HGE_CALL Input_GetKeyState(int key);
    int HGE_CALL Input_GetChar();
    bool HGE_CALL Input_GetEvent(hgeInputEvent* event);

    bool HGE_CALL Gfx_BeginScene(HTARGET target = nullptr);
    void HGE_CALL Gfx_EndScene();
    void HGE_CALL Gfx_Clear(DWORD color);
    void HGE_CALL Gfx_FlushBuffer();
    void HGE_CALL Gfx_RenderLine(float x1, float y1, float x2, float y2,
                                 DWORD color = 0xFFFFFFFF, float z = 0.5f);
    void HGE_CALL Gfx_RenderTriple(const hgeTriple* triple);
    void HGE_CALL Gfx_RenderQuad(const hgeQuad* quad, bool filled = true);
    void HGE_CALL Gfx_SetClipping(int x = 0, int y = 0, int width = 0, int height = 0);
    void HGE_CALL Gfx_GetClipping(int* x = nullptr, int* y = nullptr,
                                  int* width = nullptr, int* height = nullptr);
    void HGE_CALL Gfx_SetTransform(float x = 0, float y = 0, float dx = 0, float dy = 0,
                                   float rotation = 0, float hscale = 0, float vscale = 0);

    HTARGET HGE_CALL Target_Create(int width, int height, bool zbuffer = false);
    void HGE_CALL Target_Free(HTARGET target);
    HTEXTURE HGE_CALL Target_GetTexture(HTARGET target);
    HSURFACE HGE_CALL Target_GetSurface(HTARGET target);
    DWORD* HGE_CALL Surface_Lock(HSURFACE surface, bool readOnly = true,
                                 int left = 0, int top = 0, int width = 0, int height = 0);
    HTEXTURE HGE_CALL Texture_Create(int width, int height);
    HTEXTURE HGE_CALL Texture_Load(const char* filename, DWORD size = 0, bool mipmap = false);
    void HGE_CALL Texture_Free(HTEXTURE texture);
    int HGE_CALL Texture_GetWidth(HTEXTURE texture, bool original = false);
    int HGE_CALL Texture_GetHeight(HTEXTURE texture, bool original = false);
    unsigned int HGE_CALL Texture_GetNativeHandle(HTEXTURE texture) const;
    DWORD* HGE_CALL Texture_Lock(HTEXTURE texture, bool readOnly = true,
                                 int left = 0, int top = 0, int width = 0, int height = 0);
    void HGE_CALL Texture_Unlock(HTEXTURE texture);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    bool RunOneFrame();
};

extern "C" HGE* HGE_CALL hgeCreate(int version);

#endif
