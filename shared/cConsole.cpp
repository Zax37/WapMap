#include "cConsole.h"
#include "hge.h"
#include "commonFunc.h"
#include "../WapMap/globals.h"
#include <fstream>

extern HGE *hge;
extern cGlobals *GV;

SHR::cConsole::cConsole(hgeSprite *psprBG) {
    m_sprBG = psprBG;
    m_bFocused = false;
    m_bEnabled = true;
    m_fAnim = 0;
    m_bAnimSet = false;
    m_fAnimScroll = 0;
    m_osFile = nullptr;
    m_vCmds.clear();
    m_iScroll = 0;
    m_bFile = false;
    InitializeCriticalSection(&csLock);
}

SHR::cConsole::~cConsole() {
    for (auto &m_vCmd : m_vCmds) {
        delete[] m_vCmd->m_szName;
        delete[] m_vCmd->m_szDesc;
        delete m_vCmd;
    }
    DeleteCriticalSection(&csLock);
}

void SHR::cConsole::LogToFile(const char *pszFile) {
    if (pszFile == nullptr) {
        if (m_osFile != nullptr) {
            m_osFile->close();
            delete m_osFile;
            m_osFile = nullptr;
        }
        m_bFile = false;
    } else {
        if (m_osFile != nullptr) {
            m_osFile->close();
            delete m_osFile;
            m_osFile = nullptr;
        }
        m_osFile = new std::ofstream(pszFile, std::ios_base::out | std::ios_base::trunc);
        m_bFile = true;
        *m_osFile << "<html><body bgcolor='black'><pre><span style='color: white;'>" << std::endl;
    }
}

void SHR::cConsole::ParseCommand(const char *pszCommand) {
    char *cmdName = new char[64];
    ZeroMemory(cmdName, 64);
    for (int i = 0; i < strlen(pszCommand) && i < 64; i++)
        if (i == 63)
            cmdName[i] = '\0';
        else if (pszCommand[i] == ' ') {
            cmdName[i] = '\0';
            break;
        } else
            cmdName[i] = pszCommand[i];
    const char *pch = strchr(pszCommand, ' ');
    int argc = 0;
    while (pch != NULL) {
        argc++;
        pch = strchr(pch + 1, ' ');
    }
    char **argv = new char *[argc];
    for (int i = 0; i < argc; i++) {
        argv[i] = new char[128];
        ZeroMemory(argv[i], 128);
    }
    int offset = strlen(cmdName) + 1;
    for (int i = 0; i < argc; i++) {
        for (int y = 0; y < strlen(pszCommand) - offset && y < 128; y++) {
            if (pszCommand[y + offset] == ' ')
                break;
            else
                argv[i][y] = pszCommand[y + offset];
        }
        offset += strlen(argv[i]) + 1;
    }

    bool bFound = 0;
    for (auto &m_vCmd : m_vCmds) {
        if (!strcmp(cmdName, m_vCmd->m_szName)) {
            (m_vCmd->m_hFunc)(argc, argv);
            bFound = true;
            break;
        }
    }
    if (!bFound)
        Printf("~r~No such function: '%s'.~w~", cmdName);
    delete[] cmdName;
    for (int i = 0; i < argc; i++)
        delete[] argv[i];
    delete[] argv;
}

void SHR::cConsole::Think() {
    if (!m_bAnimSet) {
        m_fAnim = -int(hge->System_GetState(HGE_SCREENHEIGHT) / 3);
        m_bAnimSet = true;
    }
    if (m_bEnabled) {
        if (hge->Input_KeyDown(HGEK_GRAVE)) {
            m_bFocused = !m_bFocused;
        }

        if (m_bFocused && m_fAnim < 0) { //opening
            m_fAnim += hge->Timer_GetDelta() * 400;
            if (m_fAnim > 0)
                m_fAnim = 0;
        } else if (!m_bFocused && m_fAnim > -int(hge->System_GetState(HGE_SCREENHEIGHT) / 3)) { //closing
            m_fAnim -= hge->Timer_GetDelta() * 400;
            if (m_fAnim < -int(hge->System_GetState(HGE_SCREENHEIGHT) / 3))
                m_fAnim = -int(hge->System_GetState(HGE_SCREENHEIGHT) / 3);
            m_fAnimScroll += hge->Timer_GetDelta() * 32;
            if (m_fAnimScroll > 128)
                m_fAnimScroll -= 128;
        }

        if (m_bFocused) {

            if (hge->Input_KeyDown(HGEK_PGUP)) {
                m_iScroll += 3;
            } else if (hge->Input_KeyDown(HGEK_PGDN) && m_iScroll > 0) {
                m_iScroll -= 3;
            }

            if (hge->Input_GetChar() != NULL && hge->Input_GetChar() != '`') {
                if (hge->Input_GetChar() == HGEK_BACKSPACE) {
                    if (!m_szInput.empty())
                        m_szInput.pop_back();
                } else if (hge->Input_GetChar() == HGEK_ENTER) {
                    ParseCommand(m_szInput.c_str());
                    m_szInput = "";
                } else
                    m_szInput.push_back(hge->Input_GetChar());
            }

            m_fAnimScroll += hge->Timer_GetDelta() * 32;
            if (m_fAnimScroll > 128)
                m_fAnimScroll -= 128;
        }
    } else if (m_bFocused)
        m_bFocused = false;
}

