#include "imageSets.h"
#include "tiles.h"
#include "../../shared/commonFunc.h"
#include "../cObjectUserData.h"

#include "../states/editing_ww.h"

#include "../globals.h"

#include "../../shared/HashLib/hashlibpp.h"
#include "../cTextureAtlas.h"
#include "../cParallelLoop.h"
#include "../../shared/cProgressInfo.h"

extern structProgressInfo _ghProgressInfo;
extern HGE * hge;

bool cSprBank_SortAssets(cSprBankAsset * a, cSprBankAsset * b)
{
    return (std::string(a->GetID())<std::string(b->GetID()));
}

bool cSprBank_SortAssetImages(cSprBankAssetIMG * a, cSprBankAssetIMG * b)
{
    return (a->GetID() < b->GetID());
}

cBankImageSet::cBankImageSet()
{
    iGame = WWD::Game_Unknown;
    hREZ = 0;
    m_iAssetsSize = 0;
    bBatchProcess = 0;
    myAtlaser = new cTextureAtlaser();
}

cBankImageSet::~cBankImageSet()
{
    for(int i=0;i<m_vAssets.size();i++){
     delete m_vAssets[i];
     m_vAssets[i] = 0;
    }
    delete myAtlaser;
}

cSprBankAsset * cBankImageSet::GetAssetByID(const char * pszID)
{
    for(int i=0;i<m_vAssets.size();i++){
     if( !strcmp(m_vAssets[i]->strID.c_str(), pszID) ){
      return m_vAssets[i];
     }
    }
    return NULL;
}

cSprBankAsset::cSprBankAsset(std::string id)
{
    strID = id;
    m_iMaxID = 0;
}

cSprBankAsset::~cSprBankAsset()
{
    for(int i=0;i<m_vSprites.size();i++){
     delete m_vSprites[i];
     m_vSprites[i] = 0;
    }
}

void cSprBankAssetIMG::Load()
{
    cDC_MountEntry myEntry = hParent->GetParent()->GetMountEntry(GetMountPoint());
    printf("mnt entry: %s\n files: %d\n", myEntry.strMountPoint.c_str(), myEntry.vFiles.size());
    if( myEntry.vFiles[0].hFeed != GetFile().hFeed ||
        myEntry.vFiles[0].strPath != GetFile().strPath )
     SetFile(myEntry.vFiles[0]);

    if( !hParent->GetParent()->IsLoadableImage(GetFile(), &imgInfo, cImageInfo::Full) )
     return;
    imgSprite = new hgeSprite(0, 0, 0, imgInfo.iWidth, imgInfo.iHeight);
    imgSprite->SetHotSpot(-imgInfo.iOffsetX+imgInfo.iWidth/2, -imgInfo.iOffsetY+imgInfo.iHeight/2);

    ((cBankImageSet*)_hBank)->GetTextureAtlaser()->AddSprite(imgSprite);
    ((cBankImageSet*)_hBank)->GetTextureAtlaser()->Pack();

    if( imgSprite->GetTexture() == 0 )
     return;

    float x, y, w, h;
    imgSprite->GetTextureRect(&x, &y, &w, &h);
    hParent->GetParent()->RenderImage(GetFile(), imgSprite->GetTexture(), x, y, 2048);

    size_t lp = GetFile().strPath.find_last_of('/');
    _strName = GetFile().strPath.substr((lp==std::string::npos ? 0 : lp+1));

    _bLoaded = 1;
}

void cSprBankAssetIMG::Unload()
{
    if( !_bLoaded ) return;
    ((cBankImageSet*)_hBank)->GetTextureAtlaser()->DeleteSprite(imgSprite);
    delete imgSprite;
    imgSprite = 0;
    _bLoaded = 0;
}

