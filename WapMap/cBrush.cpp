#include "cBrush.h"
#include "globals.h"
#include "globlua.h"
#include "langID.h"

#include "states/editing_ww.h"

#include "databanks/tiles.h"

extern HGE * hge;

bool GV_BRUSH_bSettingsInit;
cBrush * GV_BRUSH_ptr = NULL;
std::vector<TileGhost> * hGhostingBank = 0;
bool GV_BRUSH_bChanges;

int wmAddDropDownOption(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 2) {
		GV->Console->Printf("~r~Brush lua: not enough arguments to wmAddDropDownOption (require 2, given %d)", argc);
		return 0;
	}

	if (lua_tonumber(L, 1) == 0)
		return 0;
	cBrushSetting * ptr = (cBrushSetting*)(int)lua_tonumber(L, 1);
	const char * opt = lua_tostring(L, 2);

	((cDynamicListModel*)((SHR::DropDown*)ptr->hWidg)->getListModel())->Add(opt);
	if (((SHR::DropDown*)ptr->hWidg)->getSelected() < 0)
		((SHR::DropDown*)ptr->hWidg)->setSelected(0);
	return 0;
}

int wmGetSettingValue(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc == 0) {
		lua_pushinteger(L, 0);
		return 1;
	}

	cBrushSetting * ptr = (cBrushSetting*)(int)lua_tonumber(L, 1);
	if (!ptr) {
		lua_pushinteger(L, 0);
		return 1;
	}
	if (ptr->iType == BrushSetCheckbox) {
		lua_pushinteger(L, ((SHR::CBox*)ptr->hWidg)->isSelected());
	}
	else if (ptr->iType == BrushSetDropdown) {
		lua_pushinteger(L, ((SHR::DropDown*)ptr->hWidg)->getSelected());
	}
	else if (ptr->iType == BrushSetInput) {
		const char * tmp = ((SHR::TextField*)ptr->hWidg)->getText().c_str();
		lua_pushstring(L, tmp);
	}
	return 1;
}

int wmAddSetting(lua_State *L)
{
	if (GV_BRUSH_ptr == NULL) {
		lua_pushinteger(L, 0);
		return 1;
	}
	if (!GV_BRUSH_bSettingsInit) {
		GV->Console->Printf("~r~Brush lua: call to ~y~wmAddSetting ~r~out of ~y~cbInitSettings~r~!~w~");
		lua_pushinteger(L, 0);
		return 1;
	}
	int argc = lua_gettop(L);
	if (argc < 2) {
		GV->Console->Printf("~r~Brush lua: not enough arguments to wmAddSetting (require 2, given %d)", argc);
		lua_pushinteger(L, 0);
		return 1;
	}
	enBrushSettingType iType = (enBrushSettingType)(int)lua_tonumber(L, 1);
	const char * desc = lua_tostring(L, 2);

	gcn::Widget * widg = NULL;

	if (iType == BrushSetCheckbox) {
		SHR::CBox * cb = new SHR::CBox(GV->hGfxInterface, "");
		cb->adjustSize();
		widg = cb;
		if (argc == 3)
			if (lua_tonumber(L, 3) == 1)
				cb->setSelected(1);
	}
	else if (iType == BrushSetInput) {
		SHR::TextField * tf = NULL;
		if (argc >= 3)
			tf = new SHR::TextField(lua_tostring(L, 3));
		else
			tf = new SHR::TextField();
		tf->adjustHeight();
		tf->setWidth(100);
		widg = tf;
		if (argc == 4)
			if (lua_tonumber(L, 4) == 1)
				tf->SetNumerical(1);
	}
	else if (iType = BrushSetDropdown) {
		SHR::DropDown * dd = new SHR::DropDown();
		dd->SetGfx(&GV->gcnParts);
		dd->adjustHeight();
		dd->setWidth(100);
		dd->setListModel(new cDynamicListModel());
		widg = dd;
	}
	else {
		GV->Console->Printf("~r~Brush lua: unknown widget type: ~y~%d", int(iType));
		lua_pushinteger(L, 0);
		return 1;
	}

	cBrushSetting * set = new cBrushSetting;
	set->iType = iType;
	set->hWidg = widg;
	set->labDesc = new SHR::Lab(desc);
	set->labDesc->adjustSize();

	GV_BRUSH_ptr->AddSetting(set);

#ifdef BUILD_DEBUG
	GV->Console->Printf("~g~Brush Lua: generated setting widget (0x%p).", set);
#endif
	lua_pushinteger(L, (int)set);
	return 1;
}

