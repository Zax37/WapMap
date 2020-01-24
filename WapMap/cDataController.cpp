#include "../shared/commonFunc.h"
#include "cDataController.h"
#include "../shared/cPID.h"
#include "globals.h"
#include "../shared/HashLib/hashlibpp.h"
#include "cParallelLoop.h"
#include <direct.h>

extern HGE * hge;
cDataController * _ghDataController = 0;

bool _cDC_SortMountEntry(cFile a, cFile b)
{
	int pria = _ghDataController->GetFeedPriority(a.hFeed),
		prib = _ghDataController->GetFeedPriority(b.hFeed);
	if (pria != prib)
		return pria > prib;
	return a.strPath < b.strPath;
}

bool _cDC_SortMountEntries(cDC_MountEntry a, cDC_MountEntry b)
{
	return (a.strMountPoint < b.strMountPoint);
}

cDataController::cDataController(std::string strCD, std::string strFD, std::string strFN)
{
	hLooper = 0;
	strClawDir = strCD;
	strFileDir = strFD;
	strFilename = strFN;

	fFeedRefreshTime = hge->Timer_GetTime();

	size_t pos = strFileDir.find('\\');
	while (pos != std::string::npos) {
		strFileDir[pos] = '/';
		pos = strFileDir.find('\\');
	}

	//printf("filedir '%s' filename '%s'\n", strFD.c_str(), strFN.c_str());

	std::string tmp = strClawDir;
	tmp.append("/CLAW.REZ");
	FILE * f = fopen(tmp.c_str(), "rb");
	if (!f) {
		hREZ = 0;
		GV->Console->Printf("~r~Error mounting REZ archive (~y~%s~r~)!", tmp.c_str());
	}
	else {
		fclose(f);
		hREZ = new cRezFeed(tmp);
	}

	tmp = strClawDir;
	tmp.append("/Assets");
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;
	hFind = FindFirstFile(tmp.c_str(), &fdata);
	if (hFind == INVALID_HANDLE_VALUE) {
		hDisc = 0;
		GV->Console->Printf("~r~Error mounting disc assets directory (~y~%s~r~)!", tmp.c_str());
	}
	else {
		hDisc = new cDiscFeed(tmp);
	}

	hCustom = 0;
	if (strFileDir.length() > 0) {
		tmp = strFileDir;
		char * tmp2 = SHR::GetFileWithoutExt(strFilename.c_str());
		tmp.append("/");
		tmp.append(std::string(tmp2));
		delete[] tmp2;
		hFind = FindFirstFile(tmp.c_str(), &fdata);
		if (hFind == INVALID_HANDLE_VALUE) {
			GV->Console->Printf("~r~Error mounting custom assets directory (~y~%s~r~)!", tmp.c_str());
		}
		else {
			hCustom = new cDiscFeed(tmp);
		}
	}
	else {
		GV->Console->Printf("~r~Unable to mount custom assets directory due to virtual document.");
	}

	hPalette = new PID::Palette();
}

cDataController::~cDataController()
{
	while (!vhPackages.empty()) {
		delete vhPackages[0];
		vhPackages.erase(vhPackages.begin());
	}
	delete hCustom;
	delete hDisc;
	delete hREZ;
}

cFileFeed * cDataController::GetFeed(int i)
{
	if (i == DB_FEED_REZ) {
		return hREZ;
	}
	else if (i == DB_FEED_DISC) {
		return hDisc;
	}
	else if (i == DB_FEED_CUSTOM) {
		return hCustom;
	}
	return 0;
}

cAsset::cAsset()
{
	_iLoadedDate = _iLastDate = _iFileSize = 0;
	hParent = 0;
}

cAsset::~cAsset()
{

}

void cAsset::Reload()
{
	Unload();
	Load();
}

void cAsset::SetFile(cFile nFile)
{
	_hFile = nFile;
	if (_hFile.hFeed == 0) {
		_iFileSize = 0;
		_strHash = "";
		return;
	}
	_iFileSize = _hFile.hFeed->GetFileSize(_hFile.strPath.c_str());
	hashwrapper * hasher = new md5wrapper();
	unsigned int len;
	unsigned char * ptr = GetFile().hFeed->GetFileContent(GetFile().strPath.c_str(), len);
	_strHash = hasher->getHashFromData(ptr, len);
	delete[] ptr;
	delete hasher;
	_iLoadedDate = _hFile.hFeed->GetFileModTime(_hFile.strPath.c_str());
}

