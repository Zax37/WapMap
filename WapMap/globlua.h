#ifndef GLOBLUAFUNC
#define GLOBLUAFUNC

extern "C" {
#include "../shared/lua/lua.h"
#include "../shared/lua/lualib.h"
#include "../shared/lua/lauxlib.h"
}
//input,input; output,output

//various
 int gv_wmLog(lua_State *L); //string log; void
 //int gv_wmRunScript(lua_State *L); //string filename; void

//random
 int gv_wmRandInt(lua_State *L); //int min, int max; int
 int gv_wmRandBool(lua_State *L); //float min, float max; float

//world
 int gv_wmGetFileName(lua_State *L); //void; string
 int gv_wmGetMapName(lua_State *L); //void; string
 int gv_wmGetMapAuthor(lua_State *L); //void; string
 int gv_wmGetMapDate(lua_State *L); //void; string

//planes
 //querying world
  int gv_wmGetActivePlane(lua_State *L); //void; plane
  int gv_wmGetPlanesCount(lua_State *L); //void; int
  int gv_wmGetPlaneByIt(lua_State *L); //int; plane
  int gv_wmGetPlaneByName(lua_State *L); //string; plane
 //querying plane
  int gv_wmGetPlaneName(lua_State *L); //plane; string
  int gv_wmGetPlaneWidth(lua_State *L); //plane; int
  int gv_wmGetPlaneHeight(lua_State *L); //plane; int
  int gv_wmGetObjectsCount(lua_State *L); //plane; int
  int gv_wmIsPlaneMain(lua_State *L); //plane; bool

 //objects
  int gv_wmCreateObject(lua_State *L); //string logic, string imgset, int x, int y, int z, int i; objptr
  int gv_wmDeleteObject(lua_State *L); //objptr; void
  int gv_wmGetObjectsInArea(lua_State *L); //int x1, int y1, int x2, int y2; array of objptr
  int gv_wmGetObjectName(lua_State *L); //objptr; string
  int gv_wmGetObjectImageSet(lua_State *L); //objptr; string
  int gv_wmGetObjectLogic(lua_State *L); //objptr; string
  int gv_wmGetObjectAnim(lua_State *L); //objptr; string
  int gv_wmGetObjectParam(lua_State *L); //objptr, objparam; int

#endif
