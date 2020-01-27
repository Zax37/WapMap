#include "cAutoUpdater.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <sstream>
#include "globals.h"
#include "langID.h"
#include "../shared/commonFunc.h"
#include "states/editing_ww.h"
#include "resources.h"
#include "version.h"
#include "../shared/ZipLib/ZipArchive.h"
#include "../shared/ZipLib/utils/stream_utils.h"


#define RELEASES_API "https://api.github.com/repos/Zax37/WapMap/releases/latest"

extern HGE *hge;

cAutoUpdater *_AU_GLOBAL_PTR;
int DOWNLOAD_PROGRESS = 0;

void from_json(const nlohmann::json& j, GitAPIReleaseAsset& releaseAsset) {
	j.at("browser_download_url").get_to(releaseAsset.browser_download_url);
	j.at("size").get_to(releaseAsset.size);
}

size_t writeToString(void *ptr, size_t size, size_t nmemb, std::string *data) {
	data->append((char *)ptr, size * nmemb);
	return size * nmemb;
}

size_t writeToMemory(void *ptr, size_t size, size_t nmemb, const char *data) {
	void* target = (void*)(data + _AU_GLOBAL_PTR->downloadedBytes);
	size_t realsize = size * nmemb;
	memcpy(target, ptr, realsize);
	_AU_GLOBAL_PTR->downloadedBytes += realsize;
	DOWNLOAD_PROGRESS = ((100.0 * _AU_GLOBAL_PTR->downloadedBytes) / _AU_GLOBAL_PTR->releaseAsset.size);
	return realsize;
}

