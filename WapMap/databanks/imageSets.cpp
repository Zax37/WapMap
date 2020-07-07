#include "imageSets.h"
#include "tiles.h"
#include "../../shared/commonFunc.h"
#include "../cObjectUserData.h"
#include "../states/editing_ww.h"
#include "../../shared/HashLib/hashlibpp.h"
#include "../cTextureAtlas.h"
#include "../cParallelLoop.h"

extern structProgressInfo _ghProgressInfo;
extern HGE *hge;

bool cSprBank_SortAssets(cSprBankAsset *a, cSprBankAsset *b) {
    return (std::string(a->GetID()) < std::string(b->GetID()));
}

bool cSprBank_SortAssetImages(cSprBankAssetIMG *a, cSprBankAssetIMG *b) {
    return (a->GetID() < b->GetID());
}

cBankImageSet::cBankImageSet(WWD::Parser *hParser) : cAssetBank(hParser) {
    m_iAssetsSize = 0;
    myAtlaser = new cTextureAtlaser();
}

cBankImageSet::~cBankImageSet() {
    for (auto & m_vAsset : m_vAssets) {
        delete m_vAsset;
    }
    delete myAtlaser;
}

cSprBankAsset *cBankImageSet::GetAssetByID(const char *pszID) {
    for (auto & m_vAsset : m_vAssets) {
        if (!strcmp(m_vAsset->strID.c_str(), pszID)) {
            return m_vAsset;
        }
    }
    return NULL;
}

cSprBankAsset::cSprBankAsset(std::string id) {
    strID = id;
    m_iMaxID = 0;
}

cSprBankAsset::~cSprBankAsset() {
    for (auto & m_vSprite : m_vSprites) {
        delete m_vSprite;
    }
}

void cSprBankAssetIMG::Load() {
    cDC_MountEntry* entry = hParent->GetParent()->GetMountEntry(GetMountPoint());
    if (!entry) {
        GV->Console->Print("~r~Entry not found! (cSprBankAssetIMG::Load)~w~");
        return;
    }
    if (entry->vFiles[0].hFeed != GetFile().hFeed || entry->vFiles[0].strPath != GetFile().strPath) {
        SetFile(entry->vFiles[0]);
    }

    if (!hParent->GetParent()->IsLoadableImage(GetFile(), &imgInfo, cImageInfo::Full))
        return;
    imgSprite = new hgeSprite(0, 0, 0, imgInfo.iWidth, imgInfo.iHeight);
    imgSprite->SetHotSpot(-imgInfo.iOffsetX + imgInfo.iWidth / 2, -imgInfo.iOffsetY + imgInfo.iHeight / 2);

	if (imgInfo.iWidth > hIS->m_iMaxWidth) {
		hIS->m_iMaxWidth = imgInfo.iWidth;
	}

	if (imgInfo.iHeight > hIS->m_iMaxHeight) {
		hIS->m_iMaxHeight = imgInfo.iHeight;
	}

    ((cBankImageSet *) _hBank)->GetTextureAtlaser()->AddSprite(imgSprite);
    ((cBankImageSet *) _hBank)->GetTextureAtlaser()->Pack();

    if (imgSprite->GetTexture() == 0) {
        return;
    }

    float x, y, w, h;
    imgSprite->GetTextureRect(&x, &y, &w, &h);
    hParent->GetParent()->RenderImage(GetFile(), imgSprite->GetTexture(), x, y, 2048);

    size_t lp = GetFile().strPath.find_last_of('/');
    _strName = GetFile().strPath.substr((lp == std::string::npos ? 0 : lp + 1));

    _bLoaded = true;
}

void cSprBankAssetIMG::Unload() {
    if (!_bLoaded) return;
    ((cBankImageSet *) _hBank)->GetTextureAtlaser()->DeleteSprite(imgSprite);
    delete imgSprite;
    imgSprite = 0;
    _bLoaded = false;
}

cSprBankAssetIMG::cSprBankAssetIMG(int it, cBankImageSet *par, cSprBankAsset *is, int id) {
    _bLoaded = false;
    _bForceReload = false;
    hIS = is;
    m_iID = id;
    m_iIT = it;
    imgSprite = 0;
    //imgSprite = new hgeSprite(0, 0, 0, inf.iWidth, inf.iHeight);
    //imgSprite->SetHotSpot(-inf.iOffsetX+inf.iWidth/2, -inf.iOffsetY+inf.iHeight/2);

    cFile f;
    f.hFeed = 0;
    SetFile(f);

    _hBank = par;

    //hAtlaser = par->GetTextureAtlaser();
    //hAtlaser->AddSprite(imgSprite);

    //size_t lp = hFile.strPath.find_last_of('/');
    //_strName = hFile.strPath.substr((lp==std::string::npos ? 0 : lp+1));
    //GV->Console->Printf("scanned: name %s frame %d", m_szName, m_iID);
}

