#ifndef H_C_BRUSH
#define H_C_BRUSH

#include "../shared/cWWD.h"
#include "cTileGhost.h"

#include "../shared/gcnWidgets/wLabel.h"
#include "../shared/gcnWidgets/wWin.h"
#include "../shared/gcnWidgets/wScrollArea.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#define BRUSH_EMPTY -2
#define BRUSH_FILLED -1
#define BRUSH_OUTOFBOUND -3

#define BRUSHICON_INVALID -1

int wmRandInt(lua_State *L);

int wmPlaceTile(lua_State *L);

int wmGetTile(lua_State *L);

int Brush_Panic(lua_State *L);

enum enBrushStatus {
    BrushOK,
    BrushLuaError,
    BrushWrongGame,
    BrushWrongLevel,
    BrushWrongLayer,
    BrushOtherError
};

class cTileImageSet;

enum enBrushSettingType {
    BrushSetUnknown = 0,
    BrushSetCheckbox,
    BrushSetInput,
    BrushSetDropdown
};

struct cBrushSetting {
    SHR::Lab *labDesc;
    gcn::Widget *hWidg;
    enBrushSettingType iType;
};

class cBrush {
private:
    SHR::Container *hconSettings;
    std::vector<cBrushSetting *> vSettings;
    char *m_szErrorInfo;
    int m_iBaseLvl;
    char *m_szName, *m_szFilename;
    int m_iIconTile;
    lua_State *m_L;
    cTileImageSet *m_hOwner;
    enBrushStatus m_enStatus;
    char *m_szLayerName;

    friend class cTileImageSet;

public:
    cBrush(const char *pszName, WWD::Parser *m_hParser, std::vector<std::string> vstrLayers, const char *ptr = NULL);

    ~cBrush();

    void AddSetting(cBrushSetting *n);

    bool HasSettings() { return vSettings.size() != 0; };

    void AddSettingsToContainer(SHR::Container *con, int sx, int sy);

    void RemoveSettingsFromContainer(SHR::Container *con);

    int GetSettingsHeight();

    bool Apply(WWD::Plane *phPlane, int piX, int piY);

    void ApplyGhosting(WWD::Plane *phPlane, int piX, int piY, std::vector<TileGhost> *vec);

    void SetOwner(cTileImageSet *n) { m_hOwner = n; };

    const char *GetName() { return (const char *) m_szName; };

    int GetIcon() { return m_iIconTile; };

    const char *GetLayerName() { return m_szLayerName; };

    enBrushStatus GetStatus() { return m_enStatus; };

    const char *GetErrorInfo() { return m_szErrorInfo; };
};

#endif
