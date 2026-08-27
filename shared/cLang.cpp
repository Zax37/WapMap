#include "cLang.h"
#include <locale>
#include <codecvt>
#include <string_view>
#include <cstdint>

#pragma execution_character_set("utf-8")

void ReplaceStringInPlace(std::string &subject, const std::string &search,
                          const std::string &replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

std::string SHR::cLanguage::ws2s(const std::wstring& wstr)
{
    int length = wstr.length();
    std::string str(length, 0);
    int code;
    switch (szCode[0]) {
    case 'P':
        code = 1250;
    break;
    case 'R':
        code = 1251;
    break;
    default:
        code = 1252;
    break;
    }
    WideCharToMultiByte(code, 0, wstr.c_str(), length, &str[0], length, nullptr, nullptr);
    return str;
}


const wchar_t * SHR::cLanguage::Transform(const wchar_t *str) {
    if (szCode[0] == 'E' && szCode[1] == 'S') {
        for (wchar_t * c = (wchar_t*)str; *c != '\0'; c++) {
            switch (*c) {
            case 0x00BF:
                c[0] = 0x00F4;
            break;
            case 0x00F1:
                c[0] = 0x00F2;
                break;
            case 0x00FA:
                c[0] = 0x00F5;
            break;
            case 0x00A1:
                c[0] = 0x00F6;
                break;
            }
        }
    } else if (szCode[0] == 'R' && szCode[1] == 'U') {
        for (wchar_t * c = (wchar_t*)str; *c != '\0'; c++) {
            switch (*c) {
                case 0x044F:
                    c[0] = 0x042F;
                    break;
            }
        }
    }

    return str;
}

SHR::cLanguage::cLanguage(const char *pszName, int piLang) {
    szName = new char(strlen(pszName) + 1);
    strcpy(szName, pszName);
    char *path = new char[strlen(pszName) + 10];
    sprintf(path, "lang/%s.ini", pszName);
    hINI = new CSimpleIniW(true);
    if (hINI->LoadFile(path) != SI_OK) {
        delete[] path;
        throw -1;
    }
    delete[] path;
    int lver = _wtoi(hINI->GetValue(L"Info", L"Version", L"0"));
    if (lver != piLang)
        throw -2;
    std::wstring code = hINI->GetValue(L"Info", L"Code", L"EN");
    szCode = new char[code.length() + 1];
    strcpy(szCode, ws2s(code).c_str());
}

SHR::cLanguage::~cLanguage() {
    delete szName;
    delete szCode;
    delete hINI;
}

constexpr uint32_t Hash(std::wstring_view s)
{
    uint32_t h = 2166136261u;
    for (wchar_t c : s)
        h = (h ^ c) * 16777619u;
    return h;
}

const char *SHR::cLanguage::GetString(const wchar_t *pszSection, int piID) {
    uint32_t key = Hash(pszSection) + piID;
    auto it = cache.find(key);
    if (it == cache.end()) {
        wchar_t tmp[6];
        wsprintfW(tmp, L"%d", piID);
        cache[key] = ws2s(Transform(hINI->GetValue(pszSection, tmp, tmp)));
        return cache[key].c_str();
    }
    return it->second.c_str();
}

const char* SHR::cLanguage::GetStringS(const wchar_t *pszSection, const wchar_t *pszID) {
    uint32_t key = Hash(pszSection) + Hash(pszID);
    auto it = cache.find(key);
    if (it == cache.end()) {
        cache[key] = ws2s(Transform(hINI->GetValue(pszSection, pszID, L"[unset_lang]")));
        return cache[key].c_str();
    }
    return it->second.c_str();
}