int wmGetLangCode(lua_State *L)
{
	lua_pushstring(L, GETL2S("Info", "Code"));
	return 1;
}

int wmGetTile(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 3) {
		GV->Console->Printf("~r~Brush lua: not enough arguments to wmGetTile (require 4, given %d)", argc);
		return 0;
	}

	WWD::Plane * plane = (WWD::Plane*)(int)lua_tonumber(L, 1);
	int x = lua_tonumber(L, 2);
	int y = lua_tonumber(L, 3);

	if (hGhostingBank != NULL) {
		for (int i = 0; i < hGhostingBank->size(); i++) {
			if (hGhostingBank->at(i).x == x && hGhostingBank->at(i).y == y && hGhostingBank->at(i).pl == plane) {
				if (hGhostingBank->at(i).id == EWW_TILE_ERASE) {
					lua_pushinteger(L, BRUSH_EMPTY);
					return 1;
				}
				else if (hGhostingBank->at(i).id == EWW_TILE_FILL) {
					lua_pushinteger(L, BRUSH_FILLED);
					return 1;
				}
				else {
					lua_pushinteger(L, hGhostingBank->at(i).id);
					return 1;
				}
			}
		}
	}
	if (plane->GetTile(x, y) == NULL)
		lua_pushinteger(L, BRUSH_OUTOFBOUND);
	else
		if (plane->GetTile(x, y)->IsInvisible())
			lua_pushinteger(L, BRUSH_EMPTY);
		else if (plane->GetTile(x, y)->IsFilled())
			lua_pushinteger(L, BRUSH_FILLED);
		else
			lua_pushinteger(L, plane->GetTile(x, y)->GetID());

	return 1;
}

int wmPlaceTile(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 4) {
		GV->Console->Printf("~r~Brush lua: not enough arguments to wmPlaceTile (require 4, given %d)", argc);
		return 0;
	}


	WWD::Plane * plane = (WWD::Plane*)(int)lua_tonumber(L, 1);
	int x = lua_tonumber(L, 2);
	int y = lua_tonumber(L, 3);
	int tile = lua_tonumber(L, 4);

	if (x < 0 || y < 0 || x >= plane->GetPlaneWidth() || y >= plane->GetPlaneHeight()) {
		GV->Console->Printf("~r~Brush lua: invalid coords called in wmPlaceTile(%d, %d, %d) [valid range X:0-%d Y:0-%d", x, y, tile,
			plane->GetPlaneWidth() - 1,
			plane->GetPlaneHeight() - 1);
		return 0;
	}

	if (tile == BRUSH_OUTOFBOUND ||
		tile < 0 && tile != BRUSH_EMPTY && tile != BRUSH_FILLED ||
		GV->editState->hTileset->GetSet(plane->GetImageSet(0))->GetTile(tile) == NULL && tile != BRUSH_EMPTY && tile != BRUSH_FILLED) {
		GV->Console->Printf("~r~Brush lua: invalid ID called in wmPlaceTile(%d, %d, %d)", x, y, tile);
		return 0;
	}

	if (hGhostingBank != 0) {
		if (plane->GetTile(x, y)->IsInvisible() && tile == BRUSH_EMPTY) return 0;
		if (plane->GetTile(x, y)->IsFilled() && tile == BRUSH_FILLED) return 0;
		if (plane->GetTile(x, y)->GetID() == tile) return 0;
		TileGhost tg;
		tg.x = x;
		tg.y = y;
		if (tile == BRUSH_EMPTY)
			tg.id = EWW_TILE_ERASE;
		else if (tile == BRUSH_FILLED)
			tg.id = EWW_TILE_FILL;
		else
			tg.id = tile;
		tg.pl = plane;
		for (int i = 0; i < hGhostingBank->size(); i++)
			if (hGhostingBank->at(i).x == tg.x && hGhostingBank->at(i).y == tg.y && hGhostingBank->at(i).pl == tg.pl) {
				hGhostingBank->erase(hGhostingBank->begin() + i);
				i--;
			}
		hGhostingBank->push_back(tg);
		return 0;
	}

	if (tile == BRUSH_EMPTY)
		plane->GetTile(x, y)->SetInvisible(1);
	else if (tile == BRUSH_FILLED)
		plane->GetTile(x, y)->SetFilled(1);
	else
		plane->GetTile(x, y)->SetID(tile);
	GV_BRUSH_bChanges = 1;

	return 0;
}