cAssetPackage * cDataController::CreatePackage(std::string strPath, std::string strPref, int iLoadPolicy)
{
	cAssetPackage * ap = new cAssetPackage(this);
	ap->strPrefix = strPref;
	ap->iLoadPolicy = iLoadPolicy;
	ap->hParent = this;
	ap->strPath = strPath;
	//ap->Update();
	vhPackages.push_back(ap);
	return ap;
}

void cDataController::DeletePackage(cAssetPackage * ptr)
{
	for (int i = 0; i < vhPackages.size(); i++)
		if (vhPackages[i] == ptr) {
			vhPackages.erase(vhPackages.begin() + i);
			break;
		}
	delete ptr;
}

bool cDataController::SetPalette(std::string strPath)
{
	unsigned char * data;
	unsigned int len;
	cFileFeed * feed = 0;
	if (hDisc != 0 && hDisc->FileExists(strPath.c_str())) {
		feed = hDisc;
	}
	else {
		if (hREZ == 0)
			return 0;
		bool ex = hREZ->FileExists(strPath.c_str());
		if (!ex) return 0;
		feed = hREZ;
	}
	data = feed->GetFileContent(strPath.c_str(), len);
	if (len != 768) {
		delete[] data;
		return 0;
	}
	if (hPalette != 0)
		delete hPalette;
	hPalette = new PID::Palette(data, len);
	delete[] data;
	return 1;
}

PID::Palette * cAssetPackage::GetWorkingPalette()
{
	return hParent->GetPalette();
};

void cDataController::RegisterAssetBank(cAssetBank * hPtr)
{
	vhBanks.push_back(hPtr);
	hPtr->_bModFlag = 0;
	hPtr->_iModNew = hPtr->_iModChange = hPtr->_iModDel = 0;
}

std::string cDataController::FilePathToIdentifier(std::string strPath)
{
	std::transform(strPath.begin(), strPath.end(), strPath.begin(), ::toupper);
	char * tmp = SHR::Replace(strPath.c_str(), "/", "_");
	char * tmp2 = SHR::Replace(tmp, "/", "_");
	strPath = std::string(tmp2);
	delete[] tmp2;
	delete[] tmp;

	return strPath;
}

byte * cDataController::GetImageRaw(cFile hFile, int * pW, int * pH)
{
	byte * ret = 0;
	size_t dot = hFile.strPath.rfind('.');
	if (dot == std::string::npos) return 0;
	std::string strExt = hFile.strPath.substr(dot);
	std::transform(strExt.begin(), strExt.end(), strExt.begin(), ::tolower);

	unsigned int len;
	unsigned char * ptr = hFile.hFeed->GetFileContent(hFile.strPath.c_str(), len);
	if (!ptr || len == 0) return 0;
	if (ptr[0] == 'B' && ptr[1] == 'M') {
		int pixeldata = *(int*)(ptr + 10),
			headerlen = *(int*)(ptr + 14),
			width = *(int*)(ptr + 18),
			height = *(int*)(ptr + 22);
		int compressiontype = 0;
		if (headerlen > 12) compressiontype = (*(int*)(ptr + 30));
		int padding = 4 - (width % 4);
		if (padding == 4) padding = 0;

		ret = new byte[width*height];
		*pW = width;
		*pH = height;

		int srcpos = 0;
		unsigned char * pixelptr = ptr + pixeldata;
		for (int y = 1; y <= height; y++) {
			for (int x = 0; x < width; x++) {
				ret[(height - y)*width + x] = pixelptr[srcpos];
				srcpos++;
			}
			srcpos += padding;
		}
	}
	else if (strExt.compare(".pcx") == 0) {
		unsigned char * pixelptr = ptr + 128,
			bitsPerPixel = *(unsigned char*)(ptr + 3),
			colorPlanes = *(unsigned char*)(ptr + 65);

		if (bitsPerPixel != 8 || colorPlanes != 1) {
			delete[] ptr;
			return ret; // unsupported
		}

		unsigned short xMin = *(unsigned short*)(ptr + 4),
			yMin = *(unsigned short*)(ptr + 6),
			xMax = *(unsigned short*)(ptr + 8),
			yMax = *(unsigned short*)(ptr + 10);

		int w = xMax - xMin + 1, h = yMax - yMin + 1;

		ret = new byte[w*h];
		*pW = w;
		*pH = h;

		int destpos = 0;
		while (1) {
			if (destpos >= w * h) break;
			unsigned char value = *pixelptr;
			unsigned char repeat = 1;
			if (value >= 192) {
				repeat = value - 192;
				value = *(pixelptr + 1);
				pixelptr++;
			}
			for (int i = 0; i < repeat; i++) {
				ret[(destpos / w)*w + (destpos%w) + i] = 255 - value;
			}
			pixelptr++;
			destpos += repeat;
		}
	}
	else if (strExt.compare(".pid") == 0) {
		PID::Image * pid = new PID::Image(ptr, len, GetPalette(), 0, 0, hge);
		ret = new byte[pid->GetWidth()*pid->GetHeight()];
		*pW = pid->GetWidth();
		*pH = pid->GetHeight();
		for (int y = 0; y < pid->GetHeight(); y++)
			for (int x = 0; x < pid->GetWidth(); x++)
				ret[y*pid->GetWidth() + x] = pid->GetColorIdAt(x, y);
		delete pid;
	}
	delete[] ptr;
	return ret;
}

