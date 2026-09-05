#ifndef H_C_LOGICSBANK
#define H_C_LOGICSBANK

#include "../cDataController.h"

class cCustomLogic : public cAsset {
protected:
    std::string strContent;

    cCustomLogic(cFile hFile, std::string id);

    void SetName(std::string n) { _strName = n; };

    friend class cBankLogic;

public:
    ~cCustomLogic() override {}

    void Load() override;

    void Unload() override;

    void SetContent(std::string str);

    void Save();

    void DeleteFile();

    std::string GetContent() { return strContent; };

    std::string GetPath();
};

class cBankLogic : public cAssetBank<cCustomLogic> {
private:
    cCustomLogic *hGlobalScript = 0;
    bool selectWhenAdding = false;

public:
    explicit cBankLogic(cDataController *hDC) : cAssetBank<cCustomLogic>(hDC) {}

    void SetGlobalScript(cCustomLogic *h);

    void RegisterLogic(cCustomLogic *h);

    void SelectWhenAddingNextLogic() { selectWhenAdding = true; }

    cCustomLogic *GetGlobalScript() { return hGlobalScript; };

    cCustomLogic *GetLogicByName(const char *pszID);

    bool RenameLogic(cCustomLogic *hLogic, const std::string& strName);

    void SortLogics();

    const std::string& GetFolderName() override {
        static const std::string name = "LOGICS";
        static const std::string namez = "LOGICZ";
        return hDC->GetGame() == WWD::Game_Gruntz ? namez : name;
    };

    void BatchProcessStart() override;

    std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix) override;

    cCustomLogic *AllocateAssetForMountPoint(cDC_MountEntry mountEntry) override;

    void DeleteAsset(cCustomLogic *hLogic) override;
};

#endif