void SHR::cConsole::Render() {
    if (!m_bAnimSet) {
        m_fAnim = -int(hge->System_GetState(HGE_SCREENHEIGHT) / 3);
        m_bAnimSet = 1;
    }
    if (m_bFocused || !m_bFocused && m_fAnim > -int(hge->System_GetState(HGE_SCREENHEIGHT) / 3)) {
        int x, y, w, h;
        hge->Gfx_GetClipping(&x, &y, &w, &h);
        h = std::min(float(h), hge->System_GetState(HGE_SCREENHEIGHT) / 3 + m_fAnim) + 1;
        m_fnt->SetColor(0xFFFFFFFF);
        hge->Gfx_SetClipping(x, y, w, h);
        if (m_sprBG != NULL) {
            for (int y = -1; y < int(int(hge->System_GetState(HGE_SCREENHEIGHT) / 3) / 128) + 2; ++y) {
                for (int x = 0; x < int(hge->System_GetState(HGE_SCREENWIDTH)); x++) {
                    m_sprBG->Render(int(x * 128 - m_fAnimScroll), int(m_fAnim + y * 128 + m_fAnimScroll));
                }
            }
        }

        int start = m_vLines.size() - 1 - m_iScroll;
        if (start < 0) start = 0;
        int linesc = (hge->System_GetState(HGE_SCREENHEIGHT) / 3 + m_fAnim) / 13;
        linesc = m_vLines.size() - linesc - m_iScroll;
        if (linesc < 0)
            linesc = 0;
        for (int i = start; i >= linesc; i--) {
            m_fnt->Render(5, y + (hge->System_GetState(HGE_SCREENHEIGHT) / 3 + m_fAnim) -
                             (m_vLines.size() - i + 1 - m_iScroll) * 13, HGETEXT_LEFT, m_vLines[i], 0);
        }
        m_fnt->SetColor(GV->colFontWhite);
        m_fnt->Render(5, y +(hge->System_GetState(HGE_SCREENHEIGHT) / 3 + m_fAnim) - 13, HGETEXT_LEFT, m_szInput.c_str(), 0);
        m_fnt->Render(5 + m_fnt->GetStringWidth(m_szInput.c_str()), y + (hge->System_GetState(HGE_SCREENHEIGHT) / 3 + m_fAnim) - 13,
                      HGETEXT_LEFT, "_", 0);
        hge->Gfx_SetClipping();
        hge->Gfx_RenderLine(x, y + h, x + w, y + h, 0xFF000000);
    }
}

void SHR::cConsole::Help(int argc, char **argv) {
    if (argc == 0) {
        char list[1024];
        ZeroMemory(&list, 1024);
        strcat(list, "~y~Available commands:~w~");
        for (int i = 0; i < m_vCmds.size(); i++) {
            strcat(list, " ");
            strcat(list, m_vCmds[i]->m_szName);
        }
        Print(list);
        Print("~y~[PAGE UP/PAGE DOWN] ~w~to scroll.");
    } else {
        bool bFound = 0;
        for (int i = 0; i < m_vCmds.size(); i++) {
            if (!strcmp(argv[0], m_vCmds[i]->m_szName)) {
                Print(m_vCmds[i]->m_szDesc);
                bFound = 1;
            }
        }
        if (!bFound)
            Printf("~r~No such function '%s'!~w~", argv[0]);
    }
}

