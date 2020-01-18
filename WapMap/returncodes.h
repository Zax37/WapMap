#ifndef H_RETURNCODES
#define H_RETURNCODES

enum rc_type {
 RC_LoadMap = 0,
 RC_ErrorState,
 RC_MapShot,
 RC_ObjectProp,
 RC_ObjPropSelectedValues,
 RC_CodeEditor
};

struct returnCode {
 rc_type Type;
 int Ptr;
};

#endif
