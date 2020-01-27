#ifndef H_C_LOGICSBANK
#define H_C_LOGICSBANK

#include <vector>
#include <guichan/listModel.hpp>
#include "../cDataController.h"

class cCustomLogic : public cAsset {
protected:
    std::string strContent;

    cCustomLogic(cFile hFile, std::string id);

    ~cCustomLogic();

    void SetName(std::string n) { _strName = n; };

    friend class cBankLogic;

public:

    virtual void Load();

    virtual void Unload();

    void SetContent(std::string str);

    void Save();

    void DeleteFile();

    std::string GetContent() { return strContent; };
};

class cBankLogic : public gcn::ListModel, public cAssetBank {
private:
    std::vector<cCustomLogic *> m_vAssets;
    cCustomLogic *hGlobalScript;
public:
    cBankLogic();

    ~cBankLogic();

    void SetGlobalScript(cCustomLogic *h);

    void RegisterLogic(cCustomLogic *h);

    cCustomLogic *GetGlobalScript() { return hGlobalScript; };

    cCustomLogic *GetLogicByName(const char *pszID);

    cCustomLogic *GetLogicByIterator(int iIT) {
        if (iIT < 0 || iIT >= m_vAssets.size()) return NULL;
        return m_vAssets[iIT];
    }

    int GetLogicsCount() { return m_vAssets.size(); };

    //inherited
    std::string getElementAt(int i);

    int getNumberOfElements();

    bool RenameLogic(cCustomLogic *hLogic, std::string strName);

    void SortLogics();

    virtual void ProcessAssets(cAssetPackage *hClientAP, std::vector<cFile> vFiles);

    virtual std::string GetFolderName() { return std::string("LOGICS"); };

    virtual void BatchProcessStart(cDataController *hDC);

    virtual std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix);

    virtual cAsset *AllocateAssetForMountPoint(cDataController *hDC, cDC_MountEntry mountEntry);

    virtual void DeleteAsset(cAsset *hLogic) override;
};

#endif