int Brush_Panic(lua_State *L)
{
	GV->Console->Printf("~r~Brush LUA: %s~w~", lua_tostring(L, -1));
	lua_pop(L, 1);
	longjmp(GV->jmp_env, 1);
	return 0;
}

cBrush::cBrush(const char * pszName, WWD::Parser * m_hParser, std::vector<std::string> vstrLayers, const char * ptr)
{
	hconSettings = NULL;
	m_enStatus = BrushOK;
	m_L = NULL;
	m_szName = NULL;
	m_szFilename = NULL;
	m_szErrorInfo = NULL;
	m_szLayerName = NULL;

	m_L = luaL_newstate();

	luaL_openlibs(m_L);
	lua_atpanic(m_L, Brush_Panic);

	lua_pushinteger(m_L, BRUSH_EMPTY);
	lua_setglobal(m_L, "wmEMPTY");

	lua_pushinteger(m_L, BRUSH_FILLED);
	lua_setglobal(m_L, "wmFILLED");

	lua_pushinteger(m_L, BRUSH_OUTOFBOUND);
	lua_setglobal(m_L, "wmOUTOFBOUND");

	lua_pushinteger(m_L, WWD::Game_Unknown);
	lua_setglobal(m_L, "wmUNKNOWNGAME");
	lua_pushinteger(m_L, WWD::Game_Claw);
	lua_setglobal(m_L, "wmCLAW");
	lua_pushinteger(m_L, WWD::Game_Gruntz);
	lua_setglobal(m_L, "wmGRUNTZ");
	lua_pushinteger(m_L, WWD::Game_GetMedieval);
	lua_setglobal(m_L, "wmGETMEDIEVAL");

	lua_pushinteger(m_L, 0);
	lua_setglobal(m_L, "wmERROR");

	lua_register(m_L, "wmPlaceTile", wmPlaceTile);
	lua_register(m_L, "wmGetTile", wmGetTile);
	lua_register(m_L, "wmRandInt", gv_wmRandInt);
	lua_register(m_L, "wmAddSetting", wmAddSetting);
	lua_register(m_L, "wmGetSettingValue", wmGetSettingValue);
	lua_register(m_L, "wmAddDropDownOption", wmAddDropDownOption);
	lua_register(m_L, "wmGetLangCode", wmGetLangCode);
	lua_register(m_L, "wmLog", gv_wmLog);

	lua_register(m_L, "wmIsPlaneMain", gv_wmIsPlaneMain);

	lua_register(m_L, "wmCreateObject", gv_wmCreateObject);
	lua_register(m_L, "wmDeleteObject", gv_wmDeleteObject);
	lua_register(m_L, "wmGetObjectsInArea", gv_wmGetObjectsInArea);
	lua_register(m_L, "wmGetObjectName", gv_wmGetObjectName);
	lua_register(m_L, "wmGetObjectImageSet", gv_wmGetObjectImageSet);
	lua_register(m_L, "wmGetObjectLogic", gv_wmGetObjectLogic);
	lua_register(m_L, "wmGetObjectAnim", gv_wmGetObjectAnim);
	lua_register(m_L, "wmGetObjectParam", gv_wmGetObjectParam);

	lua_pushinteger(m_L, BrushSetCheckbox);
	lua_setglobal(m_L, "wmCheckbox");
	lua_pushinteger(m_L, BrushSetInput);
	lua_setglobal(m_L, "wmInput");
	lua_pushinteger(m_L, BrushSetDropdown);
	lua_setglobal(m_L, "wmDropDown");

	lua_pushinteger(m_L, 0);
	lua_setglobal(m_L, "wmobjID");
	lua_pushinteger(m_L, 1);
	lua_setglobal(m_L, "wmobjPosX");
	lua_pushinteger(m_L, 2);
	lua_setglobal(m_L, "wmobjPosY");
	lua_pushinteger(m_L, 3);
	lua_setglobal(m_L, "wmobjPosZ");
	lua_pushinteger(m_L, 4);
	lua_setglobal(m_L, "wmobjPosI");
	lua_pushinteger(m_L, 5);
	lua_setglobal(m_L, "wmobjScore");
	lua_pushinteger(m_L, 6);
	lua_setglobal(m_L, "wmobjPoints");
	lua_pushinteger(m_L, 7);
	lua_setglobal(m_L, "wmobjPowerup");
	lua_pushinteger(m_L, 8);
	lua_setglobal(m_L, "wmobjDamage");
	lua_pushinteger(m_L, 9);
	lua_setglobal(m_L, "wmobjSmarts");
	lua_pushinteger(m_L, 10);
	lua_setglobal(m_L, "wmobjHealth");
	lua_pushinteger(m_L, 11);
	lua_setglobal(m_L, "wmobjMinX");
	lua_pushinteger(m_L, 12);
	lua_setglobal(m_L, "wmobjMinY");
	lua_pushinteger(m_L, 13);
	lua_setglobal(m_L, "wmobjMaxX");
	lua_pushinteger(m_L, 14);
	lua_setglobal(m_L, "wmobjMaxY");
	lua_pushinteger(m_L, 15);
	lua_setglobal(m_L, "wmobjSpeedX");
	lua_pushinteger(m_L, 16);
	lua_setglobal(m_L, "wmobjSpeedY");
	lua_pushinteger(m_L, 17);
	lua_setglobal(m_L, "wmobjTweakX");
	lua_pushinteger(m_L, 18);
	lua_setglobal(m_L, "wmobjTweakY");
	lua_pushinteger(m_L, 19);
	lua_setglobal(m_L, "wmobjCounter");
	lua_pushinteger(m_L, 20);
	lua_setglobal(m_L, "wmobjSpeed");
	lua_pushinteger(m_L, 21);
	lua_setglobal(m_L, "wmobjWidth");
	lua_pushinteger(m_L, 22);
	lua_setglobal(m_L, "wmobjHeight");
	lua_pushinteger(m_L, 23);
	lua_setglobal(m_L, "wmobjDirection");
	lua_pushinteger(m_L, 24);
	lua_setglobal(m_L, "wmobjFaceDir");
	lua_pushinteger(m_L, 25);
	lua_setglobal(m_L, "wmobjTimeDelay");
	lua_pushinteger(m_L, 26);
	lua_setglobal(m_L, "wmobjFrameDelay");
	lua_pushinteger(m_L, 27);
	lua_setglobal(m_L, "wmobjMoveResX");
	lua_pushinteger(m_L, 28);
	lua_setglobal(m_L, "wmobjMoveResY");

	if (ptr == NULL) {
		char tmp[256];
		sprintf(tmp, "brush/%s", pszName);
		if (luaL_dofile(m_L, tmp) != 0) {
			m_enStatus = BrushLuaError;
			m_szErrorInfo = new char[strlen(lua_tostring(m_L, -1)) + 1];
			m_szErrorInfo[strlen(lua_tostring(m_L, -1))] = '\0';
			strcpy(m_szErrorInfo, lua_tostring(m_L, -1));
			return;
		}
	}
	else {
		if (luaL_dostring(m_L, ptr) != 0) {
			m_enStatus = BrushLuaError;
			m_szErrorInfo = new char[strlen(lua_tostring(m_L, -1)) + 1];
			m_szErrorInfo[strlen(lua_tostring(m_L, -1))] = '\0';
			strcpy(m_szErrorInfo, lua_tostring(m_L, -1));
			return;
		}
	}
	lua_getglobal(m_L, "gameID");
	if (lua_tointeger(m_L, 1) != m_hParser->GetGame()) {
		m_enStatus = BrushWrongGame;
		return;
	}
	lua_pop(m_L, 1);

	lua_getglobal(m_L, "levelID");
	if (lua_tointeger(m_L, 1) != m_hParser->GetBaseLevel()) {
		m_enStatus = BrushWrongLevel;
		return;
	}
	lua_pop(m_L, 1);

	lua_getglobal(m_L, "layerName");
	bool fnd = 0;
	for (int i = 0; i < vstrLayers.size(); i++)
		if (!strcmp(lua_tostring(m_L, 1), vstrLayers[i].c_str())) {
			fnd = 1;
			break;
		}
	if (!fnd) {
		m_enStatus = BrushWrongLayer;
		return;
	}
	m_szLayerName = new char[strlen(lua_tostring(m_L, 1)) + 1];
	strcpy(m_szLayerName, lua_tostring(m_L, 1));
	lua_pop(m_L, 1);

	lua_getglobal(m_L, "brushIcon");
	m_iIconTile = lua_tointeger(m_L, 1);
	lua_pop(m_L, 1);

	char tmp[64];
	sprintf(tmp, "brushName_%s", GETL2S("Info", "Code"));
	lua_getglobal(m_L, tmp);
	if (!lua_isstring(m_L, 1)) {
		lua_pop(m_L, 1);
		lua_getglobal(m_L, "brushName");
	}

	const char * sztmp = lua_tostring(m_L, 1);
	lua_pop(m_L, 1);
	m_szName = new char[strlen(sztmp) + 2];
	strcpy(m_szName, sztmp);

	bool bSettingsFail = 0;
	lua_getglobal(m_L, "enableSettings");
	int enset = lua_tonumber(m_L, 1);
	lua_pop(m_L, 1);
	if (enset) {
		GV->jmp_val = setjmp(GV->jmp_env);
		if (!GV->jmp_val) {
			lua_getglobal(m_L, "cbInitSettings");
			GV_BRUSH_ptr = this;
			GV_BRUSH_bSettingsInit = 1;
			lua_call(m_L, 0, 0);
			GV_BRUSH_bSettingsInit = 0;
		}
		else
			bSettingsFail = 1;
	}

	if (bSettingsFail) {
		GV->Console->Printf("~y~Warning ~w~calling (~y~%s ~w~[~y~%s~w~]): unable to call cbInitSettings", m_szName, pszName);
	}
	else if (enset)
		GV->Console->Printf("~g~OK~w~ (~y~%s ~w~[~y~%s~w~] with settings)", m_szName, pszName);
	else
		GV->Console->Printf("~g~OK~w~ (~y~%s ~w~[~y~%s~w~])", m_szName, pszName);
}