void SHR::cConsole::Print(const char *pszValue) {
    EnterCriticalSection(&csLock);
    if (m_bFile) {
        if (strchr(pszValue, '~') == NULL) {
            *m_osFile << pszValue << std::endl;
        } else {
            char *val = SHR::Replace(pszValue, "<", "&lt;");
            char *val2 = SHR::Replace(val, ">", "&gt;");
            delete[] val;

            val = SHR::Replace(val2, "~y~", "</span><span style='color:yellow;'>");
            delete[] val2;

            val2 = SHR::Replace(val, "~l~", "</span><span style='color:black;'>");
            delete[] val;

            val = SHR::Replace(val2, "~r~", "</span><span style='color:red;'>");
            delete[] val2;

            val2 = SHR::Replace(val, "~g~", "</span><span style='color:green;'>");
            delete[] val;

            val = SHR::Replace(val2, "~b~", "</span><span style='color:blue;'>");
            delete[] val2;

            val2 = SHR::Replace(val, "~w~", "</span><span style='color:white;'>");
            delete[] val;

            val = SHR::Replace(val2, "~a~", "</span><span style='color:gray;'>");
            delete[] val2;

            val2 = SHR::Replace(val, "~p~", "</span><span style='color:pink;'>");
            delete[] val;

            *m_osFile << val2 << std::endl;
            delete[] val2;
        }
    }

#ifdef BUILD_DEBUG
    if (strchr(pszValue, '~') == NULL) {
        printf("%s\n", pszValue);
    }
    else {
        char * val = SHR::Replace(pszValue, "~y~", "");
        char * val2 = SHR::Replace(val, "~l~", "");
        delete[] val;

        val = SHR::Replace(val2, "~r~", "");
        delete[] val2;

        val2 = SHR::Replace(val, "~g~", "");
        delete[] val;

        val = SHR::Replace(val2, "~b~", "");
        delete[] val2;

        val2 = SHR::Replace(val, "~w~", "");
        delete[] val;

        val = SHR::Replace(val2, "~a~", "");
        delete[] val2;

        val2 = SHR::Replace(val, "~p~", "");
        delete[] val;

        printf("%s\n", val2);
        delete[] val2;
    }
#endif

    const char *pch = strchr(pszValue, '\n');

    if (pch == NULL) {
        char *tmp = new char[strlen(pszValue) + 1];
        strcpy(tmp, pszValue);
        tmp[strlen(pszValue)] = '\0';
        m_vLines.push_back(tmp);
        return;
    }

    char *input;
    if (pszValue[strlen(pszValue) - 1] == '\n') {
        input = new char[strlen(pszValue) + 1];
        strcpy(input, pszValue);
    } else {
        input = new char[strlen(pszValue) + 2];
        strcpy(input, pszValue);
        input[strlen(pszValue)] = '\n';
        input[strlen(pszValue) + 1] = '\0';
    }

    pch = strchr(input, '\n');
    int argc = 0;
    while (pch != NULL) {
        argc++;
        pch = strchr(pch + 1, '\n');
    }

    int *lens = new int[argc];
    pch = strchr(input, '\n');
    int it = 0;
    int offset = 0;
    while (pch != NULL) {
        lens[it] = pch - input + 1 - offset;
        offset += lens[it];
        it++;
        pch = strchr(pch + 1, '\n');
    }
    char **argv = new char *[argc];
    for (int i = 0; i < argc; i++) {
        argv[i] = new char[lens[i]];
        ZeroMemory(argv[i], lens[i]);
    }

    offset = 0;
    for (int i = 0; i < argc; i++) {
        for (int y = 0; y < strlen(input) - offset && y < lens[i]; y++) {
            if (input[y + offset] == '\n')
                break;
            else
                argv[i][y] = input[y + offset];
        }
        offset += strlen(argv[i]) + 1;
    }

    for (int i = 0; i < argc; i++) {
        char *tmp = new char[strlen(argv[i]) + 1];
        strcpy(tmp, argv[i]);
        tmp[strlen(argv[i])] = '\0';
        m_vLines.push_back(tmp);
    }

    for (int i = 0; i < argc; i++) {
        delete[] argv[i];
        argv[i] = 0;
    }
    delete[] argv;
    delete[] input;
    delete[] lens;
    LeaveCriticalSection(&csLock);
}

void SHR::cConsole::Printf(const char *pszFormat, ...) {
    char buffer[1024];
    va_list args;
            va_start(args, pszFormat);
    vsprintf(buffer, pszFormat, args);
            va_end(args);
    Print(buffer);
}

void SHR::cConsole::AddCommand(const char *pszName, const char *pszHelp, void(*phFunc)(int, char **)) {
    cConsoleCMD *cmd = new cConsoleCMD;
    cmd->m_szName = new char[strlen(pszName) + 1];
    cmd->m_szDesc = new char[strlen(pszHelp) + 1];
    strcpy(cmd->m_szName, pszName);
    strcpy(cmd->m_szDesc, pszHelp);
    cmd->m_hFunc = phFunc;
    m_vCmds.push_back(cmd);
}

void SHR::cConsole::AddModifiableInt(const char *pszName, int *piVar) {
    cConsoleInt *topush = new cConsoleInt();
    topush->m_szID = new char[strlen(pszName) + 1];
    strcpy(topush->m_szID, pszName);
    topush->m_iPtr = (int) piVar;
    m_vVars.push_back(topush);
}