cSprBankAssetIMG::~cSprBankAssetIMG() {
    Unload();
}

std::string cSprBankAssetIMG::GetMountPoint() {
    std::string ret("/");
    ret += _hBank->GetFolderName();
    ret += '/';
    ret += hIS->GetID();
    ret += '/';
    ret += std::to_string(m_iID);
    return ret;
}

void cSprBankAsset::AddIMG(cSprBankAssetIMG *img) {
    m_vSprites.push_back(img);
    SortAndFixIterators();
}

void cSprBankAsset::DeleteIMG(cSprBankAssetIMG *img) {
	bool checkMaxSize = false;
	if (img->GetSprite()->GetWidth() == m_iMaxWidth || img->GetSprite()->GetHeight() == m_iMaxHeight) {
		m_iMaxWidth = 0;
		m_iMaxHeight = 0;
		checkMaxSize = true;
	}

	size_t it = -1;
    for (size_t i = 0; i < m_vSprites.size(); i++) {
        if (m_vSprites[i] == img) {
			it = i;
			if (!checkMaxSize) break;
		}
		else if (checkMaxSize) {
			if (img->GetSprite()->GetWidth() > m_iMaxWidth) {
				m_iMaxWidth = img->GetSprite()->GetWidth();
			}

			if (img->GetSprite()->GetHeight() > m_iMaxHeight) {
				m_iMaxHeight = img->GetSprite()->GetHeight();
			}
		}
    }

	if (it != -1) {
		delete m_vSprites[it];
		m_vSprites.erase(m_vSprites.begin() + it);
		SortAndFixIterators();
		UpdateHash();
	}
}

void cSprBankAsset::SortAndFixIterators() {
    m_iMaxID = -1;
    sort(m_vSprites.begin(), m_vSprites.end(), cSprBank_SortAssetImages);
    for (size_t i = 0; i < m_vSprites.size(); i++) {
        m_vSprites[i]->m_iIT = i;
        if (m_vSprites[i]->m_iID > m_iMaxID)
            m_iMaxID = m_vSprites[i]->m_iID;
    }
    UpdateHash();
}

void cSprBankAsset::UpdateHash() {
    std::string acchash;
    for (size_t i = 0; i < m_vSprites.size(); i++)
        acchash.append(m_vSprites[i]->GetHash());
    hashwrapper *hasher = new md5wrapper();
    strHash = hasher->getHashFromString(acchash);
    delete hasher;
}

cSprBankAssetIMG *cSprBankAsset::GetIMGByID(int id) {
    for (int i = 0; i < m_vSprites.size(); i++) {
        if (id == m_vSprites[i]->GetID())
            return m_vSprites[i];
    }
    return 0;
}

cSprBankAssetIMG *cSprBankAsset::GetIMGByIterator(int it) {
    if (it >= m_vSprites.size()) {
        it = m_vSprites.size() - 1;
    }
    return m_vSprites[it];
}

cSprBankAssetIMG *cSprBankAsset::GetIMGByNoID(int noid) {
    for (int i = 0; i < m_vSprites.size(); i++)
        if (m_vSprites[i]->GetID() == noid)
            return m_vSprites[i];
    return m_vSprites[0];
}

hgeSprite *cBankImageSet::GetObjectSprite(WWD::Object *obj) {
    int i = obj->GetI();
    if (strstr(obj->GetImageSet(), "FONT") != NULL)
        //return GetSpriteFromAsset(obj->GetImageSet(), std::max(obj->GetParam(WWD::Param_LocationI), 0));
        return GetSpriteFromAsset(obj->GetImageSet(), std::max(i, 0));
    else if (GV->editState->hTileset->GetSet(obj->GetImageSet()) != 0) {
        cTile *t = GV->editState->hTileset->GetSet(obj->GetImageSet())->GetTile(i);
        if (!t) return GV->sprSmiley;
        t->GetImage()->SetHotSpot(t->GetImage()->GetWidth() / 2, t->GetImage()->GetHeight() / 2);
        return t->GetImage();
    } else {
        if (!strcmp(obj->GetLogic(), "GroundBlower") && !strcmp(obj->GetImageSet(), "LEVEL_BLOWING1")) {
            return GetSpriteFromAsset("LEVEL_BLOWING1", 4);
        }
        //return GetSpriteFromAsset(obj->GetImageSet(), std::max(obj->GetParam(WWD::Param_LocationI)-1, 0));
        if (!strcmp(obj->GetLogic(), "TigerGuard") && obj->GetParam(WWD::Param_Smarts) == 1)
            return GetSpriteFromAsset("LEVEL_TIGERWHITE", i);
        else if (!strcmp(obj->GetLogic(), "HermitCrab") && obj->GetUserValue(0) != 0)
            return GetSpriteFromAsset("LEVEL_BOMBERCRAB", i);
        else
            return GetSpriteFromAsset(obj->GetImageSet(), i);
    }
}

