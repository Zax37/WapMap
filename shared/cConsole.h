#ifndef H_SHARED_CONSOLE
#define H_SHARED_CONSOLE

#include <vector>
#include <fstream>

#include "hgesprite.h"
#include "hgefont.h"
#include "Windows.h"

namespace SHR {
    class cConsole;

    class cConsoleCMD {
    private:
        char *m_szName;
        char *m_szDesc;

        void (*m_hFunc)(int, char **);

        friend class cConsole;
    };

    class cConsoleVar {
    protected:
        char *m_szID;
        int m_iPtr;

        virtual void Set(cConsole *handle, int argc, char **argv) {};

        virtual void Print(cConsole *handle) {};

        friend class cConsole;
    };

    class cConsoleInt : public cConsoleVar {
    private:
        virtual void Set(cConsole *handle, int argc, char **argv);

        virtual void Print(cConsole *handle);

        friend class cConsole;
    };

    class cConsoleFloat : public cConsoleVar {
    private:
        virtual void Set(cConsole *handle, int argc, char **argv);

        virtual void Print(cConsole *handle);

        friend class cConsole;
    };

    class cConsoleBool : public cConsoleVar {
    private:
        virtual void Set(cConsole *handle, int argc, char **argv);

        virtual void Print(cConsole *handle);

        friend class cConsole;
    };

    class cConsole {
    private:
        CRITICAL_SECTION csLock;
        hgeSprite *m_sprBG;
        hgeFont *m_fnt;
        std::ofstream *m_osFile;
        bool m_bFile, m_bEnabled, m_bFocused;
        float m_fAnim;
        float m_fAnimScroll;
        std::vector<cConsoleCMD *> m_vCmds;
        std::vector<char *> m_vLines;
        std::vector<cConsoleVar *> m_vVars;

        int m_iScroll;
        std::string m_szInput;
        bool m_bAnimSet;
    public:
        cConsole(hgeSprite *psprBG = NULL);

        void Print(const char *pszValue);

        void Printf(const char *pszFormat, ...);

        ~cConsole();

        void Think();

        void Render();

        void AddCommand(const char *pszName, const char *pszHelp, void(*phFunc)(int, char **));

        void ParseCommand(const char *pszCommand);

        void AddModifiableInt(const char *pszName, int *piVar);

        void AddModifiableFloat(const char *pszName, float *pfVar);

        void AddModifiableBool(const char *pszName, bool *pbVar);

        void DeleteModifiableVar(const char *pszName);
        //void AddModifiableFloat(const char * pszID, float * pfVar);

        void LogToFile(const char *pszFile);

        bool IsLoggingToFile() { return m_bFile; };

        void SetBG(hgeSprite *psprBG) { m_sprBG = psprBG; };

        void Enable(bool pbEnable) { m_bEnabled = pbEnable; };

        bool IsEnabled() { return m_bEnabled; };

        void SetFont(hgeFont *phFnt) { m_fnt = phFnt; };

        void Help(int argc, char **argv);

        void SetVar(int argc, char **argv);

        void GetVar(int argc, char **argv);

        void ListVars();

        const char *GetLine(int i);

        void FixPos();
    };

}

#endif