cBrush::~cBrush()
{
	if (hconSettings != NULL)
		RemoveSettingsFromContainer(hconSettings);
	if (vSettings.size() != NULL) {
		for (int i = 0; i < vSettings.size(); i++) {
			delete vSettings[i]->labDesc;
			delete vSettings[i]->hWidg;
			delete vSettings[i];
			vSettings[i] = NULL;
		}
	}
	if (m_szName != NULL)
		delete[] m_szName;
	if (m_szFilename != NULL)
		delete[] m_szFilename;
	if (m_L != NULL)
		lua_close(m_L);
	if (m_szErrorInfo != NULL)
		delete[] m_szErrorInfo;
	delete[] m_szLayerName;
}

bool cBrush::Apply(WWD::Plane * phPlane, int piX, int piY)
{
	GV_BRUSH_bChanges = 0;
	if (m_enStatus != BrushOK) return 0;
	GV->jmp_val = setjmp(GV->jmp_env);
	if (!GV->jmp_val) {
		lua_getglobal(m_L, "cbBrushPlaced");

		lua_pushnumber(m_L, (int)phPlane);
		lua_pushnumber(m_L, piX);
		lua_pushnumber(m_L, piY);

		hGhostingBank = 0;
		lua_call(m_L, 3, 0);
	}
	return GV_BRUSH_bChanges;
}