bool cDataController::RenderImageRaw(byte * hData, HTEXTURE texDest, int iRx, int iRy, int iRowSpan, int w, int h, PID::Palette * pal)
{
	DWORD * td = hge->Texture_Lock(texDest, 0, iRx, iRy, w, h);
	if (!td)
		return 0;
	PID::Palette * palptr = (pal == 0 ? hPalette
		: pal);
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) {
			td[y*iRowSpan + x] = (hData[y*w + x] == 0 ? 0x00FFFFFF : palptr->GetColor(hData[y*w + x]));
		}
	hge->Texture_Unlock(texDest);
	return 1;
}

bool cDataController::IsLoadableImage(cFile hFile, cImageInfo * inf, cImageInfo::Level iInfoLevel)
{
	std::string strFileName;
	size_t cutpos = hFile.strPath.rfind('/');
	strFileName = (cutpos == std::string::npos ? hFile.strPath
		: hFile.strPath.substr(cutpos));
	size_t dot = strFileName.rfind('.');
	if (dot == std::string::npos) return 0;
	std::string strExt = strFileName.substr(dot);
	std::transform(strExt.begin(), strExt.end(), strExt.begin(), ::tolower);
	if (strExt.compare(".pid") == 0) {
		if (inf != 0) {
			unsigned int l;
			unsigned char * buf = hFile.hFeed->GetFileContent(hFile.strPath.c_str(), l, 0, 32);
			if (buf == 0 || l == 0) {
				//printf("---DataCtrl error checking loadable image %s.\n", hFile.strPath.c_str());
				return 0;
			}
			PID::HeaderInfo pidinf(buf);
			delete[] buf;
			inf->iWidth = pidinf.GetWidth();
			inf->iHeight = pidinf.GetHeight();
			inf->iFlags = pidinf.GetFlags();
			inf->iOffsetX = pidinf.GetOffsetX();
			inf->iOffsetY = pidinf.GetOffsetY();
			inf->iUser1 = pidinf.GetUserValue(2);
			inf->iUser2 = pidinf.GetUserValue(3);
			inf->iType = cImageInfo::PID;
		}
		return 1;
	}
	else if (strExt.compare(".bmp") == 0) {
		unsigned int l;
		unsigned char * buf = hFile.hFeed->GetFileContent(hFile.strPath.c_str(), l, 0, 34);
		int headerlen = *(int*)(buf + 14);
		short colordepth = (headerlen == 12 ? *(short*)(buf + 24)
			: *(short*)(buf + 28));
		int compression = 0;
		if (headerlen > 12) compression = (*(int*)(buf + 30));
		if (buf == 0 || l != 34 || buf[0] != 'B' || buf[1] != 'M' || colordepth != 8 || compression != 0) {
			delete[] buf;
			return 0;
		}
		if (inf != 0) {
			inf->iWidth = *(int*)(buf + 18);
			inf->iHeight = *(int*)(buf + 22);
			inf->iFlags = (PID::FLAGS)0;
			inf->iOffsetX = inf->iOffsetY = 0;
			inf->iUser1 = inf->iUser2 = 0;
			inf->iType = cImageInfo::BMP;
		}
		delete[] buf;
		return 1;
	}
	else if (strExt.compare(".pcx") == 0) {
		unsigned int l;
		unsigned char * buf = hFile.hFeed->GetFileContent(hFile.strPath.c_str(), l, 0, 16);
		if (!buf || l != 16 || buf[0] != 10 || buf[2] != 1 || buf[3] != 8) {
			delete[] buf;
			return 0;
		}

		if (inf != 0) {
			unsigned short xMin = *(unsigned short*)(buf + 4),
				yMin = *(unsigned short*)(buf + 6),
				xMax = *(unsigned short*)(buf + 8),
				yMax = *(unsigned short*)(buf + 10);
			inf->iWidth = xMax - xMin + 1;
			inf->iHeight = yMax - yMin + 1;
			inf->iFlags = (PID::FLAGS)0;
			inf->iOffsetX = inf->iOffsetY = 0;
			inf->iUser1 = inf->iUser2 = 0;
			inf->iType = cImageInfo::PCX;
		}
		delete[] buf;
		return 1;
	}
	return 0;
}

