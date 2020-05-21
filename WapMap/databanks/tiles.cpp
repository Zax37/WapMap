#include "tiles.h"
#include <algorithm>
#include <sys\types.h>
#include <sys\stat.h>
#include "../globals.h"
#include "../globlua.h"
#include "../langID.h"
#include "../../shared/commonFunc.h"
#include "../cParallelLoop.h"
#include "../../shared/cProgressInfo.h"
#include "../../shared/HashLib/hashlibpp.h"
#include "../../shared/cWWD.h"
#include "../cBrush.h"
#include "../states/editing_ww.h"

extern structProgressInfo _ghProgressInfo;
extern HGE *hge;

bool cTileBank_SortTilesets(cTileImageSet *a, cTileImageSet *b) {
    return (std::string(a->GetName()) < std::string(b->GetName()));
}

bool cTileBank_SortTiles(cTile *a, cTile *b) {
    return (a->GetID() < b->GetID());
}

cBankTile::cBankTile(WWD::Parser *pr) : cAssetBank(pr) {
    bReloadBrushes = 0;
}

cBankTile::~cBankTile() {
    for (int i = 0; i < m_vhSets.size(); i++)
        delete m_vhSets[i];
}

cTileImageSet::cTileImageSet(const char *pszName) {
    m_iTileMaxID = -1;
    m_szName = new char[strlen(pszName) + 1];
    strcpy(m_szName, pszName);
}

void cTilesetTexture::CalculateDimension(int iTileNum, int &iTexW, int &iTexH) {
    int txw = 512, txh = 512; //SHR::RoundTo2Power(ovtcount/8);
    while (txw * txh < iTileNum * (64 * 64)) {
        if (txw < txh) txw += txw;
        else txh += txh;
    }
    txw = SHR::RoundTo2Power(txw);
    txh = SHR::RoundTo2Power(txh);
    iTexW = txw;
    iTexH = txh;
}

void cBankTile::BatchProcessStart(cDataController *hDC) {
    GV->Console->Printf("Loading tiles...");
    _ghProgressInfo.iGlobalProgress = 5;
    _ghProgressInfo.strGlobalCaption = "Loading tiles...";
    _ghProgressInfo.iDetailedEnd = 100000;
    bReloadBrushes = 0;
}

void cBankTile::BatchProcessEnd(cDataController *hDC) {
    for (int i = 0; i < m_vhSets.size(); i++) {
        m_vhSets[i]->Sort();
    }
    if (bReloadBrushes) {
        if (GV->editState->iActiveTool == EWW_TOOL_BRUSH) {
            GV->editState->HandleBrushSwitch(GV->editState->iTilePicked, EWW_TILE_NONE);
            GV->editState->iTilePicked = EWW_TILE_NONE;
        }
        ReloadBrushes();
        if (GV->editState->iActiveTool == EWW_TOOL_BRUSH) {
            GV->editState->RebuildTilePicker();
        }
    }

    int toloadc = 0;
    int sets = 0;
    for (int i = 0; i < m_vhSets.size(); i++) {
        bool f = 0;
        for (int x = 0; x < m_vhSets[i]->GetTilesCount(); x++) {
            cTile *t = m_vhSets[i]->GetTileByIterator(x);
            if (t->NeedReload()) {
                t->Unload();
                t->SetForceReload(0);
            }
            if (!t->IsLoaded()) {
                toloadc++;
                f = 1;
            }
        }
        if (f) sets++;
    }

    GV->Console->Printf("~g~Found %d tiles in %d tile sets.", toloadc, sets);
    int texidx = 0;
    if (vTexes.empty()) {
        int txw, txh;
        cTilesetTexture::CalculateDimension(toloadc, txw, txh);
        cTilesetTexture *ntex = new cTilesetTexture(txw / 64, txh / 64);
        vTexes.push_back(ntex);
        GV->Console->Printf("~w~Registered %dx%d tile texture to store %d tiles.", txw, txh, toloadc);
    }
    if (hDC->GetLooper() != 0)
        hDC->GetLooper()->Tick();

    int caret = 0;
    for (size_t x = 0; x < m_vhSets.size(); x++) {
        for (int y = 0; y < m_vhSets[x]->GetTilesCount(); y++) {
            cTile *tile = m_vhSets[x]->GetTileByIterator(y);
            if (tile->IsLoaded())
                continue;
            _ghProgressInfo.iDetailedProgress = 50000 + 50000 * (float(toloadc - caret) / float(toloadc));
            char buf[256];
            sprintf(buf, "Rendering: %s [#%d]", m_vhSets[x]->GetName(), tile->GetID());
            _ghProgressInfo.strDetailedCaption = buf;
            if (hDC->GetLooper() != 0)
                hDC->GetLooper()->Tick();

            while (vTexes[texidx]->GetFreeSlotsNum() == 0) {
                if (texidx == vTexes.size() - 1) {
                    int txw, txh;
                    cTilesetTexture::CalculateDimension(toloadc - caret, txw, txh);
                    cTilesetTexture *ntex = new cTilesetTexture(txw / 64, txh / 64);
                    vTexes.push_back(ntex);
                    GV->Console->Printf("~w~Registered %dx%d tile texture to store %d tiles.", txw, txh,
                                        toloadc - caret);
                }
                texidx++;
            }
            tile->myTexture = vTexes[texidx];
            tile->Load();
            if (hDC->GetLooper() != 0)
                hDC->GetLooper()->Tick();
            caret++;
        }
    }
    if (bReloadBrushes) {
        SortTilesets();
    }
}

