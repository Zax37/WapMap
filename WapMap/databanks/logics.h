#ifndef H_C_LOGICSBANK
#define H_C_LOGICSBANK

#include <vector>
#include <guichan/listmodel.hpp>
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

    std::string GetPath();
};

class cBankLogic : public gcn::ListModel, public cAssetBank {
private:
    std::vector<cCustomLogic *> m_vAssets;
    cCustomLogic *hGlobalScript;
    bool selectWhenAdding;

public:
    cBankLogic(WWD::Parser *hParser);

    ~cBankLogic();

    void SetGlobalScript(cCustomLogic *h);

    void RegisterLogic(cCustomLogic *h);

    void SelectWhenAddingNextLogic() { selectWhenAdding = true; }

    cCustomLogic *GetGlobalScript() { return hGlobalScript; };

    cCustomLogic *GetLogicByName(const char *pszID);

    cCustomLogic *GetLogicByIterator(int iIT) {
        if (iIT < 0 || iIT >= m_vAssets.size()) return NULL;
        return m_vAssets[iIT];
    }

    //inherited
    std::string getElementAt(int i);

    int getNumberOfElements();

    bool RenameLogic(cCustomLogic *hLogic, const std::string& strName);

    void SortLogics();

    const std::string& GetFolderName() override {
        static const std::string name = "LOGICS";
        static const std::string namez = "LOGICZ";
        return hParser->GetGame() == WWD::Game_Gruntz ? namez : name;
    };

    virtual void BatchProcessStart(cDataController *hDC);

    virtual std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix);

    virtual cAsset *AllocateAssetForMountPoint(cDataController *hDC, cDC_MountEntry mountEntry);

    virtual void DeleteAsset(cAsset *hLogic) override;
};

#endif