bool cDataController::RenderImage(cFile hFile, HTEXTURE texDest, int iRx, int iRy, int iRowSpan)
{
	int w, h;
	byte * data = GetImageRaw(hFile, &w, &h);
	if (!data) return 0;
	bool ret = RenderImageRaw(data, texDest, iRx, iRy, iRowSpan, w, h);
	delete[] data;
	return ret;
}

void cDataController::RelocateDocument(std::string strDocPath)
{
	char * htmp = SHR::GetFileWithoutExt(strFilename.c_str());
	std::string strOldCustomPath = strFileDir + "/" + htmp;
	delete[] htmp;
	htmp = SHR::GetDir(strDocPath.c_str());
	strFileDir = std::string(htmp);
	delete[] htmp;
	htmp = SHR::GetFile(strDocPath.c_str());
	strFilename = std::string(htmp);
	delete[] htmp;
	htmp = SHR::GetFileWithoutExt(strFilename.c_str());
	std::string strNewDiscDir = strFileDir + "/" + htmp;
	delete[] htmp;
	GV->Console->Printf("Relocating document assets to '~y~%s~w~'.", strNewDiscDir.c_str());

	mkdir(strNewDiscDir.c_str());
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;
	hFind = FindFirstFile(strNewDiscDir.c_str(), &fdata);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (hCustom != 0) {
			delete hCustom;
			hCustom = 0;
		}
		GV->Console->Printf("~r~Error mounting custom assets directory!");
	}
	else {
		if (hCustom != 0) {
			GV->Console->Printf("Copying custom assets to new directory.");
			SHR::CopyDirR(strOldCustomPath.c_str(), strNewDiscDir.c_str());
			hCustom->Remount(strNewDiscDir);
		}
		else {
			hCustom = new cDiscFeed(strNewDiscDir);
		}
		GV->Console->Printf("~g~Remounted custom assets directory properly.");
	}
}

void cDataController::FixCustomDir()
{
	if (hCustom != 0) return;
	char * htmp = SHR::GetFileWithoutExt(strFilename.c_str());
	std::string strCustomPath = strFileDir + "/" + htmp;
	delete[] htmp;
	mkdir(strCustomPath.c_str());
	GV->Console->Printf("Fixing custom directory: '%s'.", strCustomPath.c_str());
	std::string tmp = strCustomPath + "/" + "LOGICS";
	mkdir(tmp.c_str());
	tmp = strCustomPath + "/" + "IMAGES";
	mkdir(tmp.c_str());
	tmp = strCustomPath + "/" + "SOUNDS";
	mkdir(tmp.c_str());
	tmp = strCustomPath + "/" + "TILES";
	mkdir(tmp.c_str());
	hCustom = new cDiscFeed(strCustomPath);
}