cSprBankAssetIMG::cSprBankAssetIMG(int it, cBankImageSet * par, cSprBankAsset * is, int id)
{
    _bLoaded = 0;
    _bForceReload = 0;
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

cSprBankAssetIMG::~cSprBankAssetIMG()
{
    Unload();
}

std::string cSprBankAssetIMG::GetMountPoint()
{
    char tmp[16];
    sprintf(tmp, "%d", m_iID);
    return std::string("/IMAGES/") + hIS->GetID() + "/" + tmp;
}

void cSprBankAsset::AddIMG(cSprBankAssetIMG * img)
{
    m_vSprites.push_back(img);
    SortAndFixIterators();
}

void cSprBankAsset::DeleteIMG(cSprBankAssetIMG * img)
{
    for(size_t i=0;i<m_vSprites.size();i++){
     if( m_vSprites[i] == img ){
      delete m_vSprites[i];
      m_vSprites.erase(m_vSprites.begin()+i);
      SortAndFixIterators();
      UpdateHash();
      return;
     }
    }
}

void cSprBankAsset::SortAndFixIterators()
{
    m_iMaxID = -1;
    sort(m_vSprites.begin(), m_vSprites.end(), cSprBank_SortAssetImages);
    for(size_t i=0;i<m_vSprites.size();i++){
     m_vSprites[i]->m_iIT = i;
     if( m_vSprites[i]->m_iID > m_iMaxID )
      m_iMaxID = m_vSprites[i]->m_iID;
    }
    UpdateHash();
}

void cSprBankAsset::UpdateHash()
{
    std::string acchash;
    for(size_t i=0; i< m_vSprites.size(); i++)
     acchash.append(m_vSprites[i]->GetHash());
    hashwrapper * hasher = new md5wrapper();
    strHash = hasher->getHashFromString(acchash);
    delete hasher;
}

cSprBankAssetIMG * cSprBankAsset::GetIMGByID(int id)
{
    for(int i=0;i<m_vSprites.size();i++){
     if( id == m_vSprites[i]->GetID() )
      return m_vSprites[i];
    }
    return 0;
}

cSprBankAssetIMG * cSprBankAsset::GetIMGByIterator(int it)
{
    if( it >= m_vSprites.size() ){
     it = m_vSprites.size()-1;
    }
    return m_vSprites[it];
}

cSprBankAssetIMG * cSprBankAsset::GetIMGByNoID(int noid)
{
    for(int i=0;i<m_vSprites.size();i++)
     if( m_vSprites[i]->GetID() == noid )
      return m_vSprites[i];
    return m_vSprites[0];
}

hgeSprite * cBankImageSet::GetObjectSprite(WWD::Object * obj)
{
    if( strstr(obj->GetImageSet(), "FONT") != NULL )
     //return GetSpriteFromAsset(obj->GetImageSet(), std::max(obj->GetParam(WWD::Param_LocationI), 0));
     return GetSpriteFromAsset(obj->GetImageSet(), std::max(GetUserDataFromObj(obj)->GetI(), 0));
    else if( GV->editState->hTileset->GetSet(obj->GetImageSet()) != 0 ){
     cTile * t = GV->editState->hTileset->GetSet(obj->GetImageSet())->GetTile(GetUserDataFromObj(obj)->GetI());
     if( !t ) return GV->sprSmiley;
     t->GetImage()->SetHotSpot(t->GetImage()->GetWidth()/2, t->GetImage()->GetHeight()/2);
     return t->GetImage();
    }else{
     if( !strcmp(obj->GetLogic(), "GroundBlower") && !strcmp(obj->GetImageSet(), "LEVEL_BLOWING1") ){
      return GetSpriteFromAsset("LEVEL_BLOWING1", 4);
     }
     //return GetSpriteFromAsset(obj->GetImageSet(), std::max(obj->GetParam(WWD::Param_LocationI)-1, 0));
     if( !strcmp(obj->GetLogic(), "TigerGuard") && obj->GetParam(WWD::Param_Smarts) == 1 )
      return GetSpriteFromAsset("LEVEL_TIGERWHITE", GetUserDataFromObj(obj)->GetI());
     else if( !strcmp(obj->GetLogic(), "HermitCrab") && obj->GetUserValue(0) != 0 )
      return GetSpriteFromAsset("LEVEL_BOMBERCRAB", GetUserDataFromObj(obj)->GetI());
     else
      return GetSpriteFromAsset(obj->GetImageSet(), GetUserDataFromObj(obj)->GetI());
    }
}

hgeSprite * cBankImageSet::GetSpriteFromAsset(const char * asset, int frame)
{
    if( GV->bKijan )
     return GV->sprKijan;
    else if( GV->bNapo )
     return GV->sprNapo;
    else{
     cSprBankAsset * as = GetAssetByID(asset);
     if( as != NULL )
      return as->GetIMGByNoID(frame)->GetSprite();
    }
    return GV->sprSmiley;
}

WWD::Rect cBankImageSet::GetSpriteRenderRect(hgeSprite * spr)
{
    WWD::Rect ret;
    float hsx, hsy;
    spr->GetHotSpot(&hsx, &hsy);
    ret.x1 = -hsx;
    ret.y1 = -hsy;
    ret.x2 = spr->GetWidth()-hsx;
    ret.y2 = spr->GetHeight()-hsy;
    return ret;
}

WWD::Rect cBankImageSet::GetObjectRenderRect(WWD::Object * obj)
{
    hgeSprite * spr = GetObjectSprite(obj);
    WWD::Rect ret;
    float hsx, hsy;
    spr->GetHotSpot(&hsx, &hsy);
    ret.x2 = spr->GetWidth();
    ret.y2 = spr->GetHeight();
    float sprw = ret.x2/2, sprh = ret.y2/2;
    hsx -= sprw;
    hsy -= sprh;
    ret.x1 = (GetUserDataFromObj(obj)->GetX()-sprw-hsx);
    ret.y1 = (GetUserDataFromObj(obj)->GetY()-sprh-hsy);
    if( !strcmp(obj->GetLogic(), "BreakPlank") ){
     for(int z=0;z<obj->GetParam(WWD::Param_Width)-1;z++)
      ret.x2 += 64;
    }else if( !strcmp(obj->GetLogic(), "FrontStackedCrates") || !strcmp(obj->GetLogic(), "BackStackedCrates") ){
     int irepeatnum = 0;
     for(int z=0;z<2;z++){
      WWD::Rect tmprect = obj->GetUserRect(z);
      if( tmprect.x1 != 0 ) irepeatnum++;
      if( tmprect.y1 != 0 ) irepeatnum++;
      if( tmprect.x2 != 0 ) irepeatnum++;
      if( tmprect.y2 != 0 ) irepeatnum++;
     }
     if( !obj->GetParam(WWD::Param_Powerup) ) irepeatnum--;
     for(int z=0;z<irepeatnum;z++){
      ret.y1 -= 43;
      ret.y2 += 43;
     }
    }else if( !strcmp(obj->GetLogic(), "PunkRat") ){
     ret.x1 -= 29;
     ret.x2 += 44;
     ret.y2 += 46;
    }
    return ret;
}

std::string cBankImageSet::getElementAt(int i)
{
    if( i < 0 || i >= m_vAssets.size() ) return "";
    return m_vAssets[i]->GetID();
}

int cBankImageSet::getNumberOfElements()
{
    return m_vAssets.size();
}

void cBankImageSet::SortAssets()
{
    sort(m_vAssets.begin(), m_vAssets.end(), cSprBank_SortAssets);
}

void cBankImageSet::BatchProcessStart(cDataController * hDC)
{
    bBatchProcess = 1;
    GV->Console->Printf("Scanning image sets...");
    _ghProgressInfo.iGlobalProgress = 6;
    _ghProgressInfo.strGlobalCaption = "Scanning image sets...";
    _ghProgressInfo.iDetailedProgress = 0;
    _ghProgressInfo.iDetailedEnd = 100000;
    iBatchPackageCount = 0;
}

void cBankImageSet::BatchProcessEnd(cDataController * hDC)
{
    cParallelLoop * looper = hDC->GetLooper();
    _ghProgressInfo.strDetailedCaption = "Packing textures...";
    _ghProgressInfo.iDetailedProgress = 50000;
    _ghProgressInfo.iDetailedEnd = 100000;
    if( looper != 0 )
     looper->Tick();
    SortAssets();
    myAtlaser->Pack();
    int spritec = 0;
    for(size_t i=0;i<m_vAssets.size();i++)
     spritec += m_vAssets[i]->m_vSprites.size();
    GV->Console->Printf("%d images in %d image sets found. Loading...", spritec, m_vAssets.size());

    int progress = 0;
    for(size_t i=0;i<m_vAssets.size();i++){
     bool bAffected = 0;
     for(size_t x=0;x<m_vAssets[i]->m_vSprites.size();x++){
      if( m_vAssets[i]->m_vSprites[x]->IsLoaded() )
       continue;
      char buf[256];
      sprintf(buf, "Rendering: %s [%d/%d]", m_vAssets[i]->GetID(), x, m_vAssets[i]->m_vSprites.size());
      _ghProgressInfo.strDetailedCaption = buf;
      _ghProgressInfo.iDetailedProgress = 50000+(float(progress)/float(spritec))*50000.0f;
      if( looper != 0 )
       looper->Tick();
      m_vAssets[i]->m_vSprites[x]->Load();
      progress++;
      bAffected = 1;
     }
     if( bAffected )
      m_vAssets[i]->UpdateHash();
    }
    bBatchProcess = 0;
}
/*
void cBankImageSet::ProcessAssets(cAssetPackage * hClientAP, std::vector<cFile> vFiles)
{
    float progperproces = 50000.0f/float(hClientAP->GetParent()->GetPackages().size());
    for(size_t i=0;i<vFiles.size();i++){
     int cut = GetFolderName().length()+1;
     if( hClientAP->GetPath().length() > 0 )
      cut += hClientAP->GetPath().length()+1;
     std::string assetid = vFiles[i].strPath.substr(cut);
     _ghProgressInfo.iDetailedProgress = iBatchPackageCount*progperproces+(progperproces)*(float(i)/float(vFiles.size()));
     _ghProgressInfo.strDetailedCaption = "File: " + assetid;
     if( hClientAP->GetParent()->GetLooper() != 0 )
      hClientAP->GetParent()->GetLooper()->Tick();
     std::transform(vFiles[i].strPath.begin(), vFiles[i].strPath.end(), vFiles[i].strPath.begin(), ::tolower);
     cImageInfo inf;
     //printf("feed 0x%p path %s\n", vFiles[i].hFeed, vFiles[i].strPath.c_str());
     if( hClientAP->GetParent()->IsLoadableImage(vFiles[i], &inf, cImageInfo::Full) ){
      size_t lp = assetid.rfind('/');
      if( lp != std::string::npos ){
       assetid = assetid.substr(0, lp);
       assetid = hClientAP->GetPrefix() + "_" + assetid;
      }else{
       assetid = hClientAP->GetPrefix();
      }
      while(1){
       size_t p = assetid.find_first_of("/\\");
       if( p == std::string::npos ) break;
       assetid[p] = '_';
      }
      std::transform(assetid.begin(), assetid.end(), assetid.begin(), ::toupper);
      cSprBankAsset * as = GetAssetByID(assetid.c_str());
      if( !as ){
       as = new cSprBankAsset(assetid);
       m_vAssets.push_back(as);
      }
      cSprBankAssetIMG * img = new cSprBankAssetIMG(vFiles[i], inf, as->m_vSprites.size(), this, as);
      as->m_vSprites.push_back(img);
      hClientAP->RegisterAsset(img);
     }else{

     }
     //printf("    %s\n", vFiles[i].strPath.c_str());
    }
    if( !bBatchProcess ){
     SortAssets();
     myAtlaser->Pack();
     for(size_t i=0;i<m_vAssets.size();i++){
      for(size_t x=0;x<m_vAssets[i]->m_vSprites.size();x++)
       m_vAssets[i]->m_vSprites[x]->Load();
     }
    }
    iBatchPackageCount++;
}*/

std::string cBankImageSet::GetMountPointForFile(std::string strFilePath, std::string strPrefix)
{
    size_t lslash = strFilePath.rfind('/');
    if(  lslash == strFilePath.length()-1 )
     return "";
    std::string strSet = (lslash == std::string::npos ? strPrefix
                                                     : strPrefix + "_" + strFilePath.substr(0, lslash)),
                strFile = (lslash == std::string::npos ? strFilePath
                                                       : strFilePath.substr(lslash));
    size_t filedot = strFile.rfind('.');
    if( filedot == std::string::npos || filedot == 0 || filedot == strFile.length()-1 )
     return "";
    std::string strExtension = strFile.substr(filedot+1);
    std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), ::tolower);
    if( strExtension != "pid" && strExtension != "bmp" && strExtension != "pcx" )
     return "";
    std::transform(strSet.begin(), strSet.end(), strSet.begin(), ::toupper);
    size_t slash = strSet.find('/');
    while( slash != std::string::npos ){
     strSet[slash] = '_';
     slash = strSet.find('/');
    }

    size_t numpos = strFile.find_first_of("1234567890");
    int tid = 0;
    if( numpos != std::string::npos ){
     while(1){
      if( numpos >= strFile.length() || strFile[numpos] < 48 || strFile[numpos] > 57 ) break;
      tid *= 10;
      tid += int(strFile[numpos])-48;
      numpos++;
     }
    }
    char buf[16];
    sprintf(buf, "%d", tid);

    return "/IMAGES/" + strSet + "/" + buf;
}

