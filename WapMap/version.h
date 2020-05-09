#ifndef VERSION_H
#define VERSION_H

#define MAJOR_VERSION 0
#define MINOR_VERSION 4
#define PATCH_VERSION 0
#define WA_VER 33

#define PRODUCT_NAME "WapMap"
#define INTERNAL_NAME PRODUCT_NAME ".exe"

#define TO_STR2(x) #x
#define TO_STR(x) TO_STR2(x)
#define APP_VERSION TO_STR(MAJOR_VERSION) "." TO_STR(MINOR_VERSION) "." TO_STR(PATCH_VERSION)

#define EXENAME INTERNAL_NAME
#define WA_TITLEBAR PRODUCT_NAME " " APP_VERSION
#define WA_VERSTRING "v" APP_VERSION " (b" TO_STR(WA_VER) ")"
#define WA_VERPURE APP_VERSION

#define WA_LANGVER 14

#endif // !VERSION_H