std::vector<cFile> cDataController::GetFilesList(std::string strPath, int iLoadPolicy)
{
	cFileFeed * sourcefeed;
	std::vector<cFile> vR;
	sourcefeed = (iLoadPolicy == cDC_STANDARD ? hDisc : hCustom);
	std::vector<std::string> vFiles;
	if (sourcefeed != 0) {
		vFiles = sourcefeed->GetDirectoryContents(strPath.c_str(), 1);
		while (!vFiles.empty()) {
			cFile n;
			n.hFeed = sourcefeed;
			n.strPath = vFiles.front();
			vFiles.erase(vFiles.begin());
			vR.push_back(n);
		}
	}

	if (iLoadPolicy == cDC_STANDARD && hREZ != 0) {
		vFiles = hREZ->GetDirectoryContents(strPath.c_str(), 1);
		while (!vFiles.empty()) {
			/*std::string lcase = vFiles.front();
			std::transform(lcase.begin(), lcase.end(), lcase.begin(), ::tolower);
			bool bFound = 0;
			for(size_t i=0;i<vR.size();i++){
			 std::string lcase2 = vR[i].strPath;
			 std::transform(lcase2.begin(), lcase2.end(), lcase2.begin(), ::tolower);
			 if( lcase.compare(lcase2) == 0 ){
			  bFound = 1;
			  break;
			 }
			}
			if( bFound ){
			 vFiles.erase(vFiles.begin());
			}else{*/
			cFile n;
			n.hFeed = hREZ;
			n.strPath = vFiles.front();
			vR.push_back(n);
			vFiles.erase(vFiles.begin());
			//}
		}
	}
	return vR;
}

cAssetPackage::cAssetPackage(cDataController * parent)
{
	hParent = parent;
}

cAssetPackage::~cAssetPackage()
{
	/*for(int x=0;x<vSets.size();x++){
	 for(int y=0;y<vSets[x].vAssets.size();y++){
	  delete vSets[x].vAssets[y];
	 }
	}*/
}

void cDataController::UpdateAllPackages()
{
	for (size_t i = 0; i < vhBanks.size(); i++) {
		vhBanks[i]->BatchProcessStart(this);
		if (GetLooper() != 0)
			GetLooper()->Tick();
		for (size_t y = 0; y < vhPackages.size(); y++) {
			vhPackages[y]->Update(vhBanks[i]);
		}
		for (size_t z = 0; z < vMountEntries.size(); z++) {
			std::string folderName = vhBanks[i]->GetFolderName();
			std::string mountPoint = vMountEntries[z].strMountPoint;
			if (vhBanks[i]->GetFolderName().compare(0, std::string::npos, vMountEntries[z].strMountPoint, 1, vhBanks[i]->GetFolderName().length()) == 0 &&
				vMountEntries[z].hAsset == 0) {
				cAsset * as = vhBanks[i]->AllocateAssetForMountPoint(this, vMountEntries[z]);
				if (!as)
					GV->Console->Printf("~r~Error loading asset for mount point ~y~%s~r~!", vMountEntries[z].strMountPoint.c_str());
				else {
					GetAssetPackageByFile(as->GetFile())->RegisterAsset(as);
					vMountEntries[z].hAsset = as;
				}
			}
		}
		vhBanks[i]->BatchProcessEnd(this);
		if (GetLooper() != 0)
			GetLooper()->Tick();
	}
}

cAssetPackage * cDataController::GetAssetPackageByFile(cFile hFile)
{
	if (hFile.hFeed == hCustom) {
		for (size_t i = 0; i < vhPackages.size(); i++)
			if (vhPackages[i]->GetLoadPolicy() == cDC_CUSTOM)
				return vhPackages[i];
	}

	std::string fdir = hFile.strPath.substr(0, hFile.strPath.find('/'));
	std::transform(fdir.begin(), fdir.end(), fdir.begin(), ::toupper);
	for (size_t i = 0; i < vhPackages.size(); i++) {
		if (fdir.compare(vhPackages[i]->GetPath()) == 0)
			return vhPackages[i];
	}
	return 0;
}