void cBrush::ApplyGhosting(WWD::Plane * phPlane, int piX, int piY, std::vector<TileGhost> * vec)
{
	if (m_enStatus != BrushOK) return;
	GV->jmp_val = setjmp(GV->jmp_env);
	if (!GV->jmp_val) {
		lua_getglobal(m_L, "cbBrushPlaced");

		lua_pushnumber(m_L, (int)phPlane);
		lua_pushnumber(m_L, piX);
		lua_pushnumber(m_L, piY);

		hGhostingBank = vec;
		lua_call(m_L, 3, 0);
		hGhostingBank = 0;
	}
}

void cBrush::AddSetting(cBrushSetting * n)
{
	vSettings.push_back(n);
}

void cBrush::AddSettingsToContainer(SHR::Contener * con, int sx, int sy)
{
	for (int i = 0; i < vSettings.size(); i++) {
		con->add(vSettings[i]->labDesc, sx, sy + i * 25);
		con->add(vSettings[i]->hWidg, sx + 125, sy + i * 25);
	}
	hconSettings = con;
}

void cBrush::RemoveSettingsFromContainer(SHR::Contener * con)
{
	for (int i = 0; i < vSettings.size(); i++) {
		con->remove(vSettings[i]->labDesc);
		con->remove(vSettings[i]->hWidg);
	}
	hconSettings = NULL;
}

int cBrush::GetSettingsHeight()
{
	return vSettings.size() * 25;
}
