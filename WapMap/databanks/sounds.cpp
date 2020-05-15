#include "sounds.h"
#include "../../shared/commonFunc.h"
#include "../globals.h"
#include "../cParallelLoop.h"
#include "../../shared/cProgressInfo.h"

extern structProgressInfo _ghProgressInfo;
extern HGE *hge;

bool cSndBank_SortAssets(cSndBankAsset *a, cSndBankAsset *b) {
    return (std::string(a->GetName()) < std::string(b->GetName()));
}

cBankSound::cBankSound(WWD::Parser *hParser) : cAssetBank(hParser) {
    bBatchProcessing = 0;
}

cBankSound::~cBankSound() {
    for (int i = 0; i < m_vAssets.size(); i++) {
        delete m_vAssets[i];
        m_vAssets[i] = 0;
    }
}

/*void cSndBank::LoadDiscRecursive(DIR * dir, const char * pszPrefix)
{
	char cwd[MAX_PATH];
	::getcwd(cwd, MAX_PATH);
	struct stat info;
	struct dirent *dirp;
	while( (dirp = readdir(dir)) != NULL ){
	 if( dirp->d_name[0] == '.' ) continue;
	 stat(dirp->d_name, &info);
	 if( S_ISDIR(info.st_mode) ){
	  DIR * dp = opendir(dirp->d_name);
	  char * prefix = new char[strlen(pszPrefix)+strlen(dirp->d_name)+2];
	  sprintf(prefix, "%s_%s", pszPrefix, dirp->d_name);
	  _chdir(dirp->d_name);
	  LoadDiscRecursive(dp, prefix);
	  _chdir("..");
	  closedir(dp);
	  delete [] prefix;
	 }else{
	  char * ext = SHR::GetExtension(dirp->d_name);
	  char * extl = SHR::ToLower(ext);
	  if( !strcmp(extl, "wav") ){
	   char * filename = SHR::GetFileWithoutExt(dirp->d_name);
	   cSndBankAsset * as = new cSndBankAsset();
	   as->m_szID = new char[strlen(filename)+strlen(pszPrefix)+2];
	   sprintf(as->m_szID, "%s_%s", pszPrefix, filename);
	   char path[strlen(cwd)+strlen(dirp->d_name)+2];
	   sprintf(path, "%s\\%s", cwd, dirp->d_name);
	   as->m_snd = hge->Effect_Load(path);
	   m_vAssets.push_back(as);
	   delete [] filename;
	  }
	  delete [] extl;
	  delete [] ext;
	 }
	}
}*/

cSndBankAsset *cBankSound::GetAssetByID(const char *pszID) {
    for (int i = 0; i < m_vAssets.size(); i++) {
        if (!strcmp(m_vAssets[i]->GetName(), pszID)) {
            return m_vAssets[i];
        }
    }
    return NULL;
}

cSndBankAsset::cSndBankAsset(cFile hFile, std::string id) {
    SetFile(hFile);
    _strName = id;
}

std::string cSndBankAsset::GetMountPoint() {
    return std::string("/SND/") + _strName;
}

cSndBankAsset::~cSndBankAsset() {
    Unload();
}

void cSndBankAsset::Load() {
    if (_bLoaded) return;
    unsigned int len;
    unsigned char *ptr = GetFile().hFeed->GetFileContent(GetFile().strPath.c_str(), len);
    if (len == 0) return;
    m_snd = hge->Effect_Load((const char *) ptr, len);
    delete[] ptr;
    _bLoaded = (m_snd != 0);
}

void cSndBankAsset::Unload() {
    hge->Effect_Free(m_snd);
    _bLoaded = 0;
}

std::string cBankSound::getElementAt(int i) {
    if (i < 0 || i >= m_vAssets.size()) return "";
    return m_vAssets[i]->GetName();
}

int cBankSound::getNumberOfElements() {
    return m_vAssets.size();
}

void cBankSound::SortAssets() {
    sort(m_vAssets.begin(), m_vAssets.end(), cSndBank_SortAssets);
}

void cBankSound::BatchProcessStart(cDataController *hDC) {
    bBatchProcessing = 1;
    GV->Console->Printf("Loading sounds...");
    _ghProgressInfo.iGlobalProgress = 7;
    _ghProgressInfo.strGlobalCaption = "Loading sounds...";
    _ghProgressInfo.strDetailedCaption = "Scanning sounds...";
    _ghProgressInfo.iDetailedProgress = 0;
    _ghProgressInfo.iDetailedEnd = 100000;
    iBatchPackageCount = 0;
}

void cBankSound::BatchProcessEnd(cDataController *hDC) {
    _ghProgressInfo.iDetailedProgress = 50000;
    _ghProgressInfo.iDetailedEnd = 100000;
    _ghProgressInfo.strDetailedCaption = "Sorting...";
    SortAssets();
    if (hDC->GetLooper() != 0)
        hDC->GetLooper()->Tick();
    for (size_t i = 0; i < m_vAssets.size(); i++) {
        char buf[256];
        sprintf(buf, "Loading: %s [%d/%d]", m_vAssets[i]->GetName(), i, m_vAssets.size());
        _ghProgressInfo.strDetailedCaption = buf;
        _ghProgressInfo.iDetailedProgress = 50000 + (float(i) / float(m_vAssets.size())) * 50000.0f;
        if (hDC->GetLooper() != 0)
            hDC->GetLooper()->Tick();
        m_vAssets[i]->Load();
    }
    bBatchProcessing = 0;
}

void cBankSound::ProcessAssets(cAssetPackage *hClientAP, std::vector<cFile> vFiles) {
    float progperproces = 50000.0f / float(hClientAP->GetParent()->GetPackages().size());
    for (int i = 0; i < vFiles.size(); i++) {
        int cut = GetFolderName().length() + 1;
        if (hClientAP->GetPath().length() > 0)
            cut += hClientAP->GetPath().length() + 1;
        std::string assetid = vFiles[i].strPath.substr(cut);
        _ghProgressInfo.iDetailedProgress =
                iBatchPackageCount * progperproces + (progperproces) * (float(i) / float(vFiles.size()));
        _ghProgressInfo.strDetailedCaption = "File: " + assetid;
        if (hClientAP->GetParent()->GetLooper() != 0)
            hClientAP->GetParent()->GetLooper()->Tick();

        std::transform(vFiles[i].strPath.begin(), vFiles[i].strPath.end(), vFiles[i].strPath.begin(), ::tolower);
        size_t lastdot = vFiles[i].strPath.rfind('.');
        if (lastdot == std::string::npos || lastdot == vFiles[i].strPath.length() - 1) continue;
        std::string ext = vFiles[i].strPath.substr(lastdot + 1);
        if (ext.compare("wav") == 0) {
            assetid = hClientAP->GetPrefix() + "_" + vFiles[i].strPath.substr(cut, lastdot - cut);
            while (1) {
                size_t p = assetid.find_first_of("/\\");
                if (p == std::string::npos) break;
                assetid[p] = '_';
            }
            std::transform(assetid.begin(), assetid.end(), assetid.begin(), ::toupper);
            cSndBankAsset *as = new cSndBankAsset(vFiles[i], assetid);
            m_vAssets.push_back(as);
            hClientAP->RegisterAsset(as);
        }
    }
    if (!bBatchProcessing) {
        SortAssets();
        for (size_t i = 0; i < m_vAssets.size(); i++) {
            m_vAssets[i]->Load();
        }
    } else
        iBatchPackageCount++;
}