void cBankTile::SortTilesets() {
    sort(m_vhSets.begin(), m_vhSets.end(), cTileBank_SortTilesets);
}

void cBankTile::DeleteAsset(cAsset *hAsset) {
    std::string strMount = hAsset->GetMountPoint();
    strMount = strMount.substr(7);
    size_t pos = strMount.find('/');
    if (pos == std::string::npos) return;
    std::string strTileset = strMount.substr(0, pos),
            strTileID = strMount.substr(pos + 1);
    int iTileID = atoi(strTileID.c_str());
    cTileImageSet *set = GetSet(strTileset.c_str());
    if (!set) return;
    cTile *tile = set->GetTile(iTileID);
    if (!tile) return;
    set->DeleteTile(tile);
    delete tile;
    if (set->GetTilesCount() == 0) {
        for (size_t i = 0; i < m_vhSets.size(); i++)
            if (m_vhSets[i] == set) {
                delete set;
                m_vhSets.erase(m_vhSets.begin() + i);
                return;
            }
    }
}

void cTileImageSet::AddTile(cTile *t) {
    m_vTiles.push_back(t);
    if (t->GetID() > m_iTileMaxID)
        m_iTileMaxID = t->GetID();
    else
        Sort();
    UpdateHash();
}

void cTileImageSet::DeleteTile(cTile *t) {
    for (size_t i = 0; i < m_vTiles.size(); i++) {
        if (m_vTiles[i] == t) {
            m_vTiles.erase(m_vTiles.begin() + i);
            break;
        }
    }
    if (m_iTileMaxID == t->GetID()) {
        m_iTileMaxID = -1;
        for (size_t i = 0; i < m_vTiles.size(); i++)
            if (m_vTiles[i]->GetID() > m_iTileMaxID)
                m_iTileMaxID = m_vTiles[i]->GetID();
    }
    UpdateHash();
}