hgeSprite *cBankImageSet::GetSpriteFromAsset(const char *asset, int frame) {
    if (GV->bKijan)
        return GV->sprKijan;
    else if (GV->bZax)
        return GV->sprZax;
    else {
        cSprBankAsset *as = GetAssetByID(asset);
        if (as != NULL)
            return as->GetIMGByNoID(frame)->GetSprite();
    }
    return GV->sprSmiley;
}

WWD::Rect cBankImageSet::GetSpriteRenderRect(hgeSprite *spr) {
    WWD::Rect ret;
    float hsx, hsy;
    spr->GetHotSpot(&hsx, &hsy);
    ret.x1 = -hsx;
    ret.y1 = -hsy;
    ret.x2 = spr->GetWidth() - hsx;
    ret.y2 = spr->GetHeight() - hsy;
    return ret;
}

WWD::Rect cBankImageSet::GetObjectRenderRect(WWD::Object *obj) {
    hgeSprite *spr = GetObjectSprite(obj);
    WWD::Rect ret;
    if (!spr) return ret;
    float hsx, hsy;
    spr->GetHotSpot(&hsx, &hsy);
    ret.x2 = spr->GetWidth();
    ret.y2 = spr->GetHeight();
    float sprw = ret.x2 / 2, sprh = ret.y2 / 2;
    hsx -= sprw;
    hsy -= sprh;
    ret.x1 = (obj->GetX() - sprw - hsx);
    ret.y1 = (obj->GetY() - sprh - hsy);

    if (!strcmp(obj->GetLogic(), "FrontStackedCrates") || !strcmp(obj->GetLogic(), "BackStackedCrates")) {
        int irepeatnum = 0;
        for (int z = 0; z < 2; z++) {
            WWD::Rect tmprect = obj->GetUserRect(z);
            if (tmprect.x1 != 0) irepeatnum++;
            if (tmprect.y1 != 0) irepeatnum++;
            if (tmprect.x2 != 0) irepeatnum++;
            if (tmprect.y2 != 0) irepeatnum++;
        }
        if (!obj->GetParam(WWD::Param_Powerup)) irepeatnum--;
        for (int z = 0; z < irepeatnum; z++) {
            ret.y1 -= 43;
            ret.y2 += 43;
        }
    } else {
        if (obj->GetFlipX()) {
            ret.x1 += 2 * hsx;
        }

        if (obj->GetFlipY()) {
            ret.y1 += 2 * hsy;
        }

        if (!strcmp(obj->GetLogic(), "BreakPlank")) {
            for (int z = 0; z < obj->GetParam(WWD::Param_Width) - 1; z++)
                ret.x2 += 64;
        } else if (!strcmp(obj->GetLogic(), "PunkRat")) {
            ret.x1 -= 29;
            ret.x2 += 44;
            ret.y2 += 46;
        }
    }

    return ret;
}

std::string cBankImageSet::getElementAt(int i) {
    if (i < 0 || i >= m_vAssets.size()) return "";
    return m_vAssets[i]->GetID();
}

int cBankImageSet::getNumberOfElements() {
    return m_vAssets.size();
}

void cBankImageSet::SortAssets() {
    sort(m_vAssets.begin(), m_vAssets.end(), cSprBank_SortAssets);
}

void cBankImageSet::BatchProcessStart(cDataController *hDC) {
    GV->Console->Printf("Scanning image sets...");
    _ghProgressInfo.iGlobalProgress = 6;
    _ghProgressInfo.strGlobalCaption = "Scanning image sets...";
    _ghProgressInfo.iDetailedProgress = 0;
    _ghProgressInfo.iDetailedEnd = 100000;
}