cAsset * cBankImageSet::AllocateAssetForMountPoint(cDataController * hDC, cDC_MountEntry mountEntry)
{
    size_t lslash = mountEntry.strMountPoint.rfind('/');
    std::string strID = mountEntry.strMountPoint.substr(8, lslash-8),
                strFrame = mountEntry.strMountPoint.substr(lslash+1);
    int iFrame = atoi(strFrame.c_str());

    cSprBankAsset * as = GetAssetByID(strID.c_str());
    if( !as ){
     as = new cSprBankAsset(strID);
     m_vAssets.push_back(as);
    }
    cSprBankAssetIMG * img = new cSprBankAssetIMG(as->m_vSprites.size(), this, as, iFrame);
    as->AddIMG(img);
    return img;
}

void cBankImageSet::DeleteAsset(cAsset * hAsset)
{
    std::string strMount = hAsset->GetMountPoint();
    strMount = strMount.substr(8);
    size_t pos = strMount.find('/');
    if( pos == std::string::npos ) return;
    std::string strSet = strMount.substr(0, pos),
                strID = strMount.substr(pos+1);
    int iID = atoi(strID.c_str());
    cSprBankAsset * set = GetAssetByID(strSet.c_str());
    if( !set ) return;
    cSprBankAssetIMG * img = set->GetIMGByID(iID);
    if( !img ) return;
    set->DeleteIMG(img);
    if( set->GetSpritesCount() == 0 ){
     for(size_t i=0;i<m_vAssets.size();i++)
      if( m_vAssets[i] == set ){
       delete m_vAssets[i];
       m_vAssets.erase(m_vAssets.begin()+i);
       return;
      }
    }
}