void cBankTile::ReloadBrushes() {
    GV->Console->Printf("~w~Reloading brushes...");
    if (!m_vhSets.size())
        return;

    for (int i = 0; i < m_vhSets.size(); i++)
        m_vhSets[i]->m_vBrushes.clear();

    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile("brush/*", &fdata);

    if (hFind == INVALID_HANDLE_VALUE) {
        GV->Console->Printf("~r~No brushes found (unable to access 'brush' directory).");
        return;
    }

    char sign;
    if (hParser->GetGame() == WWD::Game_Claw)
        sign = 'c';
    else if (hParser->GetGame() == WWD::Game_GetMedieval)
        sign = 'm';
    else if (hParser->GetGame() == WWD::Game_Gruntz)
        sign = 'g';
    else
        sign = 'x';
    std::vector<std::string> vszLayers;
    for (int i = 0; i < m_vhSets.size(); i++) {
        vszLayers.push_back(std::string(m_vhSets[i]->m_szName));
    }
    do {
        if (fdata.cFileName[0] == '.' || fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        struct stat StatBuffer;
        char temp[256];
        sprintf(temp, "brush/%s", fdata.cFileName);
        stat(temp, &StatBuffer);
        if (!(StatBuffer.st_mode & S_IFDIR)) {
            char *ext = SHR::GetExtension(fdata.cFileName);
            char *lext = SHR::ToLower(ext);
            delete[] ext;
            if (fdata.cFileName[0] == sign && !strcmp("lua", lext) || !strcmp("wbp", lext)) {
                cBrush *tmp = NULL;
                if (!strcmp("lua", lext))
                    while (1) {
                        tmp = new cBrush(fdata.cFileName, hParser, vszLayers);
                        if (tmp->GetStatus() == BrushLuaError || tmp->GetStatus() == BrushOtherError) {
                            char caption[256];
                            sprintf(caption, "%s: %s", GETL(Lang_BrushErrorCaption), fdata.cFileName);
                            char desc[256];
                            if (tmp->GetStatus() == BrushLuaError) {
                                sprintf(desc, "%s\n%s", GETL(Lang_BrushLuaError), tmp->GetErrorInfo());
                            } else {
                                sprintf(desc, "%s\n%s", GETL(Lang_BrushOtherError), tmp->GetErrorInfo());
                            }
                            delete tmp;
                            int r = MessageBox(hge->System_GetState(HGE_HWND), desc, caption,
                                               MB_RETRYCANCEL | MB_ICONERROR | MB_DEFBUTTON2 | MB_APPLMODAL);
                            if (r == 5) { //continue
                                GV->Console->Printf("~r~Error loading brush; skipping~w~ (~y~%s~w~)", fdata.cFileName);
                                break;
                            } else if (r == 4) { //retry
                                GV->Console->Printf("~r~Error loading brush; retrying~w~ (~y~%s~w~)", fdata.cFileName);
                                continue;
                            }
                        } else if (tmp->GetStatus() == BrushOK) {
                            cTileImageSet *set = GetSet(tmp->GetLayerName());
                            tmp->SetOwner(set);
                            set->m_vBrushes.push_back(tmp);
                            break;
                        } else {
                            GV->Console->Printf("~r~Rejected brush ~y~%s~r~.", fdata.cFileName);
                            delete tmp;
                            break;
                        }
                    }
                else {
                    char tmpch[128];
                    sprintf(tmpch, "brush/%s", fdata.cFileName);
                    FILE *f = fopen(tmpch, "rb");
                    if (!f) continue;
                    fread(&tmpch, 4, 1, f);
                    if (tmpch[0] != 'W' || tmpch[1] != 'B' || tmpch[2] != 'P' || tmpch[3] != '0') continue;
                    int filesc = 0;
                    fread(&filesc, 4, 1, f);

                    if (filesc == 0) continue;
                    GV->Console->Printf("Parsing ~y~%s ~w~package...", fdata.cFileName);

                    for (int i = 0; i < filesc; i++) {
                        fread(&tmpch, 64, 1, f);
                        int pos, len;
                        fread(&pos, 4, 1, f);
                        fread(&len, 4, 1, f);
                        int actpos = ftell(f);
                        fseek(f, pos, SEEK_SET);
                        char *buf = new char[len + 1];
                        fread(buf, len, 1, f);
                        fseek(f, actpos, SEEK_SET);
                        buf[len] = '\0';

                        tmp = new cBrush(tmpch, hParser, vszLayers, buf);
                        delete[] buf;
                        if (tmp->GetStatus() == BrushLuaError || tmp->GetStatus() == BrushOtherError) {
                            GV->Console->Printf("~r~Error loading brush; forced skipping~w~ (~y~%s~w~)", tmpch);
                            delete tmp;
                        } else if (tmp->GetStatus() == BrushOK) {
                            cTileImageSet *set = GetSet(tmp->GetLayerName());
                            tmp->SetOwner(set);
                            set->m_vBrushes.push_back(tmp);
                        }
                    }
                    GV->Console->Printf("End of ~y~%s ~w~package.", fdata.cFileName);

                    fclose(f);
                }
            }
            delete[] lext;
        }
    } while (FindNextFile(hFind, &fdata) != 0);
    GV->Console->Printf("~g~Brushes reloaded for ~y~%d ~g~image sets.", int(m_vhSets.size()));
}

void cTileImageSet::Sort() {
    std::sort(m_vTiles.begin(), m_vTiles.end(), cTileBank_SortTiles);
}

cTileImageSet::~cTileImageSet() {
    delete[] m_szName;
    for (int i = 0; i < m_vTiles.size(); i++)
        delete m_vTiles[i];
    for (int i = 0; i < m_vBrushes.size(); i++)
        delete m_vBrushes[i];
}

cTile *cBankTile::FindTile(short piID) {
    for (int i = 0; i < m_vhSets.size(); i++) {
        cTile *r = m_vhSets[i]->GetTile(piID);
        if (r != NULL)
            return r;
    }
    return NULL;
}

cTile *cTileImageSet::GetTile(short piID) {
    for (int i = 0; i < m_vTiles.size(); i++) {
        if (m_vTiles[i]->GetID() == piID)
            return m_vTiles[i];
    }
    return NULL;
}

cTile *cBankTile::GetTile(const char *pszSet, short piID) {
    if (pszSet == NULL) return NULL;
    for (int i = 0; i < m_vhSets.size(); i++) {
        if (!strcmp(pszSet, m_vhSets[i]->m_szName)) {
            return m_vhSets[i]->GetTile(piID);
        }
    }
    return NULL;
}

cTileImageSet *cBankTile::GetSet(const char *pszSet, bool bCaseSensitive) {
    if (pszSet == NULL) return NULL;
    for (int i = 0; i < m_vhSets.size(); i++) {
        if (bCaseSensitive && !strcmp(pszSet, m_vhSets[i]->m_szName) ||
            !bCaseSensitive && !strcmpi(pszSet, m_vhSets[i]->m_szName)) {
            return m_vhSets[i];
        }
    }
    return NULL;
}

cTilesetTexture::cTilesetTexture(int w, int h) {
    hTex = hge->Texture_Create(w * 64, h * 64);
    iLastSlotX = iLastSlotY = -1;
    iW = w;
    iH = h;
    iUsedSlots = 0;
    hSlots = new hgeSprite *[w * h];
    for (int i = 0; i < w * h; i++)
        hSlots[i] = 0;
    DWORD *tdatatmp = hge->Texture_Lock(hTex, 0);
    for (int i = 0; i < (w * 64) * (h * 64); i++)
        tdatatmp[i] = 0x00FFFFFF;
    hge->Texture_Unlock(hTex);
}

cTilesetTexture::~cTilesetTexture() {
    delete hSlots;
    hge->Texture_Free(hTex);
}

DWORD *cTilesetTexture::GetFreeSlot() {
    if (GetFreeSlotsNum() == 0) return 0;
    int sx = -1, sy = -1;
    bool stop = 0;
    for (int y = 0; y < iH; y++) {
        for (int x = 0; x < iW; x++)
            if (hSlots[y * iW + x] == 0) {
                sx = x;
                sy = y;
                stop = 1;
                break;
            }
        if (stop) break;
    }
    if (!stop) return 0;
    //DWORD * r = hge->Texture_Lock(hTex, 0, sx*64, sy*64, 64, 64);
    iLastSlotX = sx;
    iLastSlotY = sy;
    return 0;
}

void cTilesetTexture::SaveLastSlot(hgeSprite *hSpr) {
    if (iLastSlotX != -1 && iLastSlotY != -1) {
        hSlots[iLastSlotX + iLastSlotY * iW] = hSpr;
        hge->Texture_Unlock(hTex);
    }
    iUsedSlots++;
    iLastSlotX = iLastSlotY = -1;
}

void cTilesetTexture::FreeSlot(hgeSprite *hSpr) {
    for (int y = 0; y < iH; y++)
        for (int x = 0; x < iW; x++) {
            if (hSlots[y * iW + x] == hSpr) {
                hSlots[y * iW + x] = 0;
                iUsedSlots--;
                return;
            }
        }
}

int cTilesetTexture::GetRowSpan() {
    return iW * 64;
}

int cTilesetTexture::GetFreeSlotsNum() {
    return (iW * iH) - iUsedSlots;
}

std::string cBankTile::getElementAt(int i) {
    if (i < 0 || i >= m_vhSets.size()) return "";
    return std::string(m_vhSets[i]->GetName());
}

int cBankTile::getNumberOfElements() {
    if (m_vhSets.size() == 0) return 1;
    return m_vhSets.size();
}

cTile::cTile(cImageInfo inf, int id, cTileImageSet *ts, cBankTile *bank) {
    imgInfo = inf;
    iID = id;
    myTexture = 0;
    imgSprite = 0;
    _bLoaded = 0;
    _bForceReload = 0;
    hTS = ts;
    _hBank = bank;
    cFile f;
    f.hFeed = 0;
    SetFile(f);
}

cTile::~cTile() {
    Unload();
}

std::string cTile::GetMountPoint() {
    char tmp[16];
    sprintf(tmp, "%d", iID);
    return std::string("/") + _hBank->GetFolderName() + '/' + hTS->GetName() + "/" + tmp;
}

cAsset *cBankTile::AllocateAssetForMountPoint(cDataController *hDC, cDC_MountEntry mountEntry) {
    cFile f;
    cImageInfo imginf;
    bool bfnd = 0;
    for (size_t i = 0; i < mountEntry.vFiles.size(); i++) {
        if (!hDC->IsLoadableImage(mountEntry.vFiles[i], &imginf)) {
            char *src;
            if (mountEntry.vFiles[i].hFeed == hDC->GetFeed(DB_FEED_REZ))
                src = "REZ";
            else if (mountEntry.vFiles[i].hFeed == hDC->GetFeed(DB_FEED_DISC))
                src = "HDD";
            else if (mountEntry.vFiles[i].hFeed == hDC->GetFeed(DB_FEED_CUSTOM))
                src = "CUS";
            GV->Console->Printf("~r~'~y~[%s]/%s~r~' is not a supported image file.", src,
                                mountEntry.vFiles[i].strPath.c_str());
            continue;
        } else {
            bfnd = 1;
            f = mountEntry.vFiles[i];
            break;
        }
    }
    if (!bfnd) return 0;

    size_t lslash = mountEntry.strMountPoint.rfind('/');
    std::string imgset = mountEntry.strMountPoint.substr(7, lslash - 7),
                strid = mountEntry.strMountPoint.substr(lslash + 1);

    int id;
    sscanf(strid.c_str(), "%d", &id);

    std::transform(imgset.begin(), imgset.end(), imgset.begin(), ::toupper);

    cTileImageSet *is = GetSet(imgset.c_str());
    if (!is) {
        is = new cTileImageSet(imgset.c_str());
        AddTileset(is);
        GV->Console->Printf("~g~Registered tileset '~y~%s~g~'.", imgset.c_str());
        bReloadBrushes = 1;
    }
    cTile *n = new cTile(imginf, id, is, this);
    is->AddTile(n);
    return n;
}

void cTile::Load() {
    if (!myTexture) return;

    if (!hParent) {
        GV->Console->Printf("~r~Trying to load unregistered asset MP:~y~%s~r~.", GetMountPoint().c_str());
        return;
    }

    cDC_MountEntry* entry = hParent->GetParent()->GetMountEntry(GetMountPoint());
    if (!entry) {
        GV->Console->Print("~r~Entry not found! (cTile::Load)~w~");
        return;
    }

    int iSlotX, iSlotY;
    myTexture->GetFreeSlot();
    myTexture->GetLastSlotPos(iSlotX, iSlotY);

    imgSprite = new hgeSprite(myTexture->GetTexture(), iSlotX * 64, iSlotY * 64, 64, 64);
    for (auto file : entry->vFiles) {
        if (file.hFeed != GetFile().hFeed ||
            file.strPath != GetFile().strPath)
            SetFile(file);

        if (hParent->GetParent()->RenderImage(GetFile(), myTexture->GetTexture(), iSlotX * 64, iSlotY * 64,
                                              myTexture->GetRowSpan())) {
            myTexture->SaveLastSlot(imgSprite);
            _bLoaded = 1;
            return;
        }
    }
}

void cTile::Unload() {
    if (!_bLoaded)
        return;
    myTexture->FreeSlot(imgSprite);
    delete imgSprite;
    _bLoaded = 0;
}

void cTileImageSet::UpdateHash() {
    std::string acchash;
    for (size_t i = 0; i < m_vTiles.size(); i++)
        acchash.append(m_vTiles[i]->GetHash());
    hashwrapper *hasher = new md5wrapper();
    strHash = hasher->getHashFromString(acchash);
    delete hasher;
}

std::string cBankTile::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    size_t cutpos = strFilePath.find('/');
    if (cutpos == std::string::npos || cutpos == strFilePath.length() - 1)
        return "";
    std::string strTileSet = strFilePath.substr(0, cutpos),
            strFileName = strFilePath.substr(cutpos + 1);

    size_t numpos = strFileName.find_first_of("1234567890");
    if (numpos == std::string::npos) return "";
    int tid = 0;
    while (1) {
        if (numpos >= strFileName.length() || strFileName[numpos] < 48 || strFileName[numpos] > 57) break;
        tid *= 10;
        tid += int(strFileName[numpos]) - 48;
        numpos++;
    }
    char buf[16];
    sprintf(buf, "%d", tid);
    return std::string("/") + GetFolderName() + '/' + strTileSet + "/" + buf;
}
