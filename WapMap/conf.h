#ifndef H_CONF
#define H_CONF

#define WA_VER 32

#define EXENAME "WapMap.exe"

#define WA_TITLEBAR "WapMap"
#define WA_VERSTRING ""
#define WA_VERPURE ""
#define WA_AUTHORVERSTRING ""

#ifdef BUILD_RELEASE
 //#define CONF_NOSAVE
 //#define CONF_WATERMARK ""
 #define WA_TITLEBAR "WapMap 0.3.3"
 #define WA_VERSTRING "v0.3.3 (b32)"
 #define WA_AUTHORVERSTRING "0.3.3 (b32)"
 #define WA_VERPURE "0.3.3"
 #else
  #define WA_TITLEBAR "WapMap MAINDEV"
  #define WA_VERSTRING "Beta vDEV"
  #define WA_VERPURE "MAINDEV"
  #define WA_AUTHORVERSTRING "maindev"
  //#define DEBUG_DRAW
#endif

#define WA_LANGVER 14

#endif
