#ifndef H_CONF
#define H_CONF

//#define REAL_SIM

#define WA_VER 31
//#define CONF_NOSAVE
//#define WM_ADD_LUA_EXECUTER

#define EXENAME "WapMap.exe"

#define WA_TITLEBAR "WapMap"
#define WA_VERSTRING ""
#define WA_VERPURE ""
#define WA_AUTHORVERSTRING ""

//#define WM_EXT_TILESETS

#ifdef BUILD_RELEASE
 //#define CONF_NOSAVE
 //#define CONF_WATERMARK ""
 #define WA_TITLEBAR "WapMap 0.3.2"
 #define WA_VERSTRING "v0.3.2 (b31)"
 #define WA_AUTHORVERSTRING "0.3.2 (b31)"
 #define WA_VERPURE "0.3.2"
#else

#ifdef BUILD_DEVELOPER
 //#define CONF_NOSAVE
 #define CONF_WATERMARK "DEV"
 #define WA_TITLEBAR "WapMap 0.2dev"
 #define WA_VERSTRING "Dev v0.2dev (b28)"
 #define WA_VERPURE "0.2dev"
 #define WA_AUTHORVERSTRING "dev"
#else
 #ifdef BUILD_TESTER
  #define CONF_WATERMARK "DEV"
  #define WA_TITLEBAR "WapMap 0.2tdev"
  #define WA_VERSTRING "Dev v0.2tdev (b28)"
  #define WA_VERPURE "0.2tdev"
  #define WA_AUTHORVERSTRING "0.2tdev (b28)"
 #else
  #define WA_TITLEBAR "WapMap MAINDEV"
  #define WA_VERSTRING "Beta vDEV"
  #define WA_VERPURE "MAINDEV"
  #define WA_AUTHORVERSTRING "maindev"
  //#define DEBUG_DRAW
 #endif
#endif

#endif

#define WA_AUTHORSHEIGHT 280
#define WA_LANGVER 14

#endif
