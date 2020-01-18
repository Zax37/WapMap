#include "cProgressInfo.h"

cProgressInfo::cProgressInfo()
{
    InitializeCriticalSection( &cs );

    piInfo.szDetailedDescription = new char[1];
    piInfo.szDetailedDescription[0] = '\0';
    piInfo.iGlobalProgress = piInfo.iDetailedProgress = 0;
    piInfo.iGlobalEnd = piInfo.iDetailedEnd = 1;
    piInfo.bUpdate = 0;
    bLocked = 0;
}

cProgressInfo::~cProgressInfo()
{
    DeleteCriticalSection(&cs);
    delete [] piInfo.szDetailedDescription;
}

void cProgressInfo::SetGlobalEnd(int i)
{
    if( !bLocked ) throw;
    piInfo.iGlobalEnd = i;
    piInfo.bUpdate = 1;
}

void cProgressInfo::SetGlobalProgress(int i)
{
    if( !bLocked ) throw;
    piInfo.iGlobalProgress = i;
    piInfo.bUpdate = 1;
}

void cProgressInfo::SetDetailedEnd(int i)
{
    if( !bLocked ) throw;
    piInfo.iDetailedEnd = i;
    piInfo.bUpdate = 1;
}

void cProgressInfo::SetDetailedProgress(int i)
{
    if( !bLocked ) throw;
    piInfo.iDetailedProgress = i;
    piInfo.bUpdate = 1;
}

void cProgressInfo::SetDescription(const char * sz)
{
    if( !bLocked ) throw;
    delete [] piInfo.szDetailedDescription;
    piInfo.szDetailedDescription = new char[strlen(sz)+1];
    strcpy(piInfo.szDetailedDescription, sz);
    piInfo.bUpdate = 1;
}

void cProgressInfo::MarkToUpdate(bool mark)
{
    if( !bLocked ) throw;
    piInfo.bUpdate = mark;
}

void cProgressInfo::Lock()
{
    //while( bLocked ) Sleep(10);
    while( TryEnterCriticalSection(&cs) == 0 ){ Sleep(5); }
    bLocked = 1;
}

void cProgressInfo::Unlock()
{
    //while( !bLocked ) Sleep(10);
    LeaveCriticalSection(&cs);
    bLocked = 0;
}

void cProgressInfo::Set(int gp, int dp, int ge, int de)
{
    if( !bLocked ) throw;
    piInfo.iGlobalProgress = gp;
    piInfo.iDetailedProgress = dp;
    if( ge != -1 )
     piInfo.iGlobalEnd = ge;
    if( de != -1 )
     piInfo.iDetailedEnd = de;
}

int cProgressInfo::GetGlobalEnd()
{
    if( !bLocked ) throw;
    return piInfo.iGlobalEnd;
}

int cProgressInfo::GetGlobalProgress()
{
    if( !bLocked ) throw;
    return piInfo.iGlobalProgress;
}

int cProgressInfo::GetDetailedEnd()
{
    if( !bLocked ) throw;
    return piInfo.iDetailedEnd;
}

int cProgressInfo::GetDetailedProgress()
{
    if( !bLocked ) throw;
    return piInfo.iDetailedProgress;
}

const char * cProgressInfo::GetDetailedDescription()
{
    if( !bLocked ) throw;
    return (const char*)piInfo.szDetailedDescription;
}

bool cProgressInfo::IsMarkedToUpdate()
{
    if( !bLocked ) throw;
    return piInfo.bUpdate;
}
