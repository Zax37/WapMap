#include "cLang.h"
#include <algorithm>

void ReplaceStringInPlace(std::string &subject, const std::string &search,
                          const std::string &replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

SHR::cLanguage::cLanguage(const char *pszName, int piLang) throw(int) {
    szName = new char(strlen(pszName) + 1);
    strcpy(szName, pszName);
    char *path = new char[strlen(pszName) + 10];
    sprintf(path, "lang/%s.ini", pszName);
    hINI = new CSimpleIni;
    if (hINI->LoadFile(path) != SI_OK) {
        delete[] path;
        throw -1;
    }
    delete[] path;
    int lver = atoi(hINI->GetValue("Info", "Version", "0"));
    if (lver != piLang)
        throw -2;
    const char *code = hINI->GetValue("Info", "Code", "EN");
    szCode = new char[strlen(code) + 1];
    strcpy(szCode, code);
}

SHR::cLanguage::~cLanguage() {
    delete szName;
    delete szCode;
    delete hINI;
}

const char *SHR::cLanguage::GetString(const char *pszSection, int piID) {
    char tmp[6];
    sprintf(tmp, "%d", piID);
    return hINI->GetValue(pszSection, tmp, tmp);
}

const char *SHR::cLanguage::GetStringS(const char *pszSection, const char *pszID) {
    /*std::string str = hINI->GetValue(pszSection, pszID, "[unset_lang]");
    char cstr[2] = {'a', '\0'};
    ReplaceStringInPlace(str, "\n", std::string(cstr));
    return str.c_str();*/
    return hINI->GetValue(pszSection, pszID, "[unset_lang]");
}
