#include "globlua.h"
#include "globals.h"
#include "states/editing_ww.h"
#include "cObjectUserData.h"
extern HGE* hge;

extern std::vector<TileGhost> * hGhostingBank;

int gv_wmLog(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmLog (require 1, given %d)", argc);
		return 0;
	}
	GV->Console->Printf(lua_tostring(L, 1));
	return 0;
} //string log; void

/*int gv_wmRunScript(lua_State *L)
{
	int argc = lua_gettop(L);
	if( argc < 1 ){
	 GV->Console->Printf("~r~Lua: not enough arguments to wmRunScript (require 1, given %d)", argc);
	 return 0;
	}
	const char * scriptname = lua_tostring(L, 1);
	if( strchr(scriptname, '\\') != NULL || strchr(scriptname, '/') != NULL ){
	 GV->Console->Printf("~r~Lua: access violation in wmRunScript, found '/' or '\\' in filename.");
	 return 0;
	}
	char tmp[256];
	sprintf(tmp, "scripts/%s", scriptname);
	FILE * f = fopen(tmp, "r");
	if( !f ){
	 GV->Console->Printf("~r~Lua: error executing '~y~%s~r~' script, file not found or unreachable.");
	 return 0;
	}
	fclose(f);
	GV->Console->Printf("~w~Lua: executing external script...");
	if( luaL_dofile(L, tmp) ){
	 GV->Console->Printf("~r~Lua: error executing '~y~%s~r~' script:", scriptname);
	 GV->Console->Printf("~r~     %s", lua_tostring(L, -1));
	}else
	 GV->Console->Printf("~g~Lua: script '~y~%s~g~' executed.");
	return 0;
} //string filename; void*/

int gv_wmRandInt(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 2) {
		GV->Console->Printf("~r~Brush lua: not enough arguments to wmRandInt (require 2, given %d)", argc);
		return 0;
	}
	lua_pushinteger(L, hge->Random_Int(lua_tonumber(L, 1), lua_tonumber(L, 2)));
	return 1;
} //int min, int max; int

int gv_wmRandBool(lua_State *L)
{
	lua_pushinteger(L, hge->Random_Int(0, 1));
	return 1;
} //float min, float max; float

int gv_wmGetFileName(lua_State *L)
{
	return 0;
} //void; string

int gv_wmGetMapName(lua_State *L)
{
	lua_pushstring(L, GV->editState->hParser->GetName());
	return 1;
} //void; string

int gv_wmGetMapAuthor(lua_State *L)
{
	lua_pushstring(L, GV->editState->hParser->GetAuthor());
	return 1;
} //void; string

int gv_wmGetMapDate(lua_State *L)
{
	lua_pushstring(L, GV->editState->hParser->GetDate());
	return 1;
} //void; string

int gv_wmGetActivePlane(lua_State *L)
{
	lua_pushinteger(L, (int)GV->editState->GetActivePlane());
	return 1;
}

int gv_wmGetPlanesCount(lua_State *L)
{
	lua_pushinteger(L, GV->editState->hParser->GetPlanesCount());
	return 1;
} //void; int

int gv_wmGetPlaneByIt(lua_State *L)
{
	lua_pushinteger(L, (int)GV->editState->hParser->GetPlane(lua_tonumber(L, 1)));
	return 1;
} //int planeindex; plane

int gv_wmGetPlaneByName(lua_State *L)
{
	const char * planename = lua_tostring(L, 1);
	for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++)
		if (!strcmp(GV->editState->hParser->GetPlane(i)->GetName(), planename)) {
			lua_pushinteger(L, (int)GV->editState->hParser->GetPlane(i));
			return 1;
		}
	lua_pushinteger(L, 0);
	return 1;
}

int gv_wmGetPlaneName(lua_State *L)
{
	lua_pushstring(L, ((WWD::Plane*)(int)lua_tonumber(L, 1))->GetName());
	return 1;
} //plane ptr; string

int gv_wmGetPlaneWidth(lua_State *L)
{
	return 0;
}

int gv_wmGetPlaneHeight(lua_State *L)
{
	return 0;
}

int gv_wmGetObjectsCount(lua_State *L)
{
	return 0;
}