void cBankImageSet::BatchProcessEnd(cDataController *hDC) {
    cParallelLoop *looper = hDC->GetLooper();
    _ghProgressInfo.strDetailedCaption = "Packing textures...";
    _ghProgressInfo.iDetailedProgress = 50000;
    _ghProgressInfo.iDetailedEnd = 100000;
    if (looper != 0)
        looper->Tick();
    SortAssets();
    myAtlaser->Pack();
    int spritec = 0;
    for (auto & m_vAsset : m_vAssets)
        spritec += m_vAsset->m_vSprites.size();
    GV->Console->Printf("%d images in %d image sets found. Loading...", spritec, m_vAssets.size());

    int progress = 0;
    for (auto & m_vAsset : m_vAssets) {
        bool bAffected = false;
        for (size_t x = 0; x < m_vAsset->m_vSprites.size(); x++) {
            if (m_vAsset->m_vSprites[x]->IsLoaded())
                continue;
            char buf[256];
            sprintf(buf, "Rendering: %s [%d/%d]", m_vAsset->GetID(), x, m_vAsset->m_vSprites.size());
            _ghProgressInfo.strDetailedCaption = buf;
            _ghProgressInfo.iDetailedProgress = 50000 + (float(progress) / float(spritec)) * 50000.0f;
            if (looper) looper->Tick();
            m_vAsset->m_vSprites[x]->Load();
            progress++;
            bAffected = true;
        }
        if (bAffected)
            m_vAsset->UpdateHash();
    }
}

std::string cBankImageSet::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    size_t lslash = strFilePath.rfind('/');
    if (lslash == strFilePath.length() - 1)
        return "";
    std::string strSet(strPrefix);
    if (lslash != std::string::npos) {
        strSet.push_back('_');
        strSet.append(strFilePath.c_str(), lslash);
    }
    std::string strFile(lslash == std::string::npos ? strFilePath : strFilePath.c_str() + lslash);
    const char* fileDot = strrchr(strFile.c_str(), '.');
    if (!fileDot || !canReadExtension(fileDot + 1))
        return "";
    std::transform(strSet.begin(), strSet.end(), strSet.begin(), ::toupper);
    size_t slash = strSet.find('/');
    while (slash != std::string::npos) {
        strSet[slash] = '_';
        slash = strSet.find('/');
    }

    size_t numpos = strFile.find_first_of("1234567890");
    int tid = 0;
    if (numpos != std::string::npos) {
        while (1) {
            if (numpos >= strFile.length() || strFile[numpos] < 48 || strFile[numpos] > 57) break;
            tid *= 10;
            tid += int(strFile[numpos]) - 48;
            numpos++;
        }
    }

    return "/" + GetFolderName() + '/' + strSet + '/' + std::to_string(tid);
}

cAsset *cBankImageSet::AllocateAssetForMountPoint(cDataController *hDC, cDC_MountEntry mountEntry) {
    size_t lslash = mountEntry.strMountPoint.rfind('/');
    std::string strID = mountEntry.strMountPoint.substr(8, lslash - 8),
            strFrame = mountEntry.strMountPoint.substr(lslash + 1);
    int iFrame = atoi(strFrame.c_str());

    cSprBankAsset *as = GetAssetByID(strID.c_str());
    if (!as) {
        as = new cSprBankAsset(strID);
        m_vAssets.push_back(as);
    }
    cSprBankAssetIMG *img = new cSprBankAssetIMG(as->m_vSprites.size(), this, as, iFrame);
    as->AddIMG(img);
    return img;
}

void cBankImageSet::DeleteAsset(cAsset *hAsset) {
    std::string strMount = hAsset->GetMountPoint();
    strMount = strMount.substr(8);
    size_t pos = strMount.find('/');
    if (pos == std::string::npos) return;
    std::string strSet = strMount.substr(0, pos),
            strID = strMount.substr(pos + 1);
    int iID = atoi(strID.c_str());
    cSprBankAsset *set = GetAssetByID(strSet.c_str());
    if (!set) return;
    cSprBankAssetIMG *img = set->GetIMGByID(iID);
    if (!img) return;
    set->DeleteIMG(img);
    if (set->GetSpritesCount() == 0) {
        for (size_t i = 0; i < m_vAssets.size(); i++)
            if (m_vAssets[i] == set) {
                delete m_vAssets[i];
                m_vAssets.erase(m_vAssets.begin() + i);
                return;
            }
    }
}

bool cBankImageSet::canReadExtension(const char *ext) {
    if (ext[0] == 'P' || ext[0] == 'p') {
        if ((ext[1] == 'I' || ext[1] == 'i')) {
            return (ext[2] == 'D' || ext[2] == 'd') && ext[3] == 0;
        } else return (ext[1] == 'C' || ext[1] == 'c')
            && (ext[2] == 'X' || ext[2] == 'x')
            && ext[3] == 0;
    } else return ((ext[0] == 'B' || ext[0] == 'b')
               && (ext[1] == 'M' || ext[1] == 'm')
               && (ext[2] == 'P' || ext[2] == 'p')
               && ext[3] == 0);
}