void cAUAL::action(const gcn::ActionEvent &actionEvent) {
    if (actionEvent.getSource() == m_hOwn->butYes) {
        if (m_hOwn->hUpdatePackage) {
            std::stringbuf sb(std::string(m_hOwn->hUpdatePackage, m_hOwn->downloadedBytes), std::ios_base::in);
            std::istream is(&sb);
			auto archive = ZipArchive::Create(is);
			size_t entries_count = archive->GetEntriesCount();

			std::string prefix = ".wm_update";

			CreateDirectory(prefix.c_str(), NULL);
			SetFileAttributes(prefix.c_str(), FILE_ATTRIBUTE_HIDDEN);

			prefix += '/';

            std::ofstream fs;
			for (size_t i = 0; i < entries_count; i++) {
			    auto entry = archive->GetEntry(i);
			    std::string filepath = prefix + entry->GetFullName();
			    auto dataStream = entry->GetDecompressionStream();

				fs.open(filepath, std::fstream::binary | std::fstream::out);
				utils::stream::copy(*dataStream, fs);
				fs.flush();
				fs.close();

				entry->CloseDecompressionStream();
			}

			prefix.pop_back();
			prefix += ".exe";

			HRSRC hResource = FindResource(nullptr, MAKEINTRESOURCEA(WM_UPDATER), "TEXT");
			HGLOBAL hMemory = LoadResource(nullptr, hResource);
			size_t dwSize = SizeofResource(nullptr, hResource);
			void* lpLock = LockResource(hMemory);
			HANDLE hFile = CreateFile(prefix.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			DWORD dwByteWritten;
			WriteFile(hFile, lpLock, dwSize, &dwByteWritten, NULL);
			CloseHandle(hFile);
			FreeResource(hMemory);

			if (dwByteWritten == dwSize) {
				ShellExecuteA(
					NULL,
					"open",
					prefix.c_str(),
					"",
					"",
					SW_SHOWNORMAL
				);
				exit(666);
			}
			else {
				GV->Console->Print("~r~Couldn't create updater executable! (no write access?)");
			}
        } else m_hOwn->TransformToDownload();
    } else if (actionEvent.getSource() == m_hOwn->butNo) {
        m_hOwn->bKill = 1;
    }
}

cAUAL::cAUAL(cAutoUpdater *owner) {
    m_hOwn = owner;
}

cAutoUpdater::cAutoUpdater() {
    iState = AU_NONE;
    _AU_GLOBAL_PTR = this;
    winActualize = NULL;
    hAL = NULL;
    //hVP = NULL;
    vpActualize = NULL;
    labActualize = NULL;
    butYes = butNo = NULL;

    curl_handle_single = curl_easy_init();
    curl_handle_multi = curl_multi_init();
    if (curl_handle_single && curl_handle_multi) {
        GV->Console->Printf("Checking for updates on ~y~%s~w~...", RELEASES_API);
        curl_easy_setopt(curl_handle_single, CURLOPT_URL, RELEASES_API);
        curl_easy_setopt(curl_handle_single, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl_handle_single, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl_handle_single, CURLOPT_USERAGENT, "curl/7.42.0");
        curl_multi_add_handle(curl_handle_multi, curl_handle_single);
        curl_multi_perform(curl_handle_multi, &iThreadsRunning);

        iState = AU_SEARCHING_FOR_UPDATES;
    } else {
        GV->Console->Print("~r~Failed to check for updates!");
    }
}

bool cAutoUpdater::Think() {
    if (fDelay > 0) {
        fDelay -= hge->Timer_GetDelta();
        return false;
    }

	if (hUpdatePackage) {
		pbProgress->setProgress(DOWNLOAD_PROGRESS / 10);
	}

    if (iThreadsRunning > 0) {
		long tm; int dontTakeTooLong = 0;
        while (iThreadsRunning > 0) {
            curl_multi_perform(curl_handle_multi, &iThreadsRunning);
            curl_multi_timeout(curl_handle_multi, &tm);

            if (tm > 0) {
                fDelay = tm / 1000.0;
                return false;
            }

            fDelay = 0;
			if (++dontTakeTooLong > 3) {
				fDelay = 0.001;
				return false;
			}
        }
    }

    switch (iState) {
        case AU_SEARCHING_FOR_UPDATES:
            curl_easy_cleanup(curl_handle_single);
            curl_multi_cleanup(curl_handle_multi);

            if (response.empty()) {
                GV->Console->Print("~r~Failed to check for updates!");
                return true;
			}
            else {
				auto parsed = nlohmann::json::parse(response);
				auto tag_name = parsed["tag_name"].get<std::string>();
				int majorVersion, minorVersion, patchVersion;

				if (sscanf(tag_name.c_str(), "v%d.%d.%d", &majorVersion, &minorVersion, &patchVersion) != 3) {
					GV->Console->Print("~r~Received unexpected response from Git API when checking for updates!");
					GV->Console->Print(response.c_str());
					return true;
				}

				if (majorVersion > MAJOR_VERSION ||
					(majorVersion == MAJOR_VERSION && (minorVersion > MINOR_VERSION ||
					(minorVersion == MINOR_VERSION && patchVersion > PATCH_VERSION)
						))
					) {
					GV->Console->Printf("New version available to download: %s", tag_name.c_str());

					auto assets = parsed["assets"].get<std::vector<GitAPIReleaseAsset>>();
					for (auto asset : assets) {
					    if (!asset.browser_download_url.empty()) {
					        releaseAsset = asset;
                            PopupQuestion(GV->editState->conMain);
                            return false;
					    }
					}

                    GV->Console->Printf("~r~Just kidding. Nothing to download there.");
					return true;
				}
                GV->Console->Printf("All up to date!");
                return true;
			}
			break;
		case AU_ASKING_TO_UPDATE:
			return bKill;
			break;
        case AU_DOWNLOADING_UPDATE:
            curl_easy_cleanup(curl_handle_single);
            curl_multi_cleanup(curl_handle_multi);
            TransformToExit();
            break;
        default:
            return true;
    }
    return false;
}

cAutoUpdater::~cAutoUpdater() {
    if (winActualize != NULL) {
        delete butYes, butNo;
        delete labActualize;
        delete winActualize;
        delete hAL;
        delete vpActualize;
        //delete hVP;
    }
}

void cAutoUpdater::PopupQuestion(SHR::Contener *dest) {
    hAL = new cAUAL(this);
    if (winActualize == NULL)
        winActualize = new SHR::Win(&GV->gcnParts, GETL(Lang_ActualizeCaption));
    else
        winActualize->setCaption(GETL(Lang_ActualizeCaption));
    winActualize->setDimension(gcn::Rectangle(0, 0, 400, 80 + GV->fntMyriad13->GetHeightb(390, GETL(Lang_ActualizeQuestion))));
	winActualize->setMovable(false);
    if (labActualize == NULL)
        labActualize = new SHR::Lab(GETL(Lang_ActualizeQuestion));
    else
        labActualize->setCaption(GETL(Lang_ActualizeQuestion));
	labActualize->setAlignment(Graphics::CENTER);
    labActualize->adjustSize();
    labActualize->setHeight(winActualize->getHeight() - 60);
    winActualize->add(labActualize, (winActualize->getWidth() - labActualize->getWidth()) / 2, 15);
    dest->add(winActualize, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200,
              hge->System_GetState(HGE_SCREENHEIGHT) / 2 - winActualize->getHeight() / 2);

    butYes = new SHR::But(GV->hGfxInterface, GETL(Lang_Yes));
    butYes->setDimension(gcn::Rectangle(0, 0, 100, 32));
    butYes->addActionListener(hAL);
    winActualize->add(butYes, 75, winActualize->getHeight() - 55);

    butNo = new SHR::But(GV->hGfxInterface, GETL(Lang_No));
    butNo->setDimension(gcn::Rectangle(0, 0, 100, 32));
    butNo->addActionListener(hAL);
    winActualize->add(butNo, 225, winActualize->getHeight() - 55);

	iState = AU_ASKING_TO_UPDATE;
}

void cAutoUpdater::TransformToExit() {
    pbProgress->setVisible(0);
    butNo->setVisible(0);

    butYes->setCaption(GETL(Lang_OK));
    butYes->setX(150);
    butYes->setVisible(1);

    labActualize->setCaption(GETL(Lang_PatchRestart));
	labActualize->adjustSize();
	labActualize->setX((winActualize->getWidth() - labActualize->getWidth()) / 2);

	iState = AU_ASKING_TO_UPDATE;
}

void cAutoUpdater::TransformToDownload()
{
	butYes->setVisible(0);
	labActualize->setCaption(GETL(Lang_DownloadingUpdate));
	labActualize->adjustSize();
	labActualize->setX((winActualize->getWidth() - labActualize->getWidth()) / 2);
	butNo->setX(150);
	butNo->setCaption(GETL(Lang_Cancel));
	pbProgress = new SHR::ProgressBar(&GV->gcnParts);
	pbProgress->setDimension(gcn::Rectangle(0, 0, 380, 31));
	pbProgress->setProgress(0);
	pbProgress->setEnd(10);
	winActualize->add(pbProgress, 10, 50);

	/*hVP = new cAUVP(this);

	vpActualize = new WIDG::Viewport(hVP, 0);
	winActualize->add(vpActualize, 5, 5);*/

	curl_handle_single = curl_easy_init();
	curl_easy_setopt(curl_handle_single, CURLOPT_URL, releaseAsset.browser_download_url.c_str());
	curl_easy_setopt(curl_handle_single, CURLOPT_USERAGENT, "curl/7.42.0");
	auto res = curl_easy_perform(curl_handle_single);

	std::string location;
	if (res == CURLE_OK) {
		int response_code;
		res = curl_easy_getinfo(curl_handle_single, CURLINFO_RESPONSE_CODE, &response_code);

		if (res == CURLE_OK && (response_code / 100) == 3) {
			const char* tmp;
			res = curl_easy_getinfo(curl_handle_single, CURLINFO_REDIRECT_URL, &tmp);
			if (res == CURLE_OK) {
				location = tmp;
			}
		}
	}
	if (res != CURLE_OK) {
		GV->Console->Print("~r~ Couldn't download the patch file.");
		iState = AU_NONE;
		return;
	}
	curl_easy_cleanup(curl_handle_single);

	hUpdatePackage = new char[releaseAsset.size];
	downloadedBytes = 0;

	curl_handle_single = curl_easy_init();
	curl_handle_multi = curl_multi_init();

	if (curl_handle_single && curl_handle_multi) {
		curl_easy_setopt(curl_handle_single, CURLOPT_URL, location.c_str());
		curl_easy_setopt(curl_handle_single, CURLOPT_WRITEDATA, hUpdatePackage);
		curl_easy_setopt(curl_handle_single, CURLOPT_WRITEFUNCTION, writeToMemory);
		curl_easy_setopt(curl_handle_single, CURLOPT_USERAGENT, "curl/7.42.0");
		curl_multi_add_handle(curl_handle_multi, curl_handle_single);
		curl_multi_perform(curl_handle_multi, &iThreadsRunning);
		iState = AU_DOWNLOADING_UPDATE;
	}
	else {
		GV->Console->Print("~r~ Couldn't download the patch file.");
		iState = AU_NONE;
	}
}
