#ifndef H_FUNC_CONSOLE
#define H_FUNC_CONSOLE

void ConMoo(int argc, char * argv[]);
void ConHelp(int argc, char * argv[]);
void ConLoadWWD(int argc, char * argv[]);
void ConKijan(int argc, char * argv[]);
void ConVar(int argc, char * argv[]);
void ConVarSet(int argc, char * argv[]);
void ConVarGet(int argc, char * argv[]);

void ConCWD(int argc, char * argv[]);

void ConLua(int argc, char * argv[]);
void ConFlushLua(int argc, char * argv[]);
void ConLuaGetBuf(int argc, char * argv[]);

void ConMemInfo(int argc, char * argv[]);

void ConShader(int argc, char * argv[]);

#endif
