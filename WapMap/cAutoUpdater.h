#ifndef H_C_AUTOUPDATER
#define H_C_AUTOUPDATER

#include <windows.h>
#include <process.h>
#include "../shared/gcnWidgets/wContainer.h"
#include "../shared/gcnWidgets/wWin.h"
#include "../shared/gcnWidgets/wButton.h"
#include "../shared/gcnWidgets/wLink.h"
#include "../shared/gcnWidgets/wLabel.h"
#include "../shared/gcnWidgets/wContext.h"
#include "../shared/gcnWidgets/wProgressBar.h"
#include "wViewport.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <json.hpp>

#define AU_NONE 0
#define AU_SEARCHING_FOR_UPDATES 1
#define AU_ASKING_TO_UPDATE 2
#define AU_DOWNLOADING_UPDATE 3

/*
cAutoUpdater checks whether there are any updates,
and if so, downloads them. Integrates interface
handling and all things done dirty way.
*/

class cAutoUpdater;

struct GitAPIReleaseAsset {
    std::string browser_download_url;
    uint32_t size;
};

void from_json(const nlohmann::json &j, GitAPIReleaseAsset &releaseAsset);

//internal action listener for gui
class cAUAL : public gcn::ActionListener {
private:
    cAutoUpdater *m_hOwn;
public:
    void action(const gcn::ActionEvent &actionEvent);

    cAUAL(cAutoUpdater *owner);
};

//internal viewport callback for gui
/*class cAUVP : public WIDG::VpCallback {
private:
    cAutoUpdater *m_hOwn;
public:
    cAUVP(cAutoUpdater *owner) { m_hOwn = owner; };
};*/

class cAutoUpdater {
    friend class cAUVP;

public:
    float fDelay = 0;
    int iState, iThreadsRunning;
    std::string response;
    bool bKill = false;

    GitAPIReleaseAsset releaseAsset;
    const char *hUpdatePackage = 0;
    size_t downloadedBytes = 0;

    CURL *curl_handle_single;
    CURLM *curl_handle_multi;

    cAutoUpdater();

    ~cAutoUpdater();

    bool Think();

    cAUAL *hAL;
    //cAUVP *hVP;
    SHR::Win *winActualize;
    SHR::Lab *labActualize;
    SHR::But *butYes, *butNo;
    SHR::ProgressBar *pbProgress;
    WIDG::Viewport *vpActualize;

    void PopupQuestion(SHR::Container *dest);

    void TransformToDownload();

    void TransformToExit();
};

#endif