int gv_wmIsPlaneMain(lua_State *L)
{
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmIsPlaneMain (require 1, given %d)", argc);
		lua_pushinteger(L, 0);
		return 1;
	}
	WWD::Plane * pl = (WWD::Plane*)int(lua_tonumber(L, 1));
	for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++) {
		if (GV->editState->hParser->GetPlane(i) == pl) {
			bool main = pl->GetFlags() & WWD::Flag_p_MainPlane;
			lua_pushinteger(L, main);
			return 1;
		}
	}

	lua_pushinteger(L, 0);
	return 1;
}

int gv_wmCreateObject(lua_State *L) //string logic, string imgset, int x, int y, int z, int i; objptr
{
	if (hGhostingBank != 0) {
		lua_pushinteger(L, 0);
		return 1;
	}
	int argc = lua_gettop(L);
	if (argc < 4) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmCreateObject (require 4, given %d)", argc);
		lua_pushinteger(L, 0);
		return 1;
	}
	WWD::Plane * pl = GV->editState->plMain;

	int x = lua_tonumber(L, 3), y = lua_tonumber(L, 4), z = 0, i = -1;
	if (argc >= 5)
		z = lua_tonumber(L, 5);
	if (argc >= 6)
		i = lua_tonumber(L, 6);

	if (x < 0 || y < 0 || x >= pl->GetPlaneWidthPx() || y >= pl->GetPlaneHeightPx()) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int planeid = -1;
	for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++)
		if (GV->editState->hParser->GetPlane(i) == pl) {
			planeid = i;
			break;
		}

	WWD::Object * obj = new WWD::Object();
	obj->SetLogic(lua_tostring(L, 1));
	obj->SetImageSet(lua_tostring(L, 2));
	obj->SetParam(WWD::Param_LocationX, x);
	obj->SetParam(WWD::Param_LocationY, y);
	obj->SetParam(WWD::Param_LocationZ, z);
	obj->SetParam(WWD::Param_LocationI, i);

	pl->AddObjectAndCalcID(obj);
	obj->SetUserData(new cObjUserData(obj));
	GV->editState->hPlaneData[planeid]->ObjectData.hQuadTree->UpdateObject(obj);
	lua_pushinteger(L, (int)obj);
	GV->editState->vPort->MarkToRedraw(1);
	return 1;
}

int gv_wmDeleteObject(lua_State *L) //objptr; void
{
	if (hGhostingBank != 0) {
		return 0;
	}
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmDeleteObject (require 1, given %d)", argc);
		return 0;
	}

	WWD::Plane * pl = GV->editState->plMain;

	WWD::Object * obj = (WWD::Object*)int(lua_tonumber(L, 1));
	bool fnd = 0;
	for (int i = 0; i < pl->GetObjectsCount(); i++) {
		if (pl->GetObjectByIterator(i) == obj) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		GV->Console->Printf("~r~Lua: wmDeleteObject: unable to delete object, invalid pointer!");
		return 0;
	}
	pl->DeleteObject(obj);
	return 0;
}

int gv_wmGetObjectsInArea(lua_State *L) //int x1, int y1, int x2, int y2; array of objptr
{
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmGetObjectsInArea (require 2, given %d)", argc);
		return 0;
	}

	int x1 = lua_tonumber(L, 1),
		y1 = lua_tonumber(L, 2);
	int x2, y2;
	if (argc >= 3) x2 = lua_tonumber(L, 3);
	else            x2 = x1;
	if (argc == 4) y2 = lua_tonumber(L, 4);
	else            y2 = y1;
	if (x2 < x1 || y2 < y1) {
		GV->Console->Printf("~r~Lua: wmGetObjectsInArea: invalid coordinates [%d,%d - %d,%d]", x1, y1, x2, y2);
		return 0;
	}
	WWD::Plane * pl = GV->editState->plMain;
	int plid = -1;
	for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++)
		if (GV->editState->hParser->GetPlane(i) == pl) {
			plid = i;
			break;
		}
	std::vector<WWD::Object*> objs = GV->editState->hPlaneData[plid]->ObjectData.hQuadTree->GetObjectsByArea(x1, y1, x2 - x1, y2 - y1);
	for (int i = 0; i < objs.size(); i++)
		if (objs[i] == GV->editState->hStartingPosObj) {
			objs.erase(objs.begin() + i);
			break;
		}
	if (objs.size() == 0) {
		return 0;
	}
	lua_createtable(L, objs.size(), 0);
	for (int i = 0; i < objs.size(); i++) {
		lua_pushnumber(L, i);
		lua_pushnumber(L, (int)objs[i]);
		lua_settable(L, -3);
	}
	return 1;
}

