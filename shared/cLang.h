#ifndef H_SHARED_LANGUAGE
#define H_SHARED_LANGUAGE

#include "SimpleIni.h"

namespace SHR {
    class cLanguage {
    private:
        CSimpleIni *hINI;
        char *szName, *szCode;
    public:
        cLanguage(const char *pszName, int piVer) throw(int);

        ~cLanguage();

        const char *GetAuthor() { return hINI->GetValue("Info", "Author", "unknown"); };

        const char *GetString(const char *pszSection, int piID);

        const char *GetStringS(const char *pszSection, const char *pszID);

        const char *GetName() { return (const char *) szName; };

        const char *GetCode() { return (const char *) szCode; };
    };
}

#endif
