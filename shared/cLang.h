#ifndef H_SHARED_LANGUAGE
#define H_SHARED_LANGUAGE

#include <unordered_map>
#include "SimpleIni.h"

namespace SHR {
    class cLanguage {
    private:
        CSimpleIniW *hINI;
        std::string szName, szCode;
        std::unordered_map<uint32_t, std::string> cache;

        std::string ws2s(const std::wstring &wstr);

        const wchar_t *Transform(const wchar_t *str);
    public:
        cLanguage(const char *pszName, int piVer) throw(int);

        ~cLanguage();

        const char *GetString(const wchar_t *pszSection, int piID);

        const char *GetStringS(const wchar_t *pszSection, const wchar_t *pszID);

        const char *GetName() { return szName.c_str(); };

        const char *GetCode() { return szCode.c_str(); };
    };
}

#endif