int gv_wmGetObjectName(lua_State *L) //objptr; string
{
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmGetObjectName (require 1, given %d)", argc);
		return 0;
	}

	WWD::Plane * pl = GV->editState->plMain;

	WWD::Object * obj = (WWD::Object*)int(lua_tonumber(L, 1));
	bool fnd = 0;
	for (int i = 0; i < pl->GetObjectsCount(); i++) {
		if (pl->GetObjectByIterator(i) == obj) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		GV->Console->Printf("~r~Lua: wmGetObjectName: invalid object!");
		return 0;
	}
	lua_pushstring(L, obj->GetName());
	return 1;
}

int gv_wmGetObjectImageSet(lua_State *L) //objptr; string
{
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmGetImageSet (require 1, given %d)", argc);
		return 0;
	}

	WWD::Plane * pl = GV->editState->plMain;

	WWD::Object * obj = (WWD::Object*)int(lua_tonumber(L, 1));
	bool fnd = 0;
	for (int i = 0; i < pl->GetObjectsCount(); i++) {
		if (pl->GetObjectByIterator(i) == obj) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		GV->Console->Printf("~r~Lua: wmGetImageSet: invalid object!");
		return 0;
	}
	lua_pushstring(L, obj->GetImageSet());
	return 1;
}

int gv_wmGetObjectLogic(lua_State *L) //objptr; string
{
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmGetObjectLogic (require 1, given %d)", argc);
		return 0;
	}

	WWD::Plane * pl = GV->editState->plMain;

	WWD::Object * obj = (WWD::Object*)int(lua_tonumber(L, 1));
	bool fnd = 0;
	for (int i = 0; i < pl->GetObjectsCount(); i++) {
		if (pl->GetObjectByIterator(i) == obj) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		GV->Console->Printf("~r~Lua: wmGetObjectLogic: invalid object!");
		return 0;
	}
	lua_pushstring(L, obj->GetLogic());
	return 1;
}

int gv_wmGetObjectAnim(lua_State *L) //objptr; string
{
	int argc = lua_gettop(L);
	if (argc < 1) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmGetObjectAnim (require 1, given %d)", argc);
		return 0;
	}

	WWD::Plane * pl = GV->editState->plMain;

	WWD::Object * obj = (WWD::Object*)int(lua_tonumber(L, 1));
	bool fnd = 0;
	for (int i = 0; i < pl->GetObjectsCount(); i++) {
		if (pl->GetObjectByIterator(i) == obj) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		GV->Console->Printf("~r~Lua: wmGetObjectAnim: invalid object!");
		return 0;
	}
	lua_pushstring(L, obj->GetAnim());
	return 1;
}

int gv_wmGetObjectParam(lua_State *L) //objptr, objparam; int
{
	int argc = lua_gettop(L);
	if (argc < 2) {
		GV->Console->Printf("~r~Lua: not enough arguments to wmGetObjectParam (require 2, given %d)", argc);
		return 0;
	}

	WWD::Plane * pl = GV->editState->plMain;

	WWD::Object * obj = (WWD::Object*)int(lua_tonumber(L, 1));
	bool fnd = 0;
	for (int i = 0; i < pl->GetObjectsCount(); i++) {
		if (pl->GetObjectByIterator(i) == obj) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		GV->Console->Printf("~r~Lua: wmGetObjectParam: invalid object!");
		return 0;
	}
	int param = lua_tonumber(L, 2);
	if (param < 0 || param > 28) {
		GV->Console->Printf("~r~Lua: wmGetObjectParam: invalid param!");
		return 0;
	}

	lua_pushinteger(L, obj->GetParam((WWD::OBJ_PARAMS)param));
	return 1;
}
