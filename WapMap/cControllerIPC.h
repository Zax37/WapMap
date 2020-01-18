#ifndef H_C_PIPECTRL
#define H_C_PIPECTRL

#include <windows.h>

#define WM_MAILSLOT "\\\\.\\mailslot\\WapMap"

#define WM_IPC_OPENFILE 1

class cClientIPC
{
 private:
  HANDLE hMailslot;
  bool bConnected;
 public:
  cClientIPC();
  ~cClientIPC();
  bool RemoteOpenMap(const char * szPath);
  bool IsConnected(){ return bConnected; };
  bool Connect();
};

class cServerIPC
{
 private:
  bool bOk;
  HANDLE hMailslot;
 public:
  cServerIPC();
  ~cServerIPC();
  bool Think();
  bool IsOk(){ return bOk; };
};

#endif
