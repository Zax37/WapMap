#ifndef H_C_PROGRESSINFO
#define H_C_PROGRESSINFO

#include <windows.h>
#include <process.h>
#include <string>

struct structProgressInfo {
	int iGlobalProgress, iGlobalEnd;
	int iDetailedProgress, iDetailedEnd;
	std::string strGlobalCaption, strDetailedCaption;
	char* szDetailedDescription;
	bool bUpdate;
};

class cProgressInfo {
private:
	structProgressInfo piInfo;
	CRITICAL_SECTION cs;
	bool bLocked;
public:
	cProgressInfo();
	~cProgressInfo();
	void Lock();
	void Unlock();
	bool IsLocked() { return bLocked; };

	void Set(int gp, int dp, int ge = -1, int de = -1);
	void SetGlobalEnd(int i);
	void SetGlobalProgress(int i);
	void SetDetailedEnd(int i);
	void SetDetailedProgress(int i);
	void SetDescription(const char * sz);
	void MarkToUpdate(bool mark);

	int GetGlobalEnd();
	int GetGlobalProgress();
	int GetDetailedEnd();
	int GetDetailedProgress();
	const char * GetDetailedDescription();
	bool IsMarkedToUpdate();
};

#endif