void cAssetPackage::Update(cAssetBank * hBank)
{
	if (hBank == 0) {
		std::vector<cAssetBank*> vBanks = hParent->GetBanks();
		for (int i = 0; i < vBanks.size(); i++) {
			Update(vBanks[i]);
		}
		return;
	}

	std::string strSetPath("");
	if (strPath.length() > 0) {
		strSetPath = strPath;
		strSetPath.append("/");
		strSetPath.append(hBank->GetFolderName());
	}
	else
		strSetPath = hBank->GetFolderName();

	//std::transform(strSetPath.begin(), strSetPath.end(), strSetPath.begin(), ::tolower);

	if (GetParent()->GetLooper() != 0)
		GetParent()->GetLooper()->Tick();
	std::vector<cFile> vFiles = hParent->GetFilesList(strSetPath, iLoadPolicy);
	if (vFiles.size() == 0) return;
	for (size_t i = 0; i < vFiles.size(); i++) {
		std::string strmntpath = hBank->GetMountPointForFile(vFiles[i].strPath, GetPrefix());
		if (!strmntpath.empty()) {
			std::string n = strSetPath;
			n.append("/");
			n.append(vFiles[i].strPath);
			vFiles[i].strPath = n;
			hParent->MountFile(strmntpath, vFiles[i]);
		}
	}
}

void cAssetPackage::RegisterAsset(cAsset * hPtr)
{
	hPtr->SetPackage(this);
}

void cDataController::UnmountFile(std::string strMountPoint, cFile hFile)
{
	for (size_t i = 0; i < vMountEntries.size(); i++)
		if (vMountEntries[i].strMountPoint.compare(strMountPoint) == 0) {
			for (size_t f = 0; f < vMountEntries[i].vFiles.size(); f++) {
				if (vMountEntries[i].vFiles[f].hFeed == hFile.hFeed &&
					vMountEntries[i].vFiles[f].strPath == hFile.strPath) {
					vMountEntries[i].vFiles.erase(vMountEntries[i].vFiles.begin() + f);
					GV->Console->Printf("Unmounted %s in %s.", hFile.strPath.c_str(), strMountPoint.c_str());
					if (vMountEntries[i].vFiles.empty()) {
						if (vMountEntries[i].hAsset != 0) {
							cAssetPackage * ap = vMountEntries[i].hAsset->GetPackage();
							ap->UnregisterAsset(vMountEntries[i].hAsset);
						}
						vMountEntries.erase(vMountEntries.begin() + i);
					}
					return;
				}
			}
			return;
		}
}

bool cDataController::MountFile(std::string strMountPoint, cFile f)
{
	std::transform(strMountPoint.begin(), strMountPoint.end(), strMountPoint.begin(), ::toupper);
	for (size_t i = 0; i < vMountEntries.size(); i++)
		if (vMountEntries[i].strMountPoint.compare(strMountPoint) == 0) {
			for (size_t fi = 0; fi < vMountEntries[i].vFiles.size(); fi++)
				if (vMountEntries[i].vFiles[fi].hFeed == f.hFeed &&
					vMountEntries[i].vFiles[fi].strPath == f.strPath)
					return 0;

			vMountEntries[i].vFiles.push_back(f);
			GV->Console->Printf("%d files mounted @ %s", int(vMountEntries[i].vFiles.size()), strMountPoint.c_str());
			_SortMountEntry(i);
			/*for(int z=0;z<vMountEntries[i].vFiles.size();z++){
			 printf("%d", GetFeedPriority(vMountEntries[i].vFiles[z].hFeed));
			 printf(": %s\n", vMountEntries[i].vFiles[z].strPath.c_str());
			}*/
			return 0;
		}
	cDC_MountEntry n;
	n.vFiles.push_back(f);
	n.hAsset = 0;
	n.strMountPoint = strMountPoint;
	vMountEntries.push_back(n);
	GV->Console->Printf("New mount @ %s", strMountPoint.c_str());
	return 1;
}