void SHR::cConsole::AddModifiableFloat(const char *pszName, float *pfVar) {
    cConsoleFloat *topush = new cConsoleFloat();
    topush->m_szID = new char[strlen(pszName) + 1];
    strcpy(topush->m_szID, pszName);
    topush->m_iPtr = (int) pfVar;
    m_vVars.push_back(topush);
}

void SHR::cConsole::AddModifiableBool(const char *pszName, bool *pbVar) {
    cConsoleBool *topush = new cConsoleBool();
    topush->m_szID = new char[strlen(pszName) + 1];
    strcpy(topush->m_szID, pszName);
    topush->m_iPtr = (int) pbVar;
    m_vVars.push_back(topush);
}

void SHR::cConsoleInt::Set(cConsole *handle, int argc, char **argv) {
    int inval = atoi(argv[1]);
    handle->Printf("%s: changing ~y~%d ~w~to ~y~%d~w~.", m_szID, *(int *) m_iPtr, inval);
    *(int *) m_iPtr = inval;
}

void SHR::cConsoleInt::Print(cConsole *handle) {
    handle->Printf("%s: ~y~%i~w~", m_szID, *(int *) m_iPtr);
}

void SHR::cConsoleFloat::Set(cConsole *handle, int argc, char **argv) {
    float inval = atof(argv[1]);
    handle->Printf("%s: changing ~y~%f ~w~to ~y~%f~w~.", m_szID, *(float *) m_iPtr, inval);
    *(float *) m_iPtr = inval;
}

void SHR::cConsoleFloat::Print(cConsole *handle) {
    handle->Printf("%s: ~y~%f~w~", m_szID, *(float *) m_iPtr);
}

void SHR::cConsoleBool::Set(cConsole *handle, int argc, char **argv) {
    bool inval = atoi(argv[1]);
    handle->Printf("%s: changing ~y~%d ~w~to ~y~%i~w~.", m_szID, *(bool *) m_iPtr, inval);
    *(bool *) m_iPtr = inval;
}

void SHR::cConsoleBool::Print(cConsole *handle) {
    handle->Printf("%s: ~y~%i~w~", m_szID, *(bool *) m_iPtr);
}

void SHR::cConsole::ListVars() {
    int fntWidth = m_fnt->GetStringWidth("W");
    int scrWidth = hge->System_GetState(HGE_SCREENWIDTH);
    char *tmpstr = new char[int(scrWidth / fntWidth)];
    ZeroMemory(&tmpstr, int(scrWidth / fntWidth));
    int len = 0;
    for (int i = 0; i < m_vVars.size(); i++) {
        char tmp[32];
        sprintf(tmp, "~y~%16s ~w~|", m_vVars[i]->m_szID);
        strcat(tmpstr, tmp);
        len += 18;
        if (len + 18 > int(scrWidth / fntWidth)) {
            Printf(tmpstr);
            len = 0;
            ZeroMemory(&tmpstr, int(scrWidth / fntWidth));
        }
    }
    if (tmpstr[0] != '\0')
        Printf(tmpstr);
    delete[] tmpstr;
}

void SHR::cConsole::GetVar(int argc, char **argv) {
    if (argc < 1) {
        Printf("~r~Var name not specified.~w~");
        return;
    }

    for (int i = 0; i < m_vVars.size(); i++) {
        if (!strcmp(argv[0], m_vVars[i]->m_szID)) {
            m_vVars[i]->Print(this);
            return;
        }
    }

    Printf("~r~Var '~y~%s~r~' not found.~w~", argv[0]);
}

void SHR::cConsole::SetVar(int argc, char **argv) {
    if (argc < 1) {
        Printf("~r~Var name not specified.~w~");
        return;
    } else if (argc < 2) {
        Printf("~r~Var value not specified.~w~");
        return;
    }

    for (int i = 0; i < m_vVars.size(); i++) {
        if (!strcmp(argv[0], m_vVars[i]->m_szID)) {
            m_vVars[i]->Set(this, argc, argv);
            return;
        }
    }

    Printf("~r~Var '~y~%s~r~' not found.~w~", argv[0]);
}

void SHR::cConsole::DeleteModifiableVar(const char *pszName) {
    for (int i = 0; i < m_vVars.size(); i++) {
        if (!strcmp(pszName, m_vVars[i]->m_szID)) {
            m_vVars.erase(m_vVars.begin() + i);
            return;
        }
    }
}

const char *SHR::cConsole::GetLine(int i) {
    if (i == 0) i = 1;
    int count = m_vLines.size();
    if (count - i >= 0)
        return m_vLines[count - i];
    else
        return NULL;
}

void SHR::cConsole::FixPos() {
    if (!m_bFocused) {
        m_fAnim = -int(hge->System_GetState(HGE_SCREENHEIGHT) / 3);
    }
}
