#include "cControllerIPC.h"
#include "globals.h"
#include "states/editing_ww.h"

cServerIPC::cServerIPC() {
    hMailslot = CreateMailslot(WM_MAILSLOT, MAX_PATH + 1, 0, NULL);
    if (hMailslot == INVALID_HANDLE_VALUE) {
        GV->Console->Printf("~r~Failed to create mailslot: ~y~%d~r~.", GetLastError());
        bOk = 0;
    } else {
        GV->Console->Printf("~g~Mailslot created.");
        bOk = 1;
    }
}

cServerIPC::~cServerIPC() {
    if (bOk)
        CloseHandle(hMailslot);
}

bool cServerIPC::Think() {
    if (!bOk) return false;
    DWORD msgSize;
    bool err;

    err = GetMailslotInfo(hMailslot, 0, &msgSize, 0, 0);

    if (!err)
        GV->Console->Printf("~r~Failed to retrieve mailslot data: ~y~%s~r~.", GetLastError());
    else if (msgSize != (DWORD) MAILSLOT_NO_MESSAGE) {
        GV->Console->Printf("~w~Mailslot message received of ~y~%d ~w~bytes.", msgSize);
        void *buffer;
        buffer = GlobalAlloc(GMEM_FIXED, msgSize);
        if (!buffer)
            GV->Console->Printf("~r~- Error allocating memory for mailslot message.");
        else {
            DWORD numRead;
            err = ReadFile(hMailslot, buffer, msgSize, &numRead, 0);
            if (!err)
                GV->Console->Printf("~r~- Error reading mailslot message: ~y~%d~r~.", GetLastError());
            else if (msgSize != numRead)
                GV->Console->Printf("~r~- Error reading mailslot message, not enough data.");
            else {
                char *msg = (char *) buffer;
                if (msg[0] == WM_IPC_OPENFILE) {
                    GV->Console->Printf("~g~- Remote procedure: open map. Path: %s", &msg[1]);
                    GV->editState->vstrMapsToLoad.emplace_back(&(msg[1]));
                }
                GlobalFree(buffer);
                return true;
            }
        }
    }
    return false;
}

cClientIPC::cClientIPC() {
    bConnected = false;
    Connect();
}

cClientIPC::~cClientIPC() {

}

bool cClientIPC::Connect() {
    if (bConnected) return 1;
    hMailslot = CreateFile(WM_MAILSLOT, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hMailslot == INVALID_HANDLE_VALUE) {
        //GV->Console->Printf("~r~Unable to connect to mailslot: ~y~%s~r~.", GetLastError());
    } else {
        bConnected = 1;
    }
    return bConnected;
}

bool cClientIPC::RemoteOpenMap(const char *szPath) {
    if (!bConnected) return 0;
    int len;
    char *msg;

    if (szPath[0] == '"') {
        len = strlen(szPath) - 1;
        msg = new char[len + 1];
        for (int i = 1; i < len; i++) msg[i] = szPath[i];
    } else {
        len = strlen(szPath) + 1;
        msg = new char[len + 1];
        for (int i = 1; i < len; i++) msg[i] = szPath[i - 1];
    }
    msg[len] = '\0';
    msg[0] = WM_IPC_OPENFILE;

    bool err;
    DWORD numWritten;

    err = WriteFile(hMailslot, msg, len + 1, &numWritten, 0);
    if (!err) {
        //GV->Console->Printf("~r~Unable to send IPC message: ~y~%s~r~.", GetLastError());
        return 0;
    }
    if (numWritten != len + 1) {
        //GV->Console->Printf("~r~Error sending IPC message, not enough data send.");
        return 0;
    }
    return 1;
}