void cDataController::_SortMountEntries()
{
	std::sort(vMountEntries.begin(), vMountEntries.end(), _cDC_SortMountEntries);
}

void cDataController::_SortMountEntry(size_t id)
{
	_ghDataController = this;
	std::sort(vMountEntries[id].vFiles.begin(), vMountEntries[id].vFiles.end(), _cDC_SortMountEntry);
	_ghDataController = 0;
}

void cAssetPackage::UnregisterAsset(cAsset * hPtr)
{
	cAssetBank * bank = hPtr->GetAssignedBank();
	bank->DeleteAsset(hPtr);
}

cFile cDataController::AssignFileForLogic(std::string strLogicName)
{
	cFile n;
	n.hFeed = hCustom;
	n.strPath = "logics/" + strLogicName + ".lua";
	return n;
}

void cDataController::ForceRefreshFeeds()
{
	fFeedRefreshTime = hge->Timer_GetTime() - 3;
}

int cDataController::GetFeedPriority(cFileFeed * hFeed)
{
	if (hCustom != 0 && hFeed == hCustom)
		return 3;
	if (hDisc != 0 && hFeed == hDisc)
		return 2;
	if (hREZ != 0 && hFeed == hREZ)
		return 1;
	return -1;
}

cDC_MountEntry cDataController::GetMountEntry(std::string strMountPoint)
{
	for (size_t i = 0; i < vMountEntries.size(); i++)
		if (strMountPoint.compare(vMountEntries[i].strMountPoint) == 0)
			return vMountEntries[i];
	cDC_MountEntry n;
	n.hAsset = 0;
	return n;
}

int cDataController::GetMountPointID(std::string strMountPoint)
{
	for (size_t i = 0; i < vMountEntries.size(); i++)
		if (strMountPoint.compare(vMountEntries[i].strMountPoint) == 0)
			return i;
	return -1;
}

