#include "logics.h"

#include <algorithm>
#include "../../shared/cProgressInfo.h"
#include "../../shared/commonFunc.h"
#include "../globals.h"
#include "../states/editing_ww.h"

extern structProgressInfo _ghProgressInfo;

bool cLogicsBank_Sort(cCustomLogic * a, cCustomLogic * b)
{
	return (std::string(a->GetName()) < std::string(b->GetName()));
}

cCustomLogic::cCustomLogic(cFile hFile, std::string id)
{
	SetFile(hFile);
	_strName = id;
	_bLoaded = 0;
}

cCustomLogic::~cCustomLogic()
{

}

void cCustomLogic::Load()
{
	if (_bLoaded) return;
	unsigned int len;
	unsigned char * ptr = GetFile().hFeed->GetFileContent(GetFile().strPath.c_str(), len);
	if (len == 0) return;
	strContent = std::string((const char*)ptr, len);
	delete[] ptr;
	_bLoaded = 1;
}

void cCustomLogic::Unload()
{
	strContent = "";
	_bLoaded = 0;
}

void cCustomLogic::SetContent(std::string str)
{
	strContent = str;
}

void cCustomLogic::Save()
{
	cFile n;
	if (GetFile().hFeed == 0) {
		n = GV->editState->hDataCtrl->AssignFileForLogic(GetName());
	}
	else
		n = GetFile();
	n.hFeed->SetFileContent(n.strPath.c_str(), strContent);
	SetFile(n);
}

void cCustomLogic::DeleteFile()
{
	std::string strAbsPath = GetFile().hFeed->GetAbsoluteLocation() + "/" + GetFile().strPath;
	remove(strAbsPath.c_str());
}

cBankLogic::cBankLogic()
{
	hGlobalScript = 0;
}

cBankLogic::~cBankLogic()
{

}

void cBankLogic::DeleteLogic(cCustomLogic * hLogic)
{
	hLogic->DeleteFile();
	for (size_t i = 0; i < m_vAssets.size(); i++) {
		if (m_vAssets[i] == hLogic) {
			m_vAssets.erase(m_vAssets.begin() + i);
			break;
		}
	}
	delete hLogic;
}

void cBankLogic::SetGlobalScript(cCustomLogic * h)
{
	hGlobalScript = h;
}

void cBankLogic::RegisterLogic(cCustomLogic * h)
{
	m_vAssets.push_back(h);
	SortLogics();
}

cCustomLogic * cBankLogic::GetLogicByName(const char * pszID)
{
	for (size_t i = 0; i < m_vAssets.size(); i++)
		if (!strcmp(m_vAssets[i]->GetName(), pszID))
			return m_vAssets[i];
	return 0;
}

std::string cBankLogic::getElementAt(int i)
{
	if (i < 0 || i >= m_vAssets.size()) return "";
	return m_vAssets[i]->GetName();
}

int cBankLogic::getNumberOfElements()
{
	return m_vAssets.size();
}

void cBankLogic::SortLogics()
{
	std::sort(m_vAssets.begin(), m_vAssets.end(), cLogicsBank_Sort);
}

void cBankLogic::BatchProcessStart(cDataController * hDC)
{
	_ghProgressInfo.iGlobalProgress = 8;
	_ghProgressInfo.strGlobalCaption = "Loading custom logics...";
}

bool cBankLogic::RenameLogic(cCustomLogic * hLogic, std::string strName)
{
	if (strName == "main") return 0;
	cFile origFile = hLogic->GetFile();

	std::string dirpath = "";
	if (origFile.strPath.find_last_of("/\\") != std::string::npos)
		dirpath = origFile.strPath.substr(0, origFile.strPath.find_last_of("/\\"));
	std::string nFilePath = dirpath + "/" + strName + ".lua";

	std::string absOrigPath = origFile.hFeed->GetAbsoluteLocation() + "/" + origFile.strPath;
	std::string absNewPath = origFile.hFeed->GetAbsoluteLocation() + "/" + nFilePath;
	FILE * f = fopen(absNewPath.c_str(), "rb");
	if (f != 0) {
		fclose(f);
		return 0;
	}
	if (rename(absOrigPath.c_str(), absNewPath.c_str()) != 0)
		return 0;
	origFile.strPath = nFilePath;
	hLogic->SetFile(origFile);
	hLogic->SetName(strName);
	SortLogics();
	return 1;
}

void cBankLogic::ProcessAssets(cAssetPackage * hClientAP, std::vector<cFile> vFiles)
{
	std::vector<cCustomLogic*> vN;
	for (size_t i = 0; i < vFiles.size(); i++) {
		size_t lastdot = vFiles[i].strPath.rfind('.');
		if (lastdot == std::string::npos || lastdot == vFiles[i].strPath.length() - 1) continue;
		std::string ext = vFiles[i].strPath.substr(lastdot + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if (ext.compare("lua") != 0) continue;
		std::string logicname = vFiles[i].strPath.substr(0, lastdot);
		size_t slash = logicname.find_last_of("/\\");
		if (slash != std::string::npos) {
			logicname = logicname.substr(slash + 1);
		}
		cCustomLogic * n = new cCustomLogic(vFiles[i], logicname);
		vN.push_back(n);
		if (logicname == "main")
			hGlobalScript = n;
		else
			m_vAssets.push_back(n);
		hClientAP->RegisterAsset(n);
	}
	SortLogics();
	while (!vN.empty()) {
		vN.front()->Load();
		vN.erase(vN.begin());
	}
}

std::string cBankLogic::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
	return std::string("/LOGICS/") + strPrefix + "/" + strFilePath;
}

cAsset *cBankLogic::AllocateAssetForMountPoint(cDataController *hDC, cDC_MountEntry mountEntry) {
	int nameStart = mountEntry.vFiles[0].strPath.find_last_of("/\\") + 1;
	int nameEnd = mountEntry.vFiles[0].strPath.find_last_of(".");
	std::string filename = mountEntry.vFiles[0].strPath.substr(nameStart, nameEnd - nameStart);

	if (filename == "main") {
		printf("dupa");
	}

	auto customLogic = new cCustomLogic(mountEntry.vFiles[0], filename);

	m_vAssets.push_back(customLogic);

	return customLogic;
}