void cDataController::Think()
{
	for (size_t i = 0; i < vhBanks.size(); i++) {
		vhBanks[i]->_bModFlag = 0;
	}
	std::vector<cFileFeed*> vFeeds;
	if (hge->Timer_GetTime() - fFeedRefreshTime > 2.5f) {
		if (hREZ)
			vFeeds.push_back(hREZ);
		if (!hCustom) {
			char * htmp = SHR::GetFileWithoutExt(strFilename.c_str());
			std::string strCustomPath = strFileDir + "/" + htmp;
			delete[] htmp;
			HANDLE hFind = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATA fdata;
			hFind = FindFirstFile(strCustomPath.c_str(), &fdata);
			if (hFind != INVALID_HANDLE_VALUE) {
				hCustom = new cDiscFeed(strCustomPath);
			}
		}
	}
	if (hDisc) vFeeds.push_back(hDisc);
	if (hCustom) vFeeds.push_back(hCustom);

	std::vector<cFile> vFiles[3];
	for (int i = vFeeds.size() - 1; i >= 0; i--) {
		vFeeds[i]->UpdateModificationFlag();
		if (vFeeds[i]->GetModificationFlag()) {
			printf("feed modified\n");
			if (vFeeds[i]->NeedFullReload()) {

			}
			else {
				for (int f = 0; f < 3; f++) {
					std::vector<std::string> vstr;
					if (f == 0) vstr = vFeeds[i]->GetNewFiles();
					else if (f == 1) vstr = vFeeds[i]->GetModifiedFiles();
					else if (f == 2) vstr = vFeeds[i]->GetDeletedFiles();
					while (!vstr.empty()) {
						cFile n;
						n.hFeed = vFeeds[i];
						n.strPath = vstr[0];
						vFiles[f].push_back(n);
						vstr.erase(vstr.begin());
					}
				}
			}
			vFeeds[i]->ResetModificationFlag();
		}
	}
	std::vector<std::string> *strMntPntToUpdate = new std::vector<std::string>[vhBanks.size()];
	int *iBankNew = new int[vhBanks.size()], *iBankChange = new int[vhBanks.size()], *iBankDel = new int[vhBanks.size()];
	for (size_t i = 0; i < vhBanks.size(); i++)
		iBankNew[i] = iBankChange[i] = iBankDel[i] = 0;

	bool bAddedMntPnts = 0;
	for (int f = 0; f < 3; f++) {
		for (size_t i = 0; i < vFiles[f].size(); i++) {
			cAssetPackage * ap = GetAssetPackageByFile(vFiles[f].at(i));
			if (ap) {
				std::string strnfp = vFiles[f].at(i).strPath.substr((ap->GetPath().length() > 0 ? ap->GetPath().length() + 1 : 0));
				size_t slashpos = strnfp.find('/');
				if (slashpos == std::string::npos)
					continue;
				std::string bankidstr = strnfp.substr(0, slashpos);
				strnfp = strnfp.substr(slashpos + 1);
				std::transform(bankidstr.begin(), bankidstr.end(), bankidstr.begin(), ::toupper);
				cAssetBank * bank = 0;
				size_t bankid = 0;
				for (size_t b = 0; b < vhBanks.size(); b++)
					if (vhBanks[b]->GetFolderName().compare(bankidstr) == 0) {
						bank = vhBanks[b];
						bankid = b;
						break;
					}
				if (!bank)
					continue;

				std::string mntpnt = bank->GetMountPointForFile(strnfp, ap->GetPrefix());
				if (mntpnt.empty())
					continue;
				int mntid = GetMountPointID(mntpnt);
				if (f == 0 || f == 1 && mntid == -1) {
					if (MountFile(mntpnt, vFiles[f].at(i))) {
						bAddedMntPnts = 1;
						cAsset * as = bank->AllocateAssetForMountPoint(this, vMountEntries[vMountEntries.size() - 1]);
						if (!as)
							GV->Console->Printf("~r~Error loading asset for mount point ~y~%s~r~!", mntpnt.c_str());
						else {
							ap->RegisterAsset(as);
							vMountEntries[vMountEntries.size() - 1].hAsset = as;
						}
					}
					else {
						int mntid = GetMountPointID(mntpnt);
						vMountEntries[mntid].hAsset->SetForceReload(1);
					}
					iBankNew[bankid]++;
					strMntPntToUpdate[bankid].push_back(mntpnt);
				}
				else if (f == 1) {
					if (vMountEntries[mntid].hAsset == 0)
						continue;
					if (vMountEntries[mntid].hAsset->GetFile().hFeed == vFiles[f].at(i).hFeed &&
						vMountEntries[mntid].hAsset->GetFile().strPath == vFiles[f].at(i).strPath) {
						vMountEntries[mntid].hAsset->SetForceReload(1);
						iBankChange[bankid]++;
						strMntPntToUpdate[bankid].push_back(mntpnt);
					}
				}
				else if (f == 2) {
					if (vMountEntries[mntid].hAsset->GetFile().hFeed == vFiles[f].at(i).hFeed &&
						vMountEntries[mntid].hAsset->GetFile().strPath == vFiles[f].at(i).strPath &&
						vMountEntries[mntid].vFiles.size() > 1) {
						strMntPntToUpdate[bankid].push_back(mntpnt);
						vMountEntries[mntid].hAsset->SetForceReload(1);
					}
					iBankDel[bankid]++;
					UnmountFile(mntpnt, vFiles[f].at(i));
				}
			}
		}
	}
	if (bAddedMntPnts) {
		_SortMountEntries();
	}
	for (size_t b = 0; b < vhBanks.size(); b++) {
		if (iBankNew[b] != 0 || iBankChange[b] != 0 || iBankDel[b] != 0) {
			vhBanks[b]->_bModFlag = 1;
			vhBanks[b]->_iModNew = iBankNew[b];
			vhBanks[b]->_iModChange = iBankChange[b];
			vhBanks[b]->_iModDel = iBankDel[b];
		}
		if (!strMntPntToUpdate[b].empty()) {
			vhBanks[b]->BatchProcessStart(this);
			vhBanks[b]->BatchProcessEnd(this);
		}
	}

	delete[] iBankNew;
	delete[] iBankChange;
	delete[] iBankDel;
	delete[] strMntPntToUpdate;

	if (hge->Timer_GetTime() - fFeedRefreshTime > 2.5f)
		fFeedRefreshTime = hge->Timer_GetTime();
}
